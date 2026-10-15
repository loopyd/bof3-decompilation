//! Source-bound literal strings inside a pinned US battle code payload.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result, invalid};
use crate::records::{decode_name, encode_name};

pub const SCHEMA: &str = "bof3.inline-text/v1";
const LOAD: u32 = 0x80096800;
const SIZE: usize = 133316;
const LAYOUT_SHA256: &str = "3df942ff3b7822e4582cb809cec769712592d7059f39e06fef1e4e057be02c24";
// Address, capacity including NUL, and immutable pointer-word address.
const ALLOCATIONS: &[(u32, usize, u32)] = &[
    (0x800b6e60, 5, 0x800b6e88),
    (0x800b6e68, 7, 0x800b6e8c),
    (0x800b6e70, 6, 0x800b6e90),
    (0x800b6e78, 7, 0x800b6e94),
    (0x800b6e80, 6, 0x800b6e98),
    (0x800b6e9c, 5, 0x800b6ec4),
    (0x800b6ea4, 7, 0x800b6ec8),
    (0x800b6eac, 7, 0x800b6ecc),
    (0x800b6eb4, 6, 0x800b6ed0),
    (0x800b6ebc, 7, 0x800b6ed4),
];

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Document {
    schema: String,
    source: String,
    source_sha256: String,
    layout: String,
    pub entry: usize,
    entry_offset: usize,
    load_address: u32,
    consumers: Vec<u32>,
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

pub fn supports(entry: Entry, payload: &[u8]) -> bool {
    if entry.ram_ptr != LOAD || payload.len() != SIZE {
        return false;
    }
    let mut masked = payload.to_vec();
    for &(address, capacity, _) in ALLOCATIONS {
        let at = (address - LOAD) as usize;
        masked[at..at + capacity].fill(0);
    }
    sha256_hex(&masked) == LAYOUT_SHA256
}

pub fn extract(path: &Path, data: &[u8], entry: Entry, payload: &[u8]) -> Result<Document> {
    if !supports(entry, payload) {
        return invalid("unsupported inline text layout");
    }
    let strings = ALLOCATIONS
        .iter()
        .enumerate()
        .map(|(index, &(address, capacity, reference))| {
            let payload_offset = (address - LOAD) as usize;
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
                references: vec![reference],
                text: decode_name(bytes, entry.offset + payload_offset)?,
            })
        })
        .collect::<Result<_>>()?;
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        layout: "US battle categories".into(),
        entry: entry.index,
        entry_offset: entry.offset,
        load_address: LOAD,
        consumers: vec![0x800b28c8, 0x800b2f70],
        renderer: 0x8014f800,
        strings,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    if document.schema != SCHEMA
        || document.layout != "US battle categories"
        || document.load_address != LOAD
        || document.consumers != [0x800b28c8, 0x800b2f70]
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
        let Some(&(address, capacity, reference)) = ALLOCATIONS.get(row.index) else {
            return invalid("unknown inline string allocation");
        };
        let offset = (address - LOAD) as usize;
        if !seen.insert(row.index)
            || row.address != address
            || row.payload_offset != offset
            || document.entry_offset.checked_add(offset) != Some(row.file_offset)
            || row.capacity != capacity
            || row.references != [reference]
        {
            return invalid("unknown, duplicate or changed inline string allocation");
        }
        encode_name(&row.text, capacity - 1)?;
    }
    if seen.len() != ALLOCATIONS.len() {
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
    if entry.offset != checked.entry_offset || !supports(entry, payload) {
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
