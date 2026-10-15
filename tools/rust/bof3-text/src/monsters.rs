//! Fixed-width names in the US encounter template layout.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result, invalid};
use crate::records::{Name, decode_name, encode_name};

pub const SCHEMA: &str = "bof3.monster-text/v1";
const LOAD: u32 = 0x800e4000;
const PREFIX: usize = 0x48;
const COUNT: usize = 8;
const STRIDE: usize = 0x88;
const CAPACITY: usize = 8;
const SIZE: usize = PREFIX + COUNT * STRIDE;

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Document {
    schema: String,
    source: String,
    source_sha256: String,
    pub entry: usize,
    entry_offset: usize,
    load_address: u32,
    template_copy_consumer: u32,
    template_lookup_consumer: u32,
    name_copy_consumer: u32,
    pub names: Vec<Name>,
}

pub fn supports(entry: Entry, payload: &[u8]) -> bool {
    entry.ram_ptr == LOAD && payload.len() == SIZE
}

pub fn extract(path: &Path, data: &[u8], entry: Entry, payload: &[u8]) -> Result<Document> {
    if !supports(entry, payload) {
        return invalid("unsupported encounter template name layout");
    }
    let names = (0..COUNT)
        .map(|index| {
            let offset = PREFIX + index * STRIDE;
            let file_offset = entry.offset + offset;
            Ok(Name {
                family: "monsters".into(),
                index,
                file_offset,
                address: LOAD + offset as u32,
                capacity: CAPACITY,
                text: decode_name(&payload[offset..offset + CAPACITY], file_offset)?,
            })
        })
        .collect::<Result<_>>()?;
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        entry: entry.index,
        entry_offset: entry.offset,
        load_address: LOAD,
        template_copy_consumer: 0x800ac218,
        template_lookup_consumer: 0x800ac704,
        name_copy_consumer: 0x801dea18,
        names,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    if document.schema != SCHEMA
        || document.load_address != LOAD
        || document.template_copy_consumer != 0x800ac218
        || document.template_lookup_consumer != 0x800ac704
        || document.name_copy_consumer != 0x801dea18
        || document.entry_offset.checked_add(SIZE).is_none()
        || document.source_sha256.len() != 64
        || !document
            .source_sha256
            .bytes()
            .all(|byte| byte.is_ascii_hexdigit())
    {
        return invalid("invalid monster name document identity");
    }
    let mut seen = BTreeSet::new();
    for row in &document.names {
        if row.family != "monsters"
            || row.index >= COUNT
            || row.file_offset != document.entry_offset + PREFIX + row.index * STRIDE
            || row.address != LOAD + (PREFIX + row.index * STRIDE) as u32
            || row.capacity != CAPACITY
            || !seen.insert(row.index)
        {
            return invalid("unknown or duplicate monster name allocation");
        }
        encode_name(&row.text, CAPACITY)?;
    }
    if seen.len() != COUNT {
        return invalid("monster name document must retain all eight records");
    }
    Ok(document)
}

pub fn rebuild(path: &Path, data: &[u8], document: &Document) -> Result<Vec<u8>> {
    let checked = parse(&render(document)?)?;
    if std::fs::canonicalize(&checked.source)? != std::fs::canonicalize(path)?
        || checked.source_sha256 != sha256_hex(data)
    {
        return invalid("monster name document does not match this source path and SHA-256");
    }
    let entries = crate::lexagraph::entries(data)?;
    let (entry, payload) = entries
        .into_iter()
        .find(|(entry, _)| entry.index == checked.entry)
        .ok_or_else(|| Error::Invalid("monster name source entry is absent".into()))?;
    if entry.offset != checked.entry_offset || !supports(entry, payload) {
        return invalid("monster name document does not match the source entry layout");
    }
    // Decode the original too, so malformed template names cannot be silently replaced.
    extract(path, data, entry, payload)?;
    let mut output = data.to_vec();
    for row in checked.names {
        let bytes = encode_name(&row.text, CAPACITY)?;
        let span = &mut output[row.file_offset..row.file_offset + CAPACITY];
        span.fill(0);
        span[..bytes.len()].copy_from_slice(&bytes);
    }
    Ok(output)
}

pub fn render(document: &Document) -> Result<Vec<u8>> {
    serde_json::to_vec_pretty(document).map_err(|error| Error::Invalid(error.to_string()))
}
