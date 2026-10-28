//! Fixed ASCII allocations consumed by the US executable's diagnostics.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Error, Result, invalid};

pub(crate) const SCHEMA: &str = "bof3.executable-text/v9";
const LOAD: u32 = 0x80096800;
const SIZE: usize = 1445888;
// Hash of the entire executable with only the supported allocations zeroed.
const LAYOUT_SHA256: &str = "59c86326d15ebe2a24cc1fc7f0197627e7f898b61381d0920aff716bb75fcde7";
const FORMAT_ADDRESS: u32 = 0x80149990;
const FILE_LABEL: u32 = 0x80149f9c;
const FILE_FORMAT: u32 = 0x80149fac;
const DISK_PREFIX: u32 = 0x8014a30c;
const CD_INTERRUPT_LABELS: [u32; 6] = [
    0x8014a2a0, 0x8014a2ac, 0x8014a2b4, 0x8014a2c0, 0x8014a2cc, 0x8014a2d8,
];
const CD_COMMAND_LABELS: [u32; 22] = [
    0x8014a1a4, 0x8014a1b0, 0x8014a1bc, 0x8014a1c8, 0x8014a1d4, 0x8014a1e0, 0x8014a1ec, 0x8014a1fc,
    0x8014a208, 0x8014a218, 0x8014a224, 0x8014a22c, 0x8014a238, 0x8014a244, 0x8014a24c, 0x8014a258,
    0x8014a264, 0x8014a270, 0x8014a27c, 0x8014a284, 0x8014a290, 0x8014a298,
];
const BIOS_PRINTF: u32 = 0x8017e3e4;
const BIOS_PUTS: u32 = 0x8017e33c;
const CD_FALLBACK: u32 = 0x8014a1f8;
const CD_PUTS_LABELS: [u32; 6] = [
    0x8014a2e0, 0x8014a334, 0x8014a3d0, 0x8014a5e0, 0x8014a5f8, 0x8014a610,
];
const CD_SHARED_LABELS: [u32; 4] = [0x8014a368, 0x8014a370, 0x8014a394, 0x8014a3e8];
const CD_PRINTF_LITERALS: [u32; 6] = [
    0x8014a428, 0x8014a480, 0x8014a4b4, 0x8014a508, 0x8014a560, 0x8014a580,
];
const CD_PRINTF_FORMATS: [u32; 14] = [
    0x8014a2f0, 0x8014a318, 0x8014a348, 0x8014a37c, 0x8014a384, 0x8014a3dc, 0x8014a3f4, 0x8014a410,
    0x8014a444, 0x8014a464, 0x8014a470, 0x8014a4e4, 0x8014a53c, 0x8014a5c0,
];
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
    (CD_COMMAND_LABELS[0], 9),
    (CD_COMMAND_LABELS[1], 9),
    (CD_COMMAND_LABELS[2], 9),
    (CD_COMMAND_LABELS[3], 9),
    (CD_COMMAND_LABELS[4], 9),
    (CD_COMMAND_LABELS[5], 11),
    (CD_COMMAND_LABELS[6], 11),
    (CD_FALLBACK, 2),
    (CD_COMMAND_LABELS[7], 11),
    (CD_COMMAND_LABELS[8], 13),
    (CD_COMMAND_LABELS[9], 10),
    (CD_COMMAND_LABELS[10], 8),
    (CD_COMMAND_LABELS[11], 9),
    (CD_COMMAND_LABELS[12], 9),
    (CD_COMMAND_LABELS[13], 8),
    (CD_COMMAND_LABELS[14], 11),
    (CD_COMMAND_LABELS[15], 9),
    (CD_COMMAND_LABELS[16], 12),
    (CD_COMMAND_LABELS[17], 11),
    (CD_COMMAND_LABELS[18], 8),
    (CD_COMMAND_LABELS[19], 10),
    (CD_COMMAND_LABELS[20], 7),
    (CD_COMMAND_LABELS[21], 8),
    (CD_INTERRUPT_LABELS[0], 10),
    (CD_INTERRUPT_LABELS[1], 8),
    (CD_INTERRUPT_LABELS[2], 12),
    (CD_INTERRUPT_LABELS[3], 9),
    (CD_INTERRUPT_LABELS[4], 10),
    (CD_INTERRUPT_LABELS[5], 7),
    (CD_PUTS_LABELS[0], 13),
    (0x8014a2f0, 27),
    (DISK_PREFIX, 12),
    (0x8014a318, 25),
    (CD_PUTS_LABELS[1], 20),
    (0x8014a348, 6),
    (CD_SHARED_LABELS[0], 8),
    (CD_SHARED_LABELS[1], 9),
    (0x8014a37c, 7),
    (0x8014a384, 14),
    (CD_SHARED_LABELS[2], 6),
    (CD_PUTS_LABELS[2], 9),
    (0x8014a3dc, 11),
    (CD_SHARED_LABELS[3], 12),
    (0x8014a3f4, 27),
    (0x8014a410, 23),
    (CD_PRINTF_LITERALS[0], 26),
    (0x8014a444, 31),
    (0x8014a464, 12),
    (0x8014a470, 15),
    (CD_PRINTF_LITERALS[1], 41),
    (CD_PRINTF_LITERALS[2], 48),
    (0x8014a4e4, 35),
    (CD_PRINTF_LITERALS[3], 29),
    (0x8014a53c, 35),
    (CD_PRINTF_LITERALS[4], 29),
    (CD_PRINTF_LITERALS[5], 28),
    (0x8014a5c0, 30),
    (CD_PUTS_LABELS[3], 22),
    (CD_PUTS_LABELS[4], 23),
    (CD_PUTS_LABELS[5], 18),
];

#[derive(Clone, Copy, Debug, Deserialize, Eq, PartialEq, Serialize)]
#[serde(rename_all = "kebab-case")]
pub(crate) enum Codec {
    AsciiLiteral,
    AsciiPrintfLiteral,
    AsciiBiosPrintfFormat,
    AsciiU32HexFormat,
    AsciiU8TripleHexFormat,
}

impl Codec {
    pub(crate) fn as_str(self) -> &'static str {
        match self {
            Self::AsciiLiteral => "ascii-literal",
            Self::AsciiPrintfLiteral => "ascii-printf-literal",
            Self::AsciiBiosPrintfFormat => "ascii-bios-printf-format",
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
            Self::AsciiBiosPrintfFormat => {
                printf_directives(text)?;
                Ok(None)
            }
            Self::AsciiPrintfLiteral => {
                if text.contains('%') {
                    return invalid("BIOS printf literal cannot contain percent characters");
                }
                Ok(Some(text.len()))
            }
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

fn printf_directives(text: &str) -> Result<Vec<&str>> {
    let mut directives = Vec::new();
    let mut remaining = text;
    while let Some(index) = remaining.find('%') {
        remaining = &remaining[index..];
        let token = ["%02x", "%08x", "%s", "%d"]
            .into_iter()
            .find(|token| remaining.starts_with(token))
            .ok_or_else(|| Error::Invalid("unsupported BIOS printf directive".into()))?;
        directives.push(token);
        remaining = &remaining[token.len()..];
    }
    Ok(directives)
}

fn format_directives(address: u32) -> &'static [&'static str] {
    match address {
        0x8014a2f0 => &["%s", "%s", "%s", "%s"],
        0x8014a318 => &["%s", "%02x", "%02x"],
        0x8014a348 => &["%d"],
        0x8014a37c => &["%s"],
        0x8014a384 => &["%s"],
        0x8014a3dc => &["%08x"],
        0x8014a3f4 => &["%s", "%d"],
        0x8014a410 => &["%s"],
        0x8014a444 => &["%s"],
        0x8014a464 => &["%s"],
        0x8014a470 => &["%s"],
        0x8014a4e4 => &["%08x"],
        0x8014a53c => &["%d"],
        0x8014a5c0 => &["%d"],
        _ => &[],
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
    if CD_PRINTF_FORMATS.contains(&address) {
        return Codec::AsciiBiosPrintfFormat;
    }
    if CD_PRINTF_LITERALS.contains(&address) {
        return Codec::AsciiPrintfLiteral;
    }
    match address {
        FORMAT_ADDRESS => Codec::AsciiU32HexFormat,
        FILE_FORMAT => Codec::AsciiU8TripleHexFormat,
        DISK_PREFIX => Codec::AsciiPrintfLiteral,
        _ => Codec::AsciiLiteral,
    }
}

fn consumers(address: u32) -> Vec<u32> {
    if CD_COMMAND_LABELS.contains(&address) {
        return vec![0x801755d8];
    }
    if CD_INTERRUPT_LABELS.contains(&address) {
        return vec![0x8017560c];
    }
    match address {
        0x8014a318 => vec![0x80175c60],
        0x8014a348 => vec![0x80175c60],
        0x8014a37c => vec![0x80176734],
        0x8014a384 => vec![0x80176734],
        0x8014a3dc => vec![0x80176dfc],
        0x8014a3f4 => vec![0x80177348],
        0x8014a410 => vec![0x80177348],
        0x8014a444 => vec![0x80177348],
        0x8014a464 => vec![0x80177348],
        0x8014a470 => vec![0x80177348],
        0x8014a4e4 => vec![0x80177634],
        0x8014a53c => vec![0x80177634],
        0x8014a5c0 => vec![0x801779bc],
        0x8014a2f0 => vec![0x801761e8, 0x80176468, 0x80176734, 0x80176fe8],
        0x8014a2e0 => vec![0x801761e8, 0x80176468, 0x80176734, 0x80176fe8],
        FILE_LABEL | FILE_FORMAT => vec![0x80166df4],
        DISK_PREFIX | 0x8014a334 => vec![0x80175c60],
        0x8014a368 => vec![0x801761e8],
        0x8014a370 => vec![0x80176468],
        0x8014a394 => vec![0x80176734],
        0x8014a3d0 => vec![0x80176dfc],
        0x8014a3e8 => vec![0x80176fe8],
        0x8014a428 => vec![0x80177348],
        0x8014a480 | 0x8014a4b4 | 0x8014a508 => vec![0x80177634],
        0x8014a560 | 0x8014a580 => vec![0x801779bc],
        0x8014a5e0 => vec![0x80177d08],
        0x8014a5f8 | 0x8014a610 => vec![0x80177f1c],
        CD_FALLBACK => vec![0x801755d8, 0x8017560c],
        _ => vec![0x8014b3c4],
    }
}

fn all_consumers() -> Vec<u32> {
    ALLOCATIONS
        .iter()
        .flat_map(|&(address, _)| consumers(address))
        .collect::<BTreeSet<_>>()
        .into_iter()
        .collect()
}

fn cd_diagnostic_references(address: u32) -> Option<Vec<Reference>> {
    let pairs: &[(u32, u32, Option<u32>)] = match address {
        0x8014a2f0 => &[
            (0x801762f0, 0x801762f4, Some(0x801762f8)),
            (0x80176578, 0x8017657c, Some(0x80176580)),
            (0x801769e8, 0x801769ec, Some(0x801769f0)),
            (0x801770e0, 0x801770e4, Some(0x801770e8)),
        ],
        0x8014a318 => &[(0x80175e8c, 0x80175e90, Some(0x80175e94))],
        0x8014a348 => &[(0x801761bc, 0x801761c0, Some(0x801761c4))],
        0x8014a37c => &[(0x80176788, 0x8017678c, Some(0x80176790))],
        0x8014a384 => &[(0x801767e0, 0x801767e4, Some(0x801767e8))],
        0x8014a3dc => &[(0x80176e14, 0x80176e18, Some(0x80176e24))],
        0x8014a3f4 => &[(0x80177460, 0x80177464, Some(0x80177468))],
        0x8014a410 => &[(0x8017749c, 0x801774a0, Some(0x801775e4))],
        0x8014a444 => &[(0x80177500, 0x80177504, Some(0x80177508))],
        0x8014a464 => &[(0x80177568, 0x8017756c, Some(0x80177570))],
        0x8014a470 => &[(0x801775dc, 0x801775e0, Some(0x801775e4))],
        0x8014a4e4 => &[(0x80177738, 0x8017773c, Some(0x80177740))],
        0x8014a53c => &[(0x801778d8, 0x801778dc, Some(0x801778e0))],
        0x8014a5c0 => &[(0x80177c28, 0x80177c2c, Some(0x80177c30))],
        0x8014a2e0 => &[
            (0x8017629c, 0x801762a0, Some(0x801762a4)),
            (0x80176524, 0x80176528, Some(0x8017652c)),
            (0x80176994, 0x80176998, Some(0x8017699c)),
            (0x80177090, 0x80177094, Some(0x80177098)),
        ],
        0x8014a334 => &[(0x801761a8, 0x801761ac, Some(0x801761b0))],
        0x8014a368 => &[(0x8017623c, 0x80176240, None)],
        0x8014a370 => &[(0x801764c4, 0x801764c8, None)],
        0x8014a394 => &[(0x80176924, 0x80176928, None)],
        0x8014a3d0 => &[(0x80176e00, 0x80176e04, Some(0x80176e0c))],
        0x8014a3e8 => &[(0x80177030, 0x80177034, None)],
        0x8014a428 => &[(0x801774d0, 0x801774d4, Some(0x801774d8))],
        0x8014a480 => &[(0x80177690, 0x80177694, Some(0x80177698))],
        0x8014a4b4 => &[(0x801776d4, 0x801776d8, Some(0x801776dc))],
        0x8014a508 => &[(0x80177768, 0x8017776c, Some(0x80177770))],
        0x8014a560 => &[(0x80177a44, 0x80177a48, Some(0x80177a4c))],
        0x8014a580 => &[(0x80177a74, 0x80177a78, Some(0x80177a7c))],
        0x8014a5e0 => &[(0x80177d78, 0x80177d7c, Some(0x80177d80))],
        0x8014a5f8 => &[(0x80177f68, 0x80177f6c, Some(0x80177f70))],
        0x8014a610 => &[(0x80177fb0, 0x80177fb4, Some(0x80177fb8))],
        _ => return None,
    };
    Some(
        pairs
            .iter()
            .map(|&(upper, lower, call)| Reference::AddressPair {
                upper,
                lower,
                calls: call.into_iter().collect(),
            })
            .collect(),
    )
}

#[derive(Clone, Debug, Deserialize, Eq, PartialEq, Serialize)]
#[serde(tag = "kind", rename_all = "kebab-case", deny_unknown_fields)]
pub(crate) enum Output {
    GameScreen {
        renderer: u32,
        formatter: Option<u32>,
    },
    BiosPrintf {
        stub: u32,
        vector: u32,
        selector: u32,
    },
    BiosPuts {
        stub: u32,
        vector: u32,
        selector: u32,
    },
}

fn output(address: u32) -> Output {
    if CD_PUTS_LABELS.contains(&address) {
        return Output::BiosPuts {
            stub: BIOS_PUTS,
            vector: 0xb0,
            selector: 0x3f,
        };
    }
    if address == DISK_PREFIX
        || address == CD_FALLBACK
        || CD_INTERRUPT_LABELS.contains(&address)
        || CD_COMMAND_LABELS.contains(&address)
        || CD_SHARED_LABELS.contains(&address)
        || CD_PRINTF_LITERALS.contains(&address)
        || CD_PRINTF_FORMATS.contains(&address)
    {
        Output::BiosPrintf {
            stub: BIOS_PRINTF,
            vector: 0xa0,
            selector: 0x3f,
        }
    } else {
        Output::GameScreen {
            renderer: 0x80150098,
            formatter: matches!(address, FORMAT_ADDRESS | FILE_FORMAT).then_some(FORMATTER),
        }
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
    pub strings: Vec<Text>,
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Text {
    pub address: u32,
    pub consumers: Vec<u32>,
    pub(crate) output: Output,
    pub file_offset: usize,
    pub capacity: usize,
    pub references: Vec<Reference>,
    pub(crate) codec: Codec,
    pub format_directives: Vec<String>,
    pub text: String,
}

impl Text {
    fn encode(&self) -> Result<Vec<u8>> {
        let bytes = self.codec.encode(&self.text, self.capacity - 1)?;
        if self.codec == Codec::AsciiBiosPrintfFormat
            && printf_directives(&self.text)? != self.format_directives
        {
            return invalid("BIOS printf directives must retain their original bytes and order");
        }
        Ok(bytes)
    }
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
        let references = if let Some(references) = cd_diagnostic_references(address) {
            references
        } else if address == DISK_PREFIX {
            vec![Reference::AddressPair {
                upper: 0x80175e40,
                lower: 0x80175e44,
                calls: vec![0x80175e48],
            }]
        } else if address == FORMAT_ADDRESS {
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
            let table = if address == CD_FALLBACK {
                0x801857a4..0x80185844
            } else if CD_COMMAND_LABELS.contains(&address) {
                0x801857a4..0x80185814
            } else if CD_INTERRUPT_LABELS.contains(&address) {
                0x80185824..0x8018583c
            } else {
                0x8017f470..0x8017f53c
            };
            table
                .step_by(4)
                .filter(|reference| {
                    let at = offset(*reference);
                    u32::from_le_bytes(data[at..at + 4].try_into().unwrap()) == address
                })
                .map(|address| Reference::PointerWord { address })
                .collect()
        };
        let row = Text {
            address,
            consumers: consumers(address),
            output: output(address),
            file_offset,
            capacity,
            references,
            codec,
            format_directives: format_directives(address)
                .iter()
                .map(|token| (*token).into())
                .collect(),
            text: text.into(),
        };
        row.encode()?;
        strings.push(row);
    }
    Ok(Document {
        schema: SCHEMA.into(),
        source: path.to_string_lossy().into(),
        source_sha256: sha256_hex(data),
        build: "US SLUS-00422 diagnostic ASCII".into(),
        load_address: LOAD,
        consumers: all_consumers(),
        strings,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    if document.schema != SCHEMA
        || document.load_address != LOAD
        || document.build != "US SLUS-00422 diagnostic ASCII"
        || document.consumers != all_consumers()
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
            || text.consumers != consumers(text.address)
            || text.output != output(text.address)
            || text.format_directives != format_directives(text.address)
        {
            return invalid("unknown or duplicate executable string allocation");
        }
        text.encode()?;
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
