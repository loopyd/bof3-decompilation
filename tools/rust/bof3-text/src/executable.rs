//! Fixed ASCII allocations consumed by the US executable's diagnostic screens.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Error, Result, invalid};

pub(crate) const SCHEMA: &str = "bof3.executable-text/v3";
const LOAD: u32 = 0x80096800;
const SIZE: usize = 1445888;
// Hash of the entire executable with only the supported allocations zeroed.
const LAYOUT_SHA256: &str = "77b703f2e8e833e658d083fff15c3d10a4f4521f56b21313cf66c76915fa042a";
const FORMAT_ADDRESS: u32 = 0x80149990;
const FILE_LABEL: u32 = 0x80149f9c;
const FILE_FORMAT: u32 = 0x80149fac;
const CONSUMERS: [u32; 2] = [0x8014b3c4, 0x80166df4];
const FORMATTER: u32 = 0x8017e3f4;
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
    (FORMAT_ADDRESS, 5),
    (FILE_LABEL, 15),
    (FILE_FORMAT, 18),
];

#[derive(Clone, Copy, Debug, Deserialize, Eq, PartialEq, Serialize)]
#[serde(rename_all = "kebab-case")]
pub(crate) enum Codec {
    AsciiLiteral,
    AsciiU32HexFormat,
    AsciiU8TripleHexFormat,
}

impl Codec {
    pub(crate) fn as_str(self) -> &'static str {
        match self {
            Self::AsciiLiteral => "ascii-literal",
            Self::AsciiU32HexFormat => "ascii-u32-hex-format",
            Self::AsciiU8TripleHexFormat => "ascii-u8-triple-hex-format",
        }
    }

    pub(crate) fn encode(self, text: &str, maximum: usize) -> Result<Vec<u8>> {
        let bytes = text.as_bytes();
        if bytes.len() > maximum
            || !bytes
                .iter()
                .all(|byte| (0x20..=0x7e).contains(byte) || *byte == b'\n')
        {
            return invalid(format!(
                "executable text must fit {maximum} ASCII bytes plus NUL"
            ));
        }
        self.max_formatted_size(text)?;
        Ok(bytes.to_vec())
    }

    pub(crate) fn max_formatted_size(self, text: &str) -> Result<Option<usize>> {
        match self {
            Self::AsciiLiteral => Ok(None),
            Self::AsciiU32HexFormat => {
                if !matches!(
                    text.as_bytes(),
                    [b'%', b'X' | b'x']
                        | [b'%', b'1'..=b'8', b'X' | b'x']
                        | [b'%', b'0', b'1'..=b'8', b'X' | b'x']
                ) {
                    return invalid(
                        "hex format must be one %X or %x conversion, optionally with width 1..8 and zero padding",
                    );
                }
                Ok(Some(8))
            }
            Self::AsciiU8TripleHexFormat => byte_triplet_size(text.as_bytes()).map(Some),
        }
    }
}

fn byte_triplet_size(bytes: &[u8]) -> Result<usize> {
    let mut cursor = 0;
    let mut arguments = 0;
    let mut rendered = 0;
    while cursor < bytes.len() {
        if bytes[cursor] != b'%' {
            cursor += 1;
            rendered += 1;
            continue;
        }
        cursor += 1;
        if bytes.get(cursor) == Some(&b'0') {
            cursor += 1;
            if !matches!(bytes.get(cursor).copied(), Some(b'1' | b'2')) {
                return invalid("byte format zero padding requires width 1 or 2");
            }
            cursor += 1;
        } else if matches!(bytes.get(cursor).copied(), Some(b'1' | b'2')) {
            cursor += 1;
        }
        if !matches!(bytes.get(cursor).copied(), Some(b'X' | b'x')) {
            return invalid(
                "byte format requires exactly three ordered %X or %x conversions, with optional width 1..2 and zero padding",
            );
        }
        cursor += 1;
        arguments += 1;
        rendered += 2;
    }
    if arguments != 3 || rendered > 11 {
        return invalid(
            "byte format must consume three arguments and render at most 11 bytes plus NUL",
        );
    }
    Ok(rendered)
}

#[derive(Debug, Deserialize, Eq, PartialEq, Serialize)]
#[serde(tag = "kind", rename_all = "kebab-case", deny_unknown_fields)]
pub enum Reference {
    PointerWord {
        address: u32,
    },
    AddressPair {
        upper: u32,
        lower: u32,
        calls: Vec<u32>,
    },
}

fn codec(address: u32) -> Codec {
    match address {
        FORMAT_ADDRESS => Codec::AsciiU32HexFormat,
        FILE_FORMAT => Codec::AsciiU8TripleHexFormat,
        _ => Codec::AsciiLiteral,
    }
}

fn consumer(address: u32) -> u32 {
    if matches!(address, FILE_LABEL | FILE_FORMAT) {
        CONSUMERS[1]
    } else {
        CONSUMERS[0]
    }
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Document {
    schema: String,
    source: String,
    source_sha256: String,
    build: String,
    load_address: u32,
    consumers: Vec<u32>,
    renderer: u32,
    formatter: u32,
    pub strings: Vec<Text>,
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Text {
    pub address: u32,
    pub consumer: u32,
    pub file_offset: usize,
    pub capacity: usize,
    pub references: Vec<Reference>,
    pub(crate) codec: Codec,
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
            "unsupported executable layout; only the pinned US diagnostic strings are supported",
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
        let codec = codec(address);
        let text = std::str::from_utf8(&bytes[..end])
            .map_err(|error| Error::Invalid(error.to_string()))?;
        codec.encode(text, capacity - 1)?;
        let references = if address == FORMAT_ADDRESS {
            vec![
                Reference::AddressPair {
                    upper: 0x8014b518,
                    lower: 0x8014b51c,
                    calls: vec![0x8014b520],
                },
                Reference::AddressPair {
                    upper: 0x8014b598,
                    lower: 0x8014b59c,
                    calls: vec![0x8014b5a0],
                },
                Reference::AddressPair {
                    upper: 0x8014b604,
                    lower: 0x8014b608,
                    calls: vec![0x8014b610, 0x8014b634],
                },
            ]
        } else if matches!(address, FILE_LABEL | FILE_FORMAT) {
            vec![if address == FILE_LABEL {
                Reference::AddressPair {
                    upper: 0x80166e1c,
                    lower: 0x80166e20,
                    calls: vec![0x80166e24],
                }
            } else {
                Reference::AddressPair {
                    upper: 0x80166e3c,
                    lower: 0x80166e40,
                    calls: vec![0x80166e44],
                }
            }]
        } else {
            (0x8017f470..0x8017f53c)
                .step_by(4)
                .filter(|reference| {
                    let at = offset(*reference);
                    u32::from_le_bytes(data[at..at + 4].try_into().unwrap()) == address
                })
                .map(|address| Reference::PointerWord { address })
                .collect()
        };
        strings.push(Text {
            address,
            consumer: consumer(address),
            file_offset,
            capacity,
            references,
            codec,
            text: text.into(),
        });
    }
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        build: "US SLUS-00422 diagnostic ASCII".into(),
        load_address: LOAD,
        consumers: CONSUMERS.to_vec(),
        renderer: 0x80150098,
        formatter: FORMATTER,
        strings,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    if document.schema != SCHEMA
        || document.load_address != LOAD
        || document.build != "US SLUS-00422 diagnostic ASCII"
        || document.consumers != CONSUMERS
        || document.renderer != 0x80150098
        || document.formatter != FORMATTER
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
            || text.codec != codec(text.address)
            || text.consumer != consumer(text.address)
        {
            return invalid("unknown or duplicate executable string allocation");
        }
        text.codec.encode(&text.text, text.capacity - 1)?;
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
            return invalid("executable reference sites cannot be edited or relocated");
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
