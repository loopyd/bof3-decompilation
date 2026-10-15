//! Source-bound text allocations inside pinned US overlay code payloads.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result, invalid};
use crate::records::{decode_name, encode_name};

pub const SCHEMA: &str = "bof3.inline-text/v3";

#[derive(Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub(crate) enum Codec {
    GameLiteral,
    AsciiDecimalFormat,
}

impl Codec {
    pub(crate) fn encode(self, text: &str, capacity: usize) -> Result<Vec<u8>> {
        if self == Self::GameLiteral {
            return encode_name(text, capacity);
        }
        if text.len() > capacity
            || !text.bytes().all(|byte| (0x20..=0x7e).contains(&byte))
            || text.matches("%3d").count() != 1
            || text.bytes().filter(|byte| *byte == b'%').count() != 1
        {
            return invalid(
                "ASCII decimal format must fit its allocation and retain exactly one %3d with no other percent sequences",
            );
        }
        Ok(text.as_bytes().to_vec())
    }

    fn decode(self, bytes: &[u8], file_offset: usize) -> Result<String> {
        if self == Self::GameLiteral {
            return decode_name(bytes, file_offset);
        }
        let end = bytes.iter().position(|byte| *byte == 0).ok_or_else(|| {
            Error::Invalid(format!("unterminated ASCII format at {file_offset:#x}"))
        })?;
        if bytes[end..].iter().any(|byte| *byte != 0) {
            return invalid("nonzero bytes after ASCII format terminator");
        }
        let text = std::str::from_utf8(&bytes[..end])
            .map_err(|error| Error::Invalid(error.to_string()))?;
        self.encode(text, bytes.len() - 1)?;
        Ok(text.into())
    }

    pub(crate) fn as_str(self) -> &'static str {
        match self {
            Self::GameLiteral => "game-literal",
            Self::AsciiDecimalFormat => "ascii-decimal-format",
        }
    }
}

struct Allocation {
    address: u32,
    capacity: usize,
    reference: Option<u32>,
    renderer: u32,
    codec: Codec,
}

impl Allocation {
    const fn literal(address: u32, capacity: usize, reference: Option<u32>, renderer: u32) -> Self {
        Self {
            address,
            capacity,
            reference,
            renderer,
            codec: Codec::GameLiteral,
        }
    }

    const fn decimal(address: u32, capacity: usize) -> Self {
        Self {
            address,
            capacity,
            reference: None,
            renderer: 0x80150098,
            codec: Codec::AsciiDecimalFormat,
        }
    }
}

struct Layout {
    name: &'static str,
    load: u32,
    size: usize,
    sha256: &'static str,
    consumers: &'static [u32],
    allocations: &'static [Allocation],
}

const LAYOUTS: &[Layout] = &[
    Layout {
        name: "US master labels and numeric formats",
        load: 0x801d0c00,
        size: 13963,
        sha256: "a0f3799398e41708fd3df9dca0dfbc732cfec237eb47b6ece6bf9dc3d23434e8",
        consumers: &[
            0x801d1500, 0x801d151c, 0x801d1574, 0x801d1590, 0x801d15dc, 0x801d15f8, 0x801d1638,
            0x801d1654, 0x801d1864, 0x801d1920, 0x801d195c, 0x801d19a0, 0x801d19e0, 0x801d1a10,
        ],
        allocations: &[
            Allocation::decimal(0x801d0c04, 4),
            Allocation::decimal(0x801d0c08, 8),
            Allocation::decimal(0x801d0c10, 8),
            Allocation::literal(0x801d4038, 4, None, 0x8014f800),
            Allocation::literal(0x801d403c, 4, None, 0x8014f800),
            Allocation::literal(0x801d4040, 4, None, 0x8014f800),
            Allocation::literal(0x801d4044, 4, None, 0x8014f800),
            Allocation::literal(0x801d4048, 5, None, 0x8014fc90),
            Allocation::literal(0x801d4050, 5, None, 0x8014fc90),
        ],
    },
    Layout {
        name: "US battle categories",
        load: 0x80096800,
        size: 133316,
        sha256: "3df942ff3b7822e4582cb809cec769712592d7059f39e06fef1e4e057be02c24",
        consumers: &[0x800b28c8, 0x800b2f70],
        allocations: &[
            Allocation::literal(0x800b6e60, 5, Some(0x800b6e88), 0x8014f800),
            Allocation::literal(0x800b6e68, 7, Some(0x800b6e8c), 0x8014f800),
            Allocation::literal(0x800b6e70, 6, Some(0x800b6e90), 0x8014f800),
            Allocation::literal(0x800b6e78, 7, Some(0x800b6e94), 0x8014f800),
            Allocation::literal(0x800b6e80, 6, Some(0x800b6e98), 0x8014f800),
            Allocation::literal(0x800b6e9c, 5, Some(0x800b6ec4), 0x8014f800),
            Allocation::literal(0x800b6ea4, 7, Some(0x800b6ec8), 0x8014f800),
            Allocation::literal(0x800b6eac, 7, Some(0x800b6ecc), 0x8014f800),
            Allocation::literal(0x800b6eb4, 6, Some(0x800b6ed0), 0x8014f800),
            Allocation::literal(0x800b6ebc, 7, Some(0x800b6ed4), 0x8014f800),
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
            Allocation::literal(0x801eb000, 13, None, 0x8014f800),
            Allocation::literal(0x801eb01a, 13, None, 0x8014f800),
            Allocation::literal(0x801eb04e, 13, None, 0x8014f800),
            Allocation::literal(0x801eb05b, 13, None, 0x8014f800),
            Allocation::literal(0x801eb068, 13, None, 0x8014f800),
            Allocation::literal(0x801eb075, 13, None, 0x8014f800),
            Allocation::literal(0x801eb082, 13, None, 0x8014f800),
            Allocation::literal(0x801eb08f, 13, None, 0x8014f800),
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
    pub renderer: u32,
    pub(crate) codec: Codec,
    references: Vec<u32>,
    text: String,
}

fn layout_for(entry: Entry, payload: &[u8]) -> Option<&'static Layout> {
    LAYOUTS.iter().find(|layout| {
        if entry.ram_ptr != layout.load || payload.len() != layout.size {
            return false;
        }
        let mut masked = payload.to_vec();
        for allocation in layout.allocations {
            let at = (allocation.address - layout.load) as usize;
            masked[at..at + allocation.capacity].fill(0);
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
        .map(|(index, allocation)| {
            let Allocation {
                address,
                capacity,
                reference,
                renderer,
                codec,
            } = *allocation;
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
                renderer,
                codec,
                references: reference.into_iter().collect(),
                text: codec.decode(bytes, entry.offset + payload_offset)?,
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
        let Some(allocation) = layout.allocations.get(row.index) else {
            return invalid("unknown inline string allocation");
        };
        let Allocation {
            address,
            capacity,
            reference,
            renderer,
            codec,
        } = *allocation;
        let offset = (address - layout.load) as usize;
        if !seen.insert(row.index)
            || row.address != address
            || row.payload_offset != offset
            || document.entry_offset.checked_add(offset) != Some(row.file_offset)
            || row.capacity != capacity
            || row.renderer != renderer
            || row.codec != codec
            || row.references != reference.into_iter().collect::<Vec<_>>()
        {
            return invalid("unknown, duplicate or changed inline string allocation");
        }
        codec.encode(&row.text, capacity - 1)?;
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
        let bytes = row.codec.encode(&row.text, row.capacity - 1)?;
        let span = &mut output[row.file_offset..row.file_offset + row.capacity];
        span.fill(0);
        span[..bytes.len()].copy_from_slice(&bytes);
    }
    Ok(output)
}

pub fn render(document: &Document) -> Result<Vec<u8>> {
    serde_json::to_vec_pretty(document).map_err(|error| Error::Invalid(error.to_string()))
}
