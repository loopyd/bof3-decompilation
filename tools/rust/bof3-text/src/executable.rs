//! Fixed ASCII allocations consumed by the US executable's exception renderer.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Error, Result, invalid};

const SCHEMA: &str = "bof3.executable-text/v1";
const LOAD: u32 = 0x80096800;
const SIZE: usize = 1445888;
// Hash of the entire executable with only the supported allocations zeroed.
const LAYOUT_SHA256: &str = "78b41a61a0f65cfa31674f5d7508372d14b331531528dd62b571906f611922a5";
// Original string bytes plus their NUL, excluding alignment padding. The consumer
// at 0x8014b3c4 passes pointers from 0x8017f470..0x8017f53c to 0x80150098.
const ALLOCATIONS: &[(u32, usize)] = &[
    (0x80149814, 10),
    (0x80149820, 5),
    (0x80149828, 3),
    (0x8014982c, 5),
    (0x80149834, 5),
    (0x8014983c, 4),
    (0x80149840, 3),
    (0x80149844, 3),
    (0x80149848, 3),
    (0x8014984c, 3),
    (0x80149850, 3),
    (0x80149854, 3),
    (0x80149858, 3),
    (0x8014985c, 3),
    (0x80149860, 3),
    (0x80149864, 3),
    (0x80149868, 3),
    (0x8014986c, 3),
    (0x80149870, 3),
    (0x80149874, 3),
    (0x80149878, 3),
    (0x8014987c, 3),
    (0x80149880, 3),
    (0x80149884, 3),
    (0x80149888, 3),
    (0x8014988c, 3),
    (0x80149890, 3),
    (0x80149894, 3),
    (0x80149898, 3),
    (0x8014989c, 3),
    (0x801498a0, 3),
    (0x801498a4, 3),
    (0x801498a8, 3),
    (0x801498ac, 3),
    (0x801498b0, 3),
    (0x801498b4, 3),
    (0x801498b8, 3),
    (0x801498bc, 5),
    (0x801498c4, 20),
    (0x801498d8, 20),
    (0x801498ec, 21),
    (0x80149904, 18),
    (0x80149918, 8),
    (0x80149920, 18),
    (0x80149934, 25),
    (0x80149950, 23),
    (0x80149968, 22),
    (0x80149980, 1),
    (0x80149984, 11),
];

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Document {
    schema: String,
    source: String,
    source_sha256: String,
    build: String,
    load_address: u32,
    consumer: u32,
    renderer: u32,
    pub strings: Vec<Text>,
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Text {
    pub address: u32,
    pub file_offset: usize,
    pub capacity: usize,
    pub references: Vec<u32>,
    pub text: String,
}

pub fn is_executable(data: &[u8]) -> bool {
    data.starts_with(b"PS-X EXE")
}

fn offset(address: u32) -> usize {
    (address - LOAD) as usize + 0x800
}

/// Recognition binds all bytes outside the documented editable allocations.
pub fn supports(data: &[u8]) -> bool {
    if data.len() != SIZE || !is_executable(data) {
        return false;
    }
    let mut masked = data.to_vec();
    for &(address, capacity) in ALLOCATIONS {
        masked[offset(address)..offset(address) + capacity].fill(0);
    }
    sha256_hex(&masked) == LAYOUT_SHA256
}

pub fn extract(path: &Path, data: &[u8]) -> Result<Document> {
    if !supports(data) {
        return invalid(
            "unsupported executable layout; only the pinned US exception strings are supported",
        );
    }
    let mut strings = Vec::new();
    for &(address, capacity) in ALLOCATIONS {
        let file_offset = offset(address);
        let bytes = &data[file_offset..file_offset + capacity];
        let end = bytes
            .iter()
            .position(|byte| *byte == 0)
            .ok_or_else(|| Error::Invalid(format!("unterminated ASCII string at {address:#x}")))?;
        if !bytes[..end]
            .iter()
            .all(|byte| (0x20..=0x7e).contains(byte) || *byte == b'\n')
        {
            return invalid(format!("non-ASCII executable string at {address:#x}"));
        }
        let references = (0x8017f470..0x8017f53c)
            .step_by(4)
            .filter(|reference| {
                let at = offset(*reference);
                u32::from_le_bytes(data[at..at + 4].try_into().unwrap()) == address
            })
            .collect();
        strings.push(Text {
            address,
            file_offset,
            capacity,
            references,
            text: String::from_utf8(bytes[..end].to_vec()).unwrap(),
        });
    }
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        build: "US SLUS-00422 exception ASCII".into(),
        load_address: LOAD,
        consumer: 0x8014b3c4,
        renderer: 0x80150098,
        strings,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    if document.schema != SCHEMA
        || document.load_address != LOAD
        || document.build != "US SLUS-00422 exception ASCII"
        || document.consumer != 0x8014b3c4
        || document.renderer != 0x80150098
        || document.source_sha256.len() != 64
        || !document
            .source_sha256
            .bytes()
            .all(|byte| byte.is_ascii_hexdigit())
    {
        return invalid("invalid executable text document identity");
    }
    let mut seen = BTreeSet::new();
    for text in &document.strings {
        if !seen.insert(text.address)
            || !ALLOCATIONS.contains(&(text.address, text.capacity))
            || text.file_offset != offset(text.address)
        {
            return invalid("unknown or duplicate executable string allocation");
        }
        if text.text.len() >= text.capacity
            || !text
                .text
                .bytes()
                .all(|byte| (0x20..=0x7e).contains(&byte) || byte == b'\n')
        {
            return invalid(format!(
                "string at {:#x} must fit {} ASCII bytes plus NUL",
                text.address,
                text.capacity - 1
            ));
        }
    }
    if seen.len() != ALLOCATIONS.len() {
        return invalid("executable text document must retain every supported allocation");
    }
    Ok(document)
}

pub fn rebuild(path: &Path, data: &[u8], document: &Document) -> Result<Vec<u8>> {
    let original = extract(path, data)?;
    if std::fs::canonicalize(&document.source)? != std::fs::canonicalize(path)?
        || document.source_sha256 != original.source_sha256
    {
        return invalid("executable text document does not match this source path and SHA-256");
    }
    // Validate public callers as well as JSON imports before touching any output bytes.
    let checked =
        parse(&serde_json::to_vec(document).map_err(|error| Error::Invalid(error.to_string()))?)?;
    let mut output = data.to_vec();
    for text in &checked.strings {
        let expected = original
            .strings
            .iter()
            .find(|row| row.address == text.address)
            .unwrap();
        if text.references != expected.references {
            return invalid("executable pointer references cannot be edited or relocated");
        }
        if text.text != expected.text {
            let span = &mut output[text.file_offset..text.file_offset + text.capacity];
            span.fill(0);
            span[..text.text.len()].copy_from_slice(text.text.as_bytes());
        }
    }
    Ok(output)
}

pub fn render(document: &Document) -> Result<Vec<u8>> {
    serde_json::to_vec_pretty(document).map_err(|error| Error::Invalid(error.to_string()))
}
