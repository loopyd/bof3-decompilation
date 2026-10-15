//! Five-byte default character names in the US START and STATUS record copies.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Error, Result, invalid};
use crate::records::{Name, decode_name, encode_name};

pub const SCHEMA: &str = "bof3.character-text/v1";
const LOAD: u32 = 0x801d0c00;
const NAME_ADDRESS: u32 = 0x801eb514;
const COUNT: usize = 8;
const STRIDE: usize = 0xa4;
const CAPACITY: usize = 5;

struct Profile {
    kind: &'static str,
    size: usize,
    entry: usize,
    offset: usize,
    masked_sha256: &'static str,
}

const PROFILES: &[Profile] = &[
    Profile {
        kind: "start",
        size: 518144,
        entry: 8,
        offset: 0x72914,
        masked_sha256: "c128d64121b680e49818aad38df1940a8ed529d7e0357a956a0a3ddd07e8d9ad",
    },
    Profile {
        kind: "status",
        size: 262144,
        entry: 0,
        offset: 0x1b114,
        masked_sha256: "bb1349f8efac3f00d66af8e814093267120a603701c44adc2383d736a36d6a3d",
    },
];

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Document {
    schema: String,
    source: String,
    source_sha256: String,
    archive_kind: String,
    pub entry: usize,
    load_address: u32,
    initializer: u32,
    name_copy_consumer: u32,
    renderer: u32,
    pub names: Vec<Name>,
}

fn profile(data: &[u8]) -> Option<&'static Profile> {
    PROFILES.iter().find(|profile| {
        if data.len() != profile.size {
            return false;
        }
        let mut masked = data.to_vec();
        for index in 0..COUNT {
            let at = profile.offset + index * STRIDE;
            masked[at..at + CAPACITY].fill(0);
        }
        sha256_hex(&masked) == profile.masked_sha256
    })
}

pub fn supports(data: &[u8]) -> bool {
    profile(data).is_some()
}

pub fn extract(path: &Path, data: &[u8]) -> Result<Document> {
    let profile = profile(data)
        .ok_or_else(|| Error::Invalid("unsupported default character name layout".into()))?;
    let names = (0..COUNT)
        .map(|index| {
            let file_offset = profile.offset + index * STRIDE;
            Ok(Name {
                family: "characters".into(),
                index,
                file_offset,
                address: NAME_ADDRESS + (index * STRIDE) as u32,
                capacity: CAPACITY,
                text: decode_name(&data[file_offset..file_offset + CAPACITY], file_offset)?,
            })
        })
        .collect::<Result<_>>()?;
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        archive_kind: profile.kind.into(),
        entry: profile.entry,
        load_address: LOAD,
        initializer: 0x801d15c0,
        name_copy_consumer: 0x801d39cc,
        renderer: 0x8014f800,
        names,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    let profile = PROFILES
        .iter()
        .find(|profile| profile.kind == document.archive_kind)
        .ok_or_else(|| Error::Invalid("unknown character name source layout".into()))?;
    if document.schema != SCHEMA
        || document.entry != profile.entry
        || document.load_address != LOAD
        || document.initializer != 0x801d15c0
        || document.name_copy_consumer != 0x801d39cc
        || document.renderer != 0x8014f800
        || document.source_sha256.len() != 64
        || !document
            .source_sha256
            .bytes()
            .all(|byte| byte.is_ascii_hexdigit())
    {
        return invalid("invalid character name document identity");
    }
    let mut seen = BTreeSet::new();
    for row in &document.names {
        if row.family != "characters"
            || row.index >= COUNT
            || row.file_offset != profile.offset + row.index * STRIDE
            || row.address != NAME_ADDRESS + (row.index * STRIDE) as u32
            || row.capacity != CAPACITY
            || !seen.insert(row.index)
        {
            return invalid("unknown or duplicate character name allocation");
        }
        encode_name(&row.text, CAPACITY)?;
    }
    if seen.len() != COUNT {
        return invalid("character name document must retain all eight records");
    }
    Ok(document)
}

pub fn rebuild(path: &Path, data: &[u8], document: &Document) -> Result<Vec<u8>> {
    let original = extract(path, data)?;
    if std::fs::canonicalize(&document.source)? != std::fs::canonicalize(path)?
        || document.source_sha256 != original.source_sha256
        || document.archive_kind != original.archive_kind
    {
        return invalid("character name document does not match this source path and SHA-256");
    }
    let checked = parse(&render(document)?)?;
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
