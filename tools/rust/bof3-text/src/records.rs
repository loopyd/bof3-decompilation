//! Fixed twelve-byte equipment and ability names in the US GAME.EMI tables.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Error, Result, invalid, literal_byte, literal_char};

pub const SCHEMA: &str = "bof3.record-text/v1";
const LOAD: u32 = 0x80195800;
const LAYOUT_SHA256: &str = "a5caeb5e57907656436b77d840daf2be7c9703958e6a309fcaf956d4a65e4708";
const TABLES: &[(&str, usize, usize, usize)] = &[
    ("items", 0x33964, 92, 0x12),
    ("key-items", 0x33fdc, 16, 0x10),
    ("weapons", 0x340dc, 83, 0x18),
    ("armor", 0x348a4, 68, 0x16),
    ("accessories", 0x34e7c, 52, 0x14),
    ("abilities", 0x3570c, 228, 0x14),
];

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Document {
    schema: String,
    source: String,
    source_sha256: String,
    entry: usize,
    load_address: u32,
    equipment_resolver: u32,
    equipment_name_copy_consumer: u32,
    ability_name_copy_consumer: u32,
    pub names: Vec<Name>,
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Name {
    pub family: String,
    pub index: usize,
    pub file_offset: usize,
    pub address: u32,
    pub(crate) capacity: usize,
    pub text: String,
}

pub fn supports(data: &[u8]) -> bool {
    let Ok(entries) = crate::lexagraph::entries(data) else {
        return false;
    };
    if !entries
        .first()
        .is_some_and(|(entry, _)| entry.ram_ptr == LOAD)
        || data.len() < 0x368dc
    {
        return false;
    }
    let mut masked = data.to_vec();
    for &(_, start, count, stride) in TABLES {
        for index in 0..count {
            masked[start + index * stride..start + index * stride + 12].fill(0);
        }
    }
    sha256_hex(&masked) == LAYOUT_SHA256
}

pub(crate) fn encode_name(text: &str, capacity: usize) -> Result<Vec<u8>> {
    let bytes: Vec<_> = text
        .chars()
        .map(|character| {
            literal_byte(character).ok_or_else(|| {
                Error::Invalid(format!(
                    "name character {character:?} has no literal encoding"
                ))
            })
        })
        .collect::<Result<_>>()?;
    if bytes.len() > capacity {
        return invalid(format!("record name exceeds its {capacity}-byte field"));
    }
    Ok(bytes)
}

pub(crate) fn decode_name(bytes: &[u8], file_offset: usize) -> Result<String> {
    let end = bytes
        .iter()
        .position(|byte| *byte == 0)
        .unwrap_or(bytes.len());
    if bytes[end..].iter().any(|byte| *byte != 0) {
        return invalid(format!(
            "nonzero bytes after name terminator at {file_offset:#x}"
        ));
    }
    bytes[..end]
        .iter()
        .map(|byte| {
            literal_char(*byte).ok_or_else(|| {
                Error::Invalid(format!(
                    "unknown literal name byte {byte:#x} at {file_offset:#x}"
                ))
            })
        })
        .collect()
}

pub fn extract(path: &Path, data: &[u8]) -> Result<Document> {
    if !supports(data) {
        return invalid("unsupported name record layout");
    }
    let mut names = Vec::new();
    for &(family, start, count, stride) in TABLES {
        for index in 0..count {
            let file_offset = start + index * stride;
            let bytes = &data[file_offset..file_offset + 12];
            let text = decode_name(bytes, file_offset)?;
            names.push(Name {
                family: family.into(),
                index,
                file_offset,
                address: LOAD + (file_offset - 0x800) as u32,
                capacity: 12,
                text,
            });
        }
    }
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        entry: 0,
        load_address: LOAD,
        equipment_resolver: 0x80165d48,
        equipment_name_copy_consumer: 0x801f1a30,
        ability_name_copy_consumer: 0x801d3d64,
        names,
    })
}

pub fn parse(bytes: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(bytes).map_err(|error| Error::Invalid(error.to_string()))?;
    if document.schema != SCHEMA
        || document.entry != 0
        || document.load_address != LOAD
        || document.equipment_resolver != 0x80165d48
        || document.equipment_name_copy_consumer != 0x801f1a30
        || document.ability_name_copy_consumer != 0x801d3d64
        || document.source_sha256.len() != 64
        || !document
            .source_sha256
            .bytes()
            .all(|byte| byte.is_ascii_hexdigit())
    {
        return invalid("invalid name record document identity");
    }
    let mut seen = BTreeSet::new();
    for row in &document.names {
        let &(_, start, count, stride) = TABLES
            .iter()
            .find(|(family, _, _, _)| *family == row.family)
            .ok_or_else(|| Error::Invalid("unknown name family".into()))?;
        if row.index >= count
            || row.file_offset != start + row.index * stride
            || row.capacity != 12
            || row.address != LOAD + (row.file_offset - 0x800) as u32
            || !seen.insert((&row.family, row.index))
        {
            return invalid("unknown or duplicate name record allocation");
        }
        encode_name(&row.text, 12)?;
    }
    if seen.len() != TABLES.iter().map(|(_, _, count, _)| count).sum::<usize>() {
        return invalid("name record document must retain every supported name");
    }
    Ok(document)
}

pub fn rebuild(path: &Path, data: &[u8], document: &Document) -> Result<Vec<u8>> {
    let original = extract(path, data)?;
    if std::fs::canonicalize(&document.source)? != std::fs::canonicalize(path)?
        || document.source_sha256 != original.source_sha256
    {
        return invalid("name record document does not match this source path and SHA-256");
    }
    let checked = parse(&render(document)?)?;
    let mut output = data.to_vec();
    for row in checked.names {
        let bytes = encode_name(&row.text, 12)?;
        let span = &mut output[row.file_offset..row.file_offset + 12];
        span.fill(0);
        span[..bytes.len()].copy_from_slice(&bytes);
    }
    Ok(output)
}

pub fn render(document: &Document) -> Result<Vec<u8>> {
    serde_json::to_vec_pretty(document).map_err(|error| Error::Invalid(error.to_string()))
}
