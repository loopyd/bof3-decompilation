//! Fixed ASCII allocations consumed by the supported US executables' diagnostics.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Error, Result, invalid};

pub(crate) const SCHEMA: &str = "bof3.executable-text/v17";
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
const BIOS_PUTS_LABELS: [u32; 7] = [
    0x8014a2e0, 0x8014a334, 0x8014a3d0, 0x8014a5e0, 0x8014a5f8, 0x8014a610, 0x8014a0dc,
];
const CD_SHARED_LABELS: [u32; 4] = [0x8014a368, 0x8014a370, 0x8014a394, 0x8014a3e8];
const BIOS_PRINTF_LITERALS: [u32; 11] = [
    0x8014a428, 0x8014a480, 0x8014a4b4, 0x8014a508, 0x8014a560, 0x8014a580, 0x80186888, 0x801868b9,
    0x8014a048, 0x8014a06c, 0x8014a184,
];
const BIOS_PRINTF_FORMATS: [u32; 22] = [
    0x8014a2f0, 0x8014a318, 0x8014a348, 0x8014a37c, 0x8014a384, 0x8014a3dc, 0x8014a3f4, 0x8014a410,
    0x8014a444, 0x8014a464, 0x8014a470, 0x8014a4e4, 0x8014a53c, 0x8014a5c0, 0x8014a728, 0x8014a8dc,
    0x8014a910, 0x80149fc0, 0x8014a120, 0x8014a13c, 0x8014a158, 0x8014a174,
];
const BIOS_PRINTF_ARGUMENTS: [u32; 3] = [0x80149fd0, 0x80149fe0, 0x80149ff4];
const GPU_PRINTF_CALLBACK: u32 = 0x8018b158;
const GPU_PRINTF_CALLBACK_FORMATS: [u32; 27] = [
    0x8014a624, 0x8014a63c, 0x8014a64c, 0x8014a664, 0x8014a674, 0x8014a68c, 0x8014a698, 0x8014a6a4,
    0x8014a6c0, 0x8014a6dc, 0x8014a6e8, 0x8014a748, 0x8014a75c, 0x8014a774, 0x8014a7a0, 0x8014a7b4,
    0x8014a7d0, 0x8014a7e4, 0x8014a7f8, 0x8014a804, 0x8014a818, 0x8014a84c, 0x8014a864, 0x8014a87c,
    0x8014a890, 0x8014a8a8, 0x8014a8c4,
];
const GPU_PRINTF_ARGUMENTS: [u32; 4] = [0x8014a81c, 0x8014a828, 0x8014a834, 0x8014a840];
const FORMATTER: u32 = 0x8017e3f4;
// Original string bytes plus their NUL, excluding alignment padding. The consumer
// at 0x8014b3c4 passes pointers from 0x8017f470..0x8017f53c to 0x80150098.
const SLUS_ALLOCATIONS: &[(u32, usize)] = &[
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
    (0x80149fc0, 14),
    (0x80149fd0, 13),
    (0x80149fe0, 19),
    (0x80149ff4, 20),
    (0x8014a048, 36),
    (0x8014a06c, 23),
    (0x8014a0dc, 16),
    (0x8014a120, 28),
    (0x8014a13c, 25),
    (0x8014a158, 26),
    (0x8014a174, 15),
    (0x8014a184, 21),
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
    (BIOS_PUTS_LABELS[0], 13),
    (0x8014a2f0, 27),
    (DISK_PREFIX, 12),
    (0x8014a318, 25),
    (BIOS_PUTS_LABELS[1], 20),
    (0x8014a348, 6),
    (CD_SHARED_LABELS[0], 8),
    (CD_SHARED_LABELS[1], 9),
    (0x8014a37c, 7),
    (0x8014a384, 14),
    (CD_SHARED_LABELS[2], 6),
    (BIOS_PUTS_LABELS[2], 9),
    (0x8014a3dc, 11),
    (CD_SHARED_LABELS[3], 12),
    (0x8014a3f4, 27),
    (0x8014a410, 23),
    (BIOS_PRINTF_LITERALS[0], 26),
    (0x8014a444, 31),
    (0x8014a464, 12),
    (0x8014a470, 15),
    (BIOS_PRINTF_LITERALS[1], 41),
    (BIOS_PRINTF_LITERALS[2], 48),
    (0x8014a4e4, 35),
    (BIOS_PRINTF_LITERALS[3], 29),
    (0x8014a53c, 35),
    (BIOS_PRINTF_LITERALS[4], 29),
    (BIOS_PRINTF_LITERALS[5], 28),
    (0x8014a5c0, 30),
    (BIOS_PUTS_LABELS[3], 22),
    (BIOS_PUTS_LABELS[4], 23),
    (BIOS_PUTS_LABELS[5], 18),
    (0x8014a624, 22),
    (0x8014a63c, 15),
    (0x8014a64c, 24),
    (0x8014a664, 16),
    (0x8014a674, 22),
    (0x8014a68c, 10),
    (0x8014a698, 10),
    (0x8014a6a4, 26),
    (0x8014a6c0, 26),
    (0x8014a6dc, 12),
    (0x8014a6e8, 12),
    (0x8014a728, 30),
    (0x8014a748, 19),
    (0x8014a75c, 24),
    (0x8014a774, 43),
    (0x8014a7a0, 19),
    (0x8014a7b4, 27),
    (0x8014a7d0, 20),
    (0x8014a7e4, 17),
    (0x8014a7f8, 12),
    (0x8014a804, 17),
    (0x8014a818, 4),
    (0x8014a81c, 11),
    (0x8014a828, 10),
    (0x8014a834, 11),
    (0x8014a840, 10),
    (0x8014a84c, 23),
    (0x8014a864, 24),
    (0x8014a87c, 19),
    (0x8014a890, 21),
    (0x8014a8a8, 27),
    (0x8014a8c4, 21),
    (0x8014a8dc, 50),
    (0x8014a910, 24),
    (0x80186888, 49),
    (0x801868b9, 41),
];

const LOGO_ALLOCATIONS: &[(u32, usize)] = &[
    (0x801ce044, 28),
    (0x801ce060, 25),
    (0x801ce07c, 26),
    (0x801ce098, 15),
    (0x801ce0a8, 21),
    (0x801ce214, 27),
    (0x801ce230, 12),
    (0x801ce23c, 25),
    (0x801ce28c, 8),
    (0x801ce294, 9),
    (0x801ce2a0, 7),
    (0x801ce2a8, 14),
    (0x801ce2b8, 6),
    (0x801ce300, 11),
    (0x801ce30c, 12),
    (0x801ce374, 22),
    (0x801ce38c, 15),
    (0x801ce39c, 24),
    (0x801ce3b4, 16),
    (0x801ce3c4, 22),
    (0x801ce3dc, 10),
    (0x801ce3e8, 10),
    (0x801ce3f4, 26),
    (0x801ce410, 26),
    (0x801ce42c, 12),
    (0x801ce438, 12),
    (0x801ce478, 30),
    (0x801ce498, 19),
    (0x801ce4ac, 24),
    (0x801ce4c4, 43),
    (0x801ce4f0, 19),
    (0x801ce504, 27),
    (0x801ce520, 20),
    (0x801ce534, 17),
    (0x801ce548, 12),
    (0x801ce554, 17),
    (0x801ce568, 4),
    (0x801ce56c, 11),
    (0x801ce578, 10),
    (0x801ce584, 11),
    (0x801ce590, 10),
    (0x801ce59c, 23),
    (0x801ce5b4, 24),
    (0x801ce5cc, 19),
    (0x801ce5e0, 21),
    (0x801ce5f8, 27),
    (0x801ce614, 21),
    (0x801ce62c, 50),
    (0x801ce660, 24),
    (0x801ce678, 26),
    (0x801ce694, 13),
    (0x801ce6a4, 14),
    (0x801ce6b4, 38),
    (0x801ce6dc, 53),
    (0x801ce714, 13),
];
const LOGO_PRINTF: u32 = 0x801d791c;
const LOGO_PRINTF_CALLBACK: u32 = 0x801da1bc;
const LOGO_CALLBACK_TEXT: &[u32] = &[
    0x801ce374, 0x801ce38c, 0x801ce39c, 0x801ce3b4, 0x801ce3c4, 0x801ce3dc, 0x801ce3e8, 0x801ce3f4,
    0x801ce410, 0x801ce42c, 0x801ce438, 0x801ce498, 0x801ce4ac, 0x801ce4c4, 0x801ce4f0, 0x801ce504,
    0x801ce520, 0x801ce534, 0x801ce548, 0x801ce554, 0x801ce568, 0x801ce56c, 0x801ce578, 0x801ce584,
    0x801ce590, 0x801ce59c, 0x801ce5b4, 0x801ce5cc, 0x801ce5e0, 0x801ce5f8, 0x801ce614,
];

struct Layout {
    build: &'static str,
    load: u32,
    size: usize,
    masked_sha256: &'static str,
    allocations: &'static [(u32, usize)],
}

impl Layout {
    fn offset(&self, address: u32) -> usize {
        (address - self.load) as usize + 0x800
    }

    fn recognizes(&self, data: &[u8]) -> bool {
        if data.len() != self.size || !is_executable(data) {
            return false;
        }
        let mut masked = data.to_vec();
        for &(address, capacity) in self.allocations {
            let offset = self.offset(address);
            masked[offset..offset + capacity].fill(0);
        }
        sha256_hex(&masked) == self.masked_sha256
    }
}

const LAYOUTS: &[Layout] = &[
    Layout {
        build: "US SLUS-00422 diagnostic ASCII",
        load: 0x80096800,
        size: 1445888,
        masked_sha256: "f3bde09968a27ff84e37c92226bc6867b99a84f8d006501af1cfc6eeb5a24138",
        allocations: SLUS_ALLOCATIONS,
    },
    Layout {
        build: "US SLUS-00422 logo diagnostic ASCII",
        load: 0x801ce000,
        size: 122880,
        masked_sha256: "4bcea15bf0395cc97212d2715a28c191ea8881db3cb58de415d3af18b587109e",
        allocations: LOGO_ALLOCATIONS,
    },
];

#[derive(Clone, Copy, Debug, Deserialize, Eq, PartialEq, Serialize)]
#[serde(rename_all = "kebab-case")]
pub(crate) enum Codec {
    AsciiLiteral,
    AsciiPrintfLiteral,
    AsciiBiosPrintfFormat,
    AsciiBiosPrintfTabFormat,
    AsciiU32HexFormat,
    AsciiU8TripleHexFormat,
}

impl Codec {
    pub(crate) fn as_str(self) -> &'static str {
        match self {
            Self::AsciiLiteral => "ascii-literal",
            Self::AsciiPrintfLiteral => "ascii-printf-literal",
            Self::AsciiBiosPrintfFormat => "ascii-bios-printf-format",
            Self::AsciiBiosPrintfTabFormat => "ascii-bios-printf-tab-format",
            Self::AsciiU32HexFormat => "ascii-u32-hex-format",
            Self::AsciiU8TripleHexFormat => "ascii-u8-triple-hex-format",
        }
    }

    pub(crate) fn encode(self, text: &str, maximum: usize) -> Result<Vec<u8>> {
        let bytes = text.as_bytes();
        if bytes.len() > maximum
            || !bytes.iter().all(|byte| {
                (0x20..=0x7e).contains(byte)
                    || *byte == b'\n'
                    || (self == Self::AsciiBiosPrintfTabFormat && *byte == b'\t')
            })
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
            Self::AsciiBiosPrintfFormat | Self::AsciiBiosPrintfTabFormat => {
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
        let token = ["%02x", "%04x", "%08x", "%3d", "%s", "%d"]
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
        0x801ce678 => &["%d"],
        0x801ce6b4 => &["%d", "%d", "%08x", "%08x"],
        0x801ce6dc => &["%d", "%d", "%d", "%d", "%d", "%d", "%d"],
        0x801ce714 => &["%s"],
        0x801ce374 => &["%d", "%d", "%d", "%d"],
        0x801ce38c => &["%d", "%d"],
        0x801ce39c => &["%3d", "%3d", "%d", "%d"],
        0x801ce3b4 => &["%3d", "%3d"],
        0x801ce3c4 => &["%d", "%d", "%d", "%d"],
        0x801ce3dc => &["%d"],
        0x801ce3e8 => &["%d"],
        0x801ce3f4 => &["%3d", "%3d", "%d", "%d"],
        0x801ce410 => &["%3d", "%3d", "%d", "%d"],
        0x801ce42c => &["%d"],
        0x801ce438 => &["%d"],
        0x801ce478 => &["%08x", "%08x"],
        0x801ce498 => &["%d"],
        0x801ce4ac => &["%d"],
        0x801ce4c4 => &["%d", "%d", "%d"],
        0x801ce4f0 => &["%d"],
        0x801ce504 => &["%08x"],
        0x801ce520 => &["%d"],
        0x801ce534 => &["%d"],
        0x801ce548 => &["%s"],
        0x801ce554 => &["%d", "%d", "%d", "%d"],
        0x801ce568 => &["%s"],
        0x801ce59c => &["%08x", "%d"],
        0x801ce5b4 => &["%08x", "%d"],
        0x801ce5cc => &["%08x"],
        0x801ce5e0 => &["%08x"],
        0x801ce5f8 => &["%08x"],
        0x801ce614 => &["%08x"],
        0x801ce62c => &["%d", "%08x", "%08x", "%08x"],
        0x801ce660 => &["%08x", "%08x", "%08x"],
        0x801ce044 => &["%04x"],
        0x801ce060 => &["%04x", "%04x"],
        0x801ce07c => &["%08x"],
        0x801ce098 => &["%d", "%08x"],
        0x801ce214 => &["%s", "%s", "%s", "%s"],
        0x801ce23c => &["%s", "%02x", "%02x"],
        0x801ce2a0 | 0x801ce2a8 => &["%s"],
        0x801ce300 => &["%08x"],
        0x80149fc0 => &["%s"],
        0x8014a120 => &["%04x"],
        0x8014a13c => &["%04x", "%04x"],
        0x8014a158 => &["%08x"],
        0x8014a174 => &["%d", "%08x"],
        0x8014a7f8 => &["%s"],
        0x8014a804 => &["%d", "%d", "%d", "%d"],
        0x8014a818 => &["%s"],
        0x8014a84c => &["%08x", "%d"],
        0x8014a864 => &["%08x", "%d"],
        0x8014a87c => &["%08x"],
        0x8014a890 => &["%08x"],
        0x8014a8a8 => &["%08x"],
        0x8014a8c4 => &["%08x"],
        0x8014a8dc => &["%d", "%08x", "%08x", "%08x"],
        0x8014a910 => &["%08x", "%08x", "%08x"],
        0x8014a728 => &["%08x", "%08x"],
        0x8014a748 => &["%d"],
        0x8014a75c => &["%d"],
        0x8014a774 => &["%d", "%d", "%d"],
        0x8014a7a0 => &["%d"],
        0x8014a7b4 => &["%08x"],
        0x8014a7d0 => &["%d"],
        0x8014a7e4 => &["%d"],
        0x8014a624 => &["%d", "%d", "%d", "%d"],
        0x8014a63c => &["%d", "%d"],
        0x8014a64c => &["%3d", "%3d", "%d", "%d"],
        0x8014a664 => &["%3d", "%3d"],
        0x8014a674 => &["%d", "%d", "%d", "%d"],
        0x8014a68c => &["%d"],
        0x8014a698 => &["%d"],
        0x8014a6a4 => &["%3d", "%3d", "%d", "%d"],
        0x8014a6c0 => &["%3d", "%3d", "%d", "%d"],
        0x8014a6dc => &["%d"],
        0x8014a6e8 => &["%d"],
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
    if LOGO_ALLOCATIONS.iter().any(|&(at, _)| at == address) {
        return match address {
            0x801ce0a8 | 0x801ce230 => Codec::AsciiPrintfLiteral,
            0x801ce28c | 0x801ce294 | 0x801ce2b8 | 0x801ce30c | 0x801ce56c | 0x801ce578
            | 0x801ce584 | 0x801ce590 | 0x801ce694 | 0x801ce6a4 => Codec::AsciiLiteral,
            0x801ce6b4 | 0x801ce6dc => Codec::AsciiBiosPrintfTabFormat,
            _ => Codec::AsciiBiosPrintfFormat,
        };
    }
    if BIOS_PRINTF_FORMATS.contains(&address) || GPU_PRINTF_CALLBACK_FORMATS.contains(&address) {
        return Codec::AsciiBiosPrintfFormat;
    }
    if BIOS_PRINTF_LITERALS.contains(&address) {
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
        0x801ce678 => vec![0x801d70ac],
        0x801ce694 => vec![0x801d72cc],
        0x801ce6a4 => vec![0x801d7364],
        0x801ce6b4 | 0x801ce6dc | 0x801ce714 => vec![0x801d7414],
        0x801ce374 => vec![0x801d354c, 0x801d3b24],
        0x801ce38c => vec![0x801d361c],
        0x801ce39c => vec![0x801d3b24],
        0x801ce3b4 => vec![0x801d3b24],
        0x801ce3c4 => vec![0x801d3b24],
        0x801ce3dc => vec![0x801d3b24],
        0x801ce3e8 => vec![0x801d3b24],
        0x801ce3f4 => vec![0x801d3ca4],
        0x801ce410 => vec![0x801d3ca4],
        0x801ce42c => vec![0x801d3ca4],
        0x801ce438 => vec![0x801d3ca4],
        0x801ce478 => vec![0x801d3d50],
        0x801ce498 => vec![0x801d3d50],
        0x801ce4ac => vec![0x801d3ed4],
        0x801ce4c4 => vec![0x801d3fe8],
        0x801ce4f0 => vec![0x801d404c],
        0x801ce504 => vec![0x801d4118],
        0x801ce520 => vec![0x801d4174],
        0x801ce534 => vec![0x801d4210],
        0x801ce548 => vec![0x801d427c],
        0x801ce554 => vec![0x801d427c],
        0x801ce568 => vec![0x801d427c],
        0x801ce56c => vec![0x801d43a4, 0x801d4438],
        0x801ce578 => vec![0x801d44d4],
        0x801ce584 => vec![0x801d4538],
        0x801ce590 => vec![0x801d459c],
        0x801ce59c => vec![0x801d4660],
        0x801ce5b4 => vec![0x801d4718],
        0x801ce5cc => vec![0x801d4810],
        0x801ce5e0 => vec![0x801d4884],
        0x801ce5f8 => vec![0x801d4988],
        0x801ce614 => vec![0x801d4adc],
        0x801ce62c => vec![0x801d6b70],
        0x801ce660 => vec![0x801d6b70],

        0x801ce044 | 0x801ce060 => vec![0x801cf36c],
        0x801ce07c | 0x801ce098 => vec![0x801cf980],
        0x801ce0a8 => vec![0x801cfc30],
        0x801ce214 => vec![0x801d0a2c, 0x801d0cac, 0x801d0f78, 0x801d182c],
        0x801ce230 | 0x801ce23c => vec![0x801d04a4],
        0x801ce28c => vec![0x801d0a2c],
        0x801ce294 => vec![0x801d0cac],
        0x801ce2a0 | 0x801ce2a8 | 0x801ce2b8 => vec![0x801d0f78],
        0x801ce300 => vec![0x801d1640],
        0x801ce30c => vec![0x801d182c],
        0x80149fc0 => vec![0x80168374, 0x80168608],
        0x80149fd0 => vec![0x80168374],
        0x80149fe0 => vec![0x80168608],
        0x80149ff4 => vec![0x80168608],
        0x8014a048 => vec![0x8016b38c],
        0x8014a06c => vec![0x8016b4b8],
        0x8014a0dc => vec![0x80174848],
        0x8014a120 => vec![0x80174b58],
        0x8014a13c => vec![0x80174b58],
        0x8014a158 => vec![0x8017516c],
        0x8014a174 => vec![0x8017516c],
        0x8014a184 => vec![0x801753ec],
        0x80186888 => vec![0x80178b78],
        0x801868b9 => vec![0x80178c18],
        0x8014a7f8 => vec![0x8017b438],
        0x8014a804 => vec![0x8017b438],
        0x8014a818 => vec![0x8017b438],
        0x8014a81c => vec![0x8017b560, 0x8017b5f4],
        0x8014a828 => vec![0x8017b690],
        0x8014a834 => vec![0x8017b6f4],
        0x8014a840 => vec![0x8017b758],
        0x8014a84c => vec![0x8017b81c],
        0x8014a864 => vec![0x8017b8d4],
        0x8014a87c => vec![0x8017b9cc],
        0x8014a890 => vec![0x8017ba40],
        0x8014a8a8 => vec![0x8017bb44],
        0x8014a8c4 => vec![0x8017bc98],
        0x8014a8dc => vec![0x8017dd2c],
        0x8014a910 => vec![0x8017dd2c],
        0x8014a728 => vec![0x8017af0c],
        0x8014a748 => vec![0x8017af0c],
        0x8014a75c => vec![0x8017b090],
        0x8014a774 => vec![0x8017b1a4],
        0x8014a7a0 => vec![0x8017b208],
        0x8014a7b4 => vec![0x8017b2d4],
        0x8014a7d0 => vec![0x8017b330],
        0x8014a7e4 => vec![0x8017b3cc],
        0x8014a624 => vec![0x8017a708, 0x8017ace0],
        0x8014a63c => vec![0x8017a7d8],
        0x8014a64c => vec![0x8017ace0],
        0x8014a664 => vec![0x8017ace0],
        0x8014a674 => vec![0x8017ace0],
        0x8014a68c => vec![0x8017ace0],
        0x8014a698 => vec![0x8017ace0],
        0x8014a6a4 => vec![0x8017ae60],
        0x8014a6c0 => vec![0x8017ae60],
        0x8014a6dc => vec![0x8017ae60],
        0x8014a6e8 => vec![0x8017ae60],
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

fn all_consumers(layout: &Layout) -> Vec<u32> {
    layout
        .allocations
        .iter()
        .flat_map(|&(address, _)| consumers(address))
        .collect::<BTreeSet<_>>()
        .into_iter()
        .collect()
}

fn diagnostic_references(address: u32) -> Option<Vec<Reference>> {
    let pairs: &[(u32, u32, Option<u32>)] = match address {
        0x801ce678 => &[(0x801d7188, 0x801d718c, Some(0x801d7190))],
        0x801ce694 => &[(0x801d731c, 0x801d7320, Some(0x801d7324))],
        0x801ce6a4 => &[(0x801d73b4, 0x801d73b8, Some(0x801d73bc))],
        0x801ce6b4 => &[(0x801d7430, 0x801d7434, Some(0x801d7484))],
        0x801ce6dc => &[(0x801d748c, 0x801d7490, Some(0x801d74d8))],
        0x801ce714 => &[(0x801d741c, 0x801d7420, Some(0x801d7428))],
        0x801ce374 => &[
            (0x801d3580, 0x801d3584, Some(0x801d35b4)),
            (0x801d35c4, 0x801d35c8, Some(0x801d3600)),
            (0x801d3c04, 0x801d3c08, Some(0x801d3c38)),
            (0x801d3c48, 0x801d3c4c, Some(0x801d3c88)),
        ],
        0x801ce38c => &[(0x801d362c, 0x801d3630, Some(0x801d3644))],
        0x801ce39c => &[(0x801d3b4c, 0x801d3b50, Some(0x801d3b54))],
        0x801ce3b4 => &[(0x801d3b6c, 0x801d3b70, Some(0x801d3b74))],
        0x801ce3c4 => &[(0x801d3b94, 0x801d3b98, Some(0x801d3b9c))],
        0x801ce3dc => &[(0x801d3bb0, 0x801d3bb4, Some(0x801d3bb8))],
        0x801ce3e8 => &[(0x801d3bcc, 0x801d3bd0, Some(0x801d3bd4))],
        0x801ce3f4 => &[(0x801d3ccc, 0x801d3cd0, Some(0x801d3cd4))],
        0x801ce410 => &[(0x801d3cf4, 0x801d3cf8, Some(0x801d3cfc))],
        0x801ce42c => &[(0x801d3d10, 0x801d3d14, Some(0x801d3d18))],
        0x801ce438 => &[(0x801d3d2c, 0x801d3d30, Some(0x801d3d34))],
        0x801ce478 => &[(0x801d3da0, 0x801d3da4, Some(0x801d3db8))],
        0x801ce498 => &[(0x801d3e84, 0x801d3e88, Some(0x801d3e98))],
        0x801ce4ac => &[(0x801d3f08, 0x801d3f0c, Some(0x801d3f1c))],
        0x801ce4c4 => &[(0x801d4024, 0x801d4028, Some(0x801d402c))],
        0x801ce4f0 => &[(0x801d4080, 0x801d4084, Some(0x801d4094))],
        0x801ce504 => &[(0x801d4140, 0x801d4144, Some(0x801d4148))],
        0x801ce520 => &[(0x801d41a0, 0x801d41a4, Some(0x801d41b4))],
        0x801ce534 => &[(0x801d4230, 0x801d4234, Some(0x801d4244))],
        0x801ce548 => &[(0x801d433c, 0x801d4340, Some(0x801d4360))],
        0x801ce554 => &[(0x801d4380, 0x801d4384, Some(0x801d4388))],
        0x801ce568 => &[(0x801d434c, 0x801d4350, Some(0x801d4360))],
        0x801ce56c => &[
            (0x801d43b0, 0x801d43b4, Some(0x801d43d4)),
            (0x801d4444, 0x801d4448, Some(0x801d4468)),
        ],
        0x801ce578 => &[(0x801d44e8, 0x801d44ec, Some(0x801d44f4))],
        0x801ce584 => &[(0x801d454c, 0x801d4550, Some(0x801d4558))],
        0x801ce590 => &[(0x801d45b8, 0x801d45bc, Some(0x801d45c4))],
        0x801ce59c => &[(0x801d4690, 0x801d4694, Some(0x801d469c))],
        0x801ce5b4 => &[(0x801d4740, 0x801d4744, Some(0x801d4758))],
        0x801ce5cc => &[(0x801d4830, 0x801d4834, Some(0x801d4844))],
        0x801ce5e0 => &[(0x801d48b4, 0x801d48b8, Some(0x801d48c8))],
        0x801ce5f8 => &[(0x801d49c0, 0x801d49c4, Some(0x801d49d8))],
        0x801ce614 => &[(0x801d4b0c, 0x801d4b10, Some(0x801d4b20))],
        0x801ce62c => &[(0x801d6bc8, 0x801d6bcc, Some(0x801d6c08))],
        0x801ce660 => &[(0x801d6c2c, 0x801d6c30, Some(0x801d6c34))],
        0x801ce044 => &[(0x801cf3b0, 0x801cf3b4, Some(0x801cf3b8))],
        0x801ce060 => &[(0x801cf4e4, 0x801cf4e8, Some(0x801cf4f4))],
        0x801ce07c => &[(0x801cfa94, 0x801cfa98, Some(0x801cfaa0))],
        0x801ce098 => &[(0x801cfaa8, 0x801cfaac, Some(0x801cfac8))],
        0x801ce0a8 => &[(0x801cfc98, 0x801cfc9c, Some(0x801cfca0))],
        0x801ce214 => &[
            (0x801d0b34, 0x801d0b38, Some(0x801d0b3c)),
            (0x801d0dbc, 0x801d0dc0, Some(0x801d0dc4)),
            (0x801d122c, 0x801d1230, Some(0x801d1234)),
            (0x801d1924, 0x801d1928, Some(0x801d192c)),
        ],
        0x801ce230 => &[(0x801d0684, 0x801d0688, Some(0x801d068c))],
        0x801ce23c => &[(0x801d06d0, 0x801d06d4, Some(0x801d06d8))],
        0x801ce28c => &[(0x801d0a80, 0x801d0a84, None)],
        0x801ce294 => &[(0x801d0d08, 0x801d0d0c, None)],
        0x801ce2a0 => &[(0x801d0fcc, 0x801d0fd0, Some(0x801d0fd4))],
        0x801ce2a8 => &[(0x801d1024, 0x801d1028, Some(0x801d102c))],
        0x801ce2b8 => &[(0x801d1168, 0x801d116c, None)],
        0x801ce300 => &[(0x801d1658, 0x801d165c, Some(0x801d1668))],
        0x801ce30c => &[(0x801d1874, 0x801d1878, None)],
        0x80149fc0 => &[
            (0x8016840c, 0x80168410, Some(0x8016841c)),
            (0x801686e0, 0x801686e4, Some(0x801686f0)),
            (0x80168774, 0x80168778, Some(0x80168784)),
        ],
        0x80149fd0 => &[(0x80168414, 0x80168418, Some(0x8016841c))],
        0x80149fe0 => &[(0x801686e8, 0x801686ec, Some(0x801686f0))],
        0x80149ff4 => &[(0x8016877c, 0x80168780, Some(0x80168784))],
        0x8014a048 => &[(0x8016b3c4, 0x8016b3c8, Some(0x8016b3cc))],
        0x8014a06c => &[(0x8016b5dc, 0x8016b5e0, Some(0x8016b5e4))],
        0x8014a0dc => &[(0x80174890, 0x80174894, Some(0x80174898))],
        0x8014a120 => &[(0x80174b9c, 0x80174ba0, Some(0x80174ba4))],
        0x8014a13c => &[(0x80174cd0, 0x80174cd4, Some(0x80174ce0))],
        0x8014a158 => &[(0x80175280, 0x80175284, Some(0x8017528c))],
        0x8014a174 => &[(0x80175294, 0x80175298, Some(0x801752b4))],
        0x8014a184 => &[(0x80175454, 0x80175458, Some(0x8017545c))],
        0x80186888 => &[(0x80178b94, 0x80178b9c, Some(0x80178b98))],
        0x801868b9 => &[(0x80178c30, 0x80178c38, Some(0x80178c34))],
        0x8014a7f8 => &[(0x8017b4f8, 0x8017b4fc, Some(0x8017b51c))],
        0x8014a804 => &[(0x8017b53c, 0x8017b540, Some(0x8017b544))],
        0x8014a818 => &[(0x8017b508, 0x8017b50c, Some(0x8017b51c))],
        0x8014a81c => &[
            (0x8017b56c, 0x8017b570, Some(0x8017b590)),
            (0x8017b600, 0x8017b604, Some(0x8017b624)),
        ],
        0x8014a828 => &[(0x8017b6a4, 0x8017b6a8, Some(0x8017b6b0))],
        0x8014a834 => &[(0x8017b708, 0x8017b70c, Some(0x8017b714))],
        0x8014a840 => &[(0x8017b774, 0x8017b778, Some(0x8017b780))],
        0x8014a84c => &[(0x8017b84c, 0x8017b850, Some(0x8017b858))],
        0x8014a864 => &[(0x8017b8fc, 0x8017b900, Some(0x8017b914))],
        0x8014a87c => &[(0x8017b9ec, 0x8017b9f0, Some(0x8017ba00))],
        0x8014a890 => &[(0x8017ba70, 0x8017ba74, Some(0x8017ba84))],
        0x8014a8a8 => &[(0x8017bb7c, 0x8017bb80, Some(0x8017bb94))],
        0x8014a8c4 => &[(0x8017bcc8, 0x8017bccc, Some(0x8017bcdc))],
        0x8014a8dc => &[(0x8017dd84, 0x8017dd88, Some(0x8017ddc4))],
        0x8014a910 => &[(0x8017dde8, 0x8017ddec, Some(0x8017ddf0))],
        0x8014a728 => &[(0x8017af5c, 0x8017af60, Some(0x8017af74))],
        0x8014a748 => &[(0x8017b040, 0x8017b044, Some(0x8017b054))],
        0x8014a75c => &[(0x8017b0c4, 0x8017b0c8, Some(0x8017b0d8))],
        0x8014a774 => &[(0x8017b1e0, 0x8017b1e4, Some(0x8017b1e8))],
        0x8014a7a0 => &[(0x8017b23c, 0x8017b240, Some(0x8017b250))],
        0x8014a7b4 => &[(0x8017b2fc, 0x8017b300, Some(0x8017b304))],
        0x8014a7d0 => &[(0x8017b35c, 0x8017b360, Some(0x8017b370))],
        0x8014a7e4 => &[(0x8017b3ec, 0x8017b3f0, Some(0x8017b400))],
        0x8014a624 => &[
            (0x8017a73c, 0x8017a740, Some(0x8017a770)),
            (0x8017a780, 0x8017a784, Some(0x8017a7bc)),
            (0x8017adc0, 0x8017adc4, Some(0x8017adf4)),
            (0x8017ae04, 0x8017ae08, Some(0x8017ae44)),
        ],
        0x8014a63c => &[(0x8017a7e8, 0x8017a7ec, Some(0x8017a800))],
        0x8014a64c => &[(0x8017ad08, 0x8017ad0c, Some(0x8017ad10))],
        0x8014a664 => &[(0x8017ad28, 0x8017ad2c, Some(0x8017ad30))],
        0x8014a674 => &[(0x8017ad50, 0x8017ad54, Some(0x8017ad58))],
        0x8014a68c => &[(0x8017ad6c, 0x8017ad70, Some(0x8017ad74))],
        0x8014a698 => &[(0x8017ad88, 0x8017ad8c, Some(0x8017ad90))],
        0x8014a6a4 => &[(0x8017ae88, 0x8017ae8c, Some(0x8017ae90))],
        0x8014a6c0 => &[(0x8017aeb0, 0x8017aeb4, Some(0x8017aeb8))],
        0x8014a6dc => &[(0x8017aecc, 0x8017aed0, Some(0x8017aed4))],
        0x8014a6e8 => &[(0x8017aee8, 0x8017aeec, Some(0x8017aef0))],
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
    BiosPrintfCallback {
        callback_word: u32,
        initial_stub: u32,
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
    if LOGO_CALLBACK_TEXT.contains(&address) {
        return Output::BiosPrintfCallback {
            callback_word: LOGO_PRINTF_CALLBACK,
            initial_stub: LOGO_PRINTF,
            vector: 0xa0,
            selector: 0x3f,
        };
    }
    if LOGO_ALLOCATIONS.iter().any(|&(at, _)| at == address) {
        return Output::BiosPrintf {
            stub: LOGO_PRINTF,
            vector: 0xa0,
            selector: 0x3f,
        };
    }
    if GPU_PRINTF_CALLBACK_FORMATS.contains(&address) || GPU_PRINTF_ARGUMENTS.contains(&address) {
        return Output::BiosPrintfCallback {
            callback_word: GPU_PRINTF_CALLBACK,
            initial_stub: BIOS_PRINTF,
            vector: 0xa0,
            selector: 0x3f,
        };
    }
    if BIOS_PUTS_LABELS.contains(&address) {
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
        || BIOS_PRINTF_ARGUMENTS.contains(&address)
        || BIOS_PRINTF_LITERALS.contains(&address)
        || BIOS_PRINTF_FORMATS.contains(&address)
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
        if matches!(
            self.codec,
            Codec::AsciiBiosPrintfFormat | Codec::AsciiBiosPrintfTabFormat
        ) && printf_directives(&self.text)? != self.format_directives
        {
            return invalid("BIOS printf directives must retain their original bytes and order");
        }
        Ok(bytes)
    }
}

pub fn is_executable(data: &[u8]) -> bool {
    data.starts_with(b"PS-X EXE")
}

/// Recognition binds all bytes outside the documented editable allocations.
pub fn supports(data: &[u8]) -> bool {
    LAYOUTS.iter().any(|layout| layout.recognizes(data))
}

pub fn extract(path: &Path, data: &[u8]) -> Result<Document> {
    let layout = LAYOUTS.iter().find(|layout| layout.recognizes(data)).ok_or_else(|| {
        Error::Invalid(
            "unsupported executable layout; only the pinned US diagnostic strings are supported".into(),
        )
    })?;
    let mut strings = Vec::new();
    for &(address, capacity) in layout.allocations {
        let file_offset = layout.offset(address);
        let bytes = &data[file_offset..file_offset + capacity];
        let end = bytes
            .iter()
            .position(|byte| *byte == 0)
            .ok_or_else(|| Error::Invalid(format!("unterminated ASCII string at {address:#x}")))?;
        let codec = codec(address);
        let text = std::str::from_utf8(&bytes[..end])
            .map_err(|error| Error::Invalid(error.to_string()))?;
        let references = if let Some(references) = diagnostic_references(address) {
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
                    let at = layout.offset(*reference);
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
        build: layout.build.into(),
        load_address: layout.load,
        consumers: all_consumers(layout),
        strings,
    })
}

pub fn parse(data: &[u8]) -> Result<Document> {
    let document: Document =
        serde_json::from_slice(data).map_err(|error| Error::Invalid(error.to_string()))?;
    let layout = LAYOUTS
        .iter()
        .find(|layout| document.load_address == layout.load && document.build == layout.build)
        .ok_or_else(|| Error::Invalid("invalid executable text document identity".into()))?;
    if document.schema != SCHEMA
        || document.consumers != all_consumers(layout)
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
            || !layout.allocations.contains(&(text.address, text.capacity))
            || text.file_offset != layout.offset(text.address)
            || text.codec != codec(text.address)
            || text.consumers != consumers(text.address)
            || text.output != output(text.address)
            || text.format_directives != format_directives(text.address)
        {
            return invalid("unknown or duplicate executable string allocation");
        }
        text.encode()?;
    }
    if seen.len() != layout.allocations.len() {
        return invalid("executable text document must retain every supported allocation");
    }
    Ok(document)
}

pub fn rebuild(path: &Path, data: &[u8], document: &Document) -> Result<Vec<u8>> {
    let original = extract(path, data)?;
    if std::fs::canonicalize(&document.source)? != std::fs::canonicalize(path)?
        || document.source_sha256 != original.source_sha256
        || document.build != original.build
        || document.load_address != original.load_address
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
