//! Source-bound literal allocations inside pinned US overlay code payloads.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result, invalid};
use crate::records::{decode_name, encode_name};

pub const SCHEMA: &str = "bof3.inline-text/v1";
struct Layout {
    name: &'static str,
    load: u32,
    size: usize,
    sha256: &'static str,
    consumers: &'static [u32],
    // Address, capacity including NUL, and an immutable pointer-word address, if any.
    allocations: &'static [(u32, usize, Option<u32>)],
}

const LAYOUTS: &[Layout] = &[
    Layout {
        name: "US master stat labels",
        load: 0x801d0c00,
        size: 13963,
        sha256: "51385434af5360c35d86a0646d253f36480351b2826a173addcc0b0ca00ed91a",
        consumers: &[0x801d1500, 0x801d1574, 0x801d15dc, 0x801d1638],
        allocations: &[
            (0x801d4038, 4, None),
            (0x801d403c, 4, None),
            (0x801d4040, 4, None),
            (0x801d4044, 4, None),
        ],
    },
    Layout {
        name: "US battle categories",
        load: 0x80096800,
        size: 133316,
        sha256: "3df942ff3b7822e4582cb809cec769712592d7059f39e06fef1e4e057be02c24",
        consumers: &[0x800b28c8, 0x800b2f70],
        allocations: &[
            (0x800b6e60, 5, Some(0x800b6e88)),
            (0x800b6e68, 7, Some(0x800b6e8c)),
            (0x800b6e70, 6, Some(0x800b6e90)),
            (0x800b6e78, 7, Some(0x800b6e94)),
            (0x800b6e80, 6, Some(0x800b6e98)),
            (0x800b6e9c, 5, Some(0x800b6ec4)),
            (0x800b6ea4, 7, Some(0x800b6ec8)),
            (0x800b6eac, 7, Some(0x800b6ecc)),
            (0x800b6eb4, 6, Some(0x800b6ed0)),
            (0x800b6ebc, 7, Some(0x800b6ed4)),
        ],
    },
    Layout {
        name: "US battle literal messages",
        load: 0x801d0c00,
        size: 118224,
        sha256: "321468941d42c8d749a319f3d862a29f890671d18284c6fc2d89c496a9caf92b",
        consumers: &[0x801de94c, 0x801de560, 0x801e9538],
        // Indexed records and direct code references, rather than a pointer table.
        // Rows 1/3 contain control bytes; rows 4/5 lack traced selections.
        allocations: &[
            (0x801eb000, 13, None),
            (0x801eb01a, 13, None),
            (0x801eb04e, 13, None),
            (0x801eb05b, 13, None),
            (0x801eb068, 13, None),
            (0x801eb075, 13, None),
            (0x801eb082, 13, None),
            (0x801eb08f, 13, None),
        ],
    },
];

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Document {
    schema: String,
    source: String,
    source_sha256: String,
    pub(crate) layout: String,
    pub entry: usize,
    entry_offset: usize,
    load_address: u32,
    pub(crate) consumers: Vec<u32>,
    renderer: u32,
    pub strings: Vec<Text>,
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Text {
    index: usize,
    pub address: u32,
    pub file_offset: usize,
    payload_offset: usize,
    pub capacity: usize,
    references: Vec<u32>,
    text: String,
}

fn layout_for(entry: Entry, payload: &[u8]) -> Option<&'static Layout> {
    LAYOUTS.iter().find(|layout| {
        if entry.ram_ptr != layout.load || payload.len() != layout.size {
            return false;
        }
        let mut masked = payload.to_vec();
        for &(address, capacity, _) in layout.allocations {
            let at = (address - layout.load) as usize;
            masked[at..at + capacity].fill(0);
        }
        sha256_hex(&masked) == layout.sha256
    })
}

pub fn supports(entry: Entry, payload: &[u8]) -> bool {
    layout_for(entry, payload).is_some()
}

pub fn extract(path: &Path, data: &[u8], entry: Entry, payload: &[u8]) -> Result<Document> {
    let layout = layout_for(entry, payload)
        .ok_or_else(|| Error::Invalid("unsupported inline text layout".into()))?;
    let strings = layout
        .allocations
        .iter()
        .enumerate()
        .map(|(index, &(address, capacity, reference))| {
            let payload_offset = (address - layout.load) as usize;
            let bytes = &payload[payload_offset..payload_offset + capacity];
            if !bytes.contains(&0) {
                return invalid("inline literal string must retain its NUL terminator");
            }
            Ok(Text {
                index,
                address,
                file_offset: entry.offset + payload_offset,
                payload_offset,
                capacity,
                references: reference.into_iter().collect(),
                text: decode_name(bytes, entry.offset + payload_offset)?,
            })
        })
        .collect::<Result<_>>()?;
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        layout: layout.name.into(),
        entry: entry.index,
        entry_offset: entry.offset,
        load_address: layout.load,
        consumers: layout.consumers.into(),
        renderer: 0x8014f800,
        strings,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    let layout = LAYOUTS
        .iter()
        .find(|layout| layout.name == document.layout)
        .ok_or_else(|| Error::Invalid("unknown inline text layout".into()))?;
    if document.schema != SCHEMA
        || document.load_address != layout.load
        || document.consumers != layout.consumers
        || document.renderer != 0x8014f800
        || document.source_sha256.len() != 64
        || !document
            .source_sha256
            .bytes()
            .all(|byte| byte.is_ascii_hexdigit())
    {
        return invalid("invalid inline text document identity");
    }
    let mut seen = BTreeSet::new();
    for row in &document.strings {
        let Some(&(address, capacity, reference)) = layout.allocations.get(row.index) else {
            return invalid("unknown inline string allocation");
        };
        let offset = (address - layout.load) as usize;
        if !seen.insert(row.index)
            || row.address != address
            || row.payload_offset != offset
            || document.entry_offset.checked_add(offset) != Some(row.file_offset)
            || row.capacity != capacity
            || row.references != reference.into_iter().collect::<Vec<_>>()
        {
            return invalid("unknown, duplicate or changed inline string allocation");
        }
        encode_name(&row.text, capacity - 1)?;
    }
    if seen.len() != layout.allocations.len() {
        return invalid("inline text document must retain all supported strings");
    }
    Ok(document)
}

pub fn rebuild(path: &Path, data: &[u8], document: &Document) -> Result<Vec<u8>> {
    if std::fs::canonicalize(&document.source)? != std::fs::canonicalize(path)?
        || document.source_sha256 != sha256_hex(data)
    {
        return invalid("inline text document does not match this source path and SHA-256");
    }
    let checked = parse(&render(document)?)?;
    let entries = crate::lexagraph::entries(data)?;
    let &(entry, payload) = entries
        .iter()
        .find(|(entry, _)| entry.index == checked.entry)
        .ok_or_else(|| Error::Invalid("inline text source entry is absent".into()))?;
    if entry.offset != checked.entry_offset
        || !layout_for(entry, payload).is_some_and(|layout| layout.name == checked.layout)
    {
        return invalid("inline text source entry does not match the supported layout");
    }
    let mut output = data.to_vec();
    for row in checked.strings {
        let bytes = encode_name(&row.text, row.capacity - 1)?;
        let span = &mut output[row.file_offset..row.file_offset + row.capacity];
        span.fill(0);
        span[..bytes.len()].copy_from_slice(&bytes);
    }
    Ok(output)
}

pub fn render(document: &Document) -> Result<Vec<u8>> {
    serde_json::to_vec_pretty(document).map_err(|error| Error::Invalid(error.to_string()))
}
