//! Source-bound text allocations inside pinned US overlay code payloads.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result, invalid, literal_byte, literal_char};
use crate::records::{decode_name, encode_name};

pub const SCHEMA: &str = "bof3.inline-text/v6";

#[derive(Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub(crate) enum Codec {
    GameLiteral,
    GameFixedLiteral,
    GameCountedLiteral,
    GameGlyphs,
    AsciiDecimalFormat,
    AsciiSignedDecimalPairFormat,
    AsciiU8DecimalFormat,
    AsciiU7DecimalFormat,
    AsciiU16DecimalFormat,
}

impl Codec {
    pub(crate) fn encode(self, text: &str, capacity: usize) -> Result<Vec<u8>> {
        if matches!(
            self,
            Self::GameLiteral | Self::GameFixedLiteral | Self::GameCountedLiteral
        ) {
            return encode_name(text, capacity);
        }
        if self == Self::GameGlyphs {
            return encode_glyphs(text, capacity);
        }
        if self != Self::AsciiDecimalFormat {
            self.max_formatted_size(text)?;
            if text.len() > capacity {
                return invalid("ASCII decimal format exceeds its allocation");
            }
            return Ok(text.as_bytes().to_vec());
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

    pub(crate) fn max_formatted_size(self, text: &str) -> Result<Option<usize>> {
        let (arguments, limit): (&[usize], usize) = match self {
            Self::AsciiSignedDecimalPairFormat => (&[4, 4], 9),
            Self::AsciiU8DecimalFormat | Self::AsciiU7DecimalFormat => (&[3], 3),
            Self::AsciiU16DecimalFormat => (&[5], 5),
            _ => return Ok(None),
        };
        let size = decimal_format_size(text, arguments)?;
        if size > limit {
            return invalid(format!(
                "ASCII decimal format exceeds its {limit}-byte output bound"
            ));
        }
        Ok(Some(size))
    }

    fn decode(self, bytes: &[u8], file_offset: usize) -> Result<String> {
        if self == Self::GameCountedLiteral {
            let (&length, field) = bytes
                .split_last()
                .ok_or_else(|| Error::Invalid("counted literal has no length byte".into()))?;
            let length = usize::from(length);
            if length > field.len()
                || field[..length].contains(&0)
                || field[length..].iter().any(|byte| *byte != 0)
            {
                return invalid("counted literal length or padding does not match its field");
            }
            return decode_name(&field[..length], file_offset);
        }
        if matches!(self, Self::GameLiteral | Self::GameFixedLiteral) {
            return decode_name(bytes, file_offset);
        }
        if self == Self::GameGlyphs {
            return decode_glyphs(bytes, file_offset);
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
            Self::GameFixedLiteral => "game-fixed-literal",
            Self::GameCountedLiteral => "game-counted-literal",
            Self::GameGlyphs => "game-glyphs",
            Self::AsciiDecimalFormat => "ascii-decimal-format",
            Self::AsciiSignedDecimalPairFormat => "ascii-signed-decimal-pair-format",
            Self::AsciiU8DecimalFormat => "ascii-u8-decimal-format",
            Self::AsciiU7DecimalFormat => "ascii-u7-decimal-format",
            Self::AsciiU16DecimalFormat => "ascii-u16-decimal-format",
        }
    }

    pub(crate) const fn terminator_bytes(self) -> usize {
        match self {
            Self::GameFixedLiteral | Self::GameCountedLiteral => 0,
            _ => 1,
        }
    }

    pub(crate) const fn length_bytes(self) -> usize {
        match self {
            Self::GameCountedLiteral => 1,
            _ => 0,
        }
    }

    pub(crate) fn max_encoded_size(self, capacity: usize) -> Result<usize> {
        capacity
            .checked_sub(self.terminator_bytes() + self.length_bytes())
            .ok_or_else(|| Error::Invalid("inline allocation cannot hold its metadata".into()))
    }

    pub(crate) fn commands(self, text: &str, capacity: usize) -> Result<Vec<GlyphCommand>> {
        if self != Self::GameGlyphs {
            return Ok(Vec::new());
        }
        let bytes = self.encode(text, capacity)?;
        let mut commands = Vec::new();
        let mut offset = 0;
        while offset < bytes.len() {
            if bytes[offset] == 0x15 {
                commands.push(GlyphCommand {
                    offset,
                    code: 0x15,
                    name: "glyph",
                    operands: [bytes[offset + 1]],
                });
                offset += 2;
            } else {
                offset += 1;
            }
        }
        Ok(commands)
    }
}

fn decimal_format_size(text: &str, arguments: &[usize]) -> Result<usize> {
    if !text.bytes().all(|byte| (0x20..=0x7e).contains(&byte)) {
        return invalid("ASCII decimal format requires printable ASCII");
    }
    let mut bytes = text.bytes().peekable();
    let mut argument = 0;
    let mut size = 0;
    while let Some(byte) = bytes.next() {
        if byte != b'%' {
            size += 1;
            continue;
        }
        let padded = bytes.peek() == Some(&b'0');
        if padded {
            bytes.next();
        }
        let width = match bytes.peek().copied() {
            Some(b'1'..=b'4') => usize::from(bytes.next().unwrap() - b'0'),
            _ if !padded => 0,
            _ => return invalid("zero padding requires a decimal width from 1 to 4"),
        };
        if bytes.next() != Some(b'd') || argument >= arguments.len() {
            return invalid(
                "decimal format requires its ordered %d arguments and optional width 1..4",
            );
        }
        size += arguments[argument].max(width);
        argument += 1;
    }
    if argument != arguments.len() {
        return invalid("decimal format must retain every argument");
    }
    Ok(size)
}

#[derive(Serialize)]
pub(crate) struct GlyphCommand {
    offset: usize,
    code: u8,
    name: &'static str,
    operands: [u8; 1],
}

fn encode_glyphs(mut text: &str, capacity: usize) -> Result<Vec<u8>> {
    let mut bytes = Vec::new();
    while !text.is_empty() {
        if let Some(tail) = text.strip_prefix("{glyph(0x") {
            let (operand, remaining) = tail
                .split_once(")}")
                .ok_or_else(|| Error::Invalid("glyph command must use {glyph(0xNN)}".into()))?;
            if operand.len() != 2 || !operand.bytes().all(|byte| byte.is_ascii_hexdigit()) {
                return invalid("glyph operand must contain exactly two hexadecimal digits");
            }
            let operand = u8::from_str_radix(operand, 16)
                .map_err(|error| Error::Invalid(error.to_string()))?;
            if operand == 0 {
                return invalid("NUL glyph operand would truncate the BIOS string append");
            }
            bytes.extend_from_slice(&[0x15, operand]);
            text = remaining;
        } else {
            let character = text.chars().next().unwrap();
            bytes.push(literal_byte(character).ok_or_else(|| {
                Error::Invalid(format!("unsupported inline glyph character {character:?}"))
            })?);
            text = &text[character.len_utf8()..];
        }
        if bytes.len() > capacity {
            return invalid(format!(
                "inline glyph text exceeds its {capacity}-byte field"
            ));
        }
    }
    Ok(bytes)
}

fn decode_glyphs(bytes: &[u8], file_offset: usize) -> Result<String> {
    let end = bytes.iter().position(|byte| *byte == 0).ok_or_else(|| {
        Error::Invalid(format!(
            "unterminated inline glyph text at {file_offset:#x}"
        ))
    })?;
    if bytes[end..].iter().any(|byte| *byte != 0) {
        return invalid("nonzero bytes after inline glyph text terminator");
    }
    let mut text = String::new();
    let mut offset = 0;
    while offset < end {
        if bytes[offset] == 0x15 {
            if offset + 1 >= end {
                return invalid("inline glyph command has no nonzero operand before NUL");
            }
            text.push_str(&format!("{{glyph(0x{:02X})}}", bytes[offset + 1]));
            offset += 2;
        } else {
            text.push(literal_char(bytes[offset]).ok_or_else(|| {
                Error::Invalid(format!(
                    "unknown inline glyph byte at {:#x}",
                    file_offset + offset
                ))
            })?);
            offset += 1;
        }
    }
    Ok(text)
}

struct Allocation {
    address: u32,
    capacity: usize,
    reference: Option<u32>,
    renderer: Option<u32>,
    codec: Codec,
}

impl Allocation {
    const fn fixed_literal(address: u32, capacity: usize) -> Self {
        Self {
            address,
            capacity,
            reference: None,
            renderer: Some(0x8014f800),
            codec: Codec::GameFixedLiteral,
        }
    }

    const fn counted(address: u32) -> Self {
        Self {
            address,
            capacity: 8,
            reference: None,
            renderer: Some(0x8014fc90),
            codec: Codec::GameCountedLiteral,
        }
    }

    const fn literal(address: u32, capacity: usize, reference: Option<u32>, renderer: u32) -> Self {
        Self {
            address,
            capacity,
            reference,
            renderer: Some(renderer),
            codec: Codec::GameLiteral,
        }
    }

    const fn decimal(address: u32, capacity: usize) -> Self {
        Self {
            address,
            capacity,
            reference: None,
            renderer: Some(0x80150098),
            codec: Codec::AsciiDecimalFormat,
        }
    }

    const fn bounded_decimal(
        address: u32,
        capacity: usize,
        codec: Codec,
        renderer: Option<u32>,
    ) -> Self {
        Self {
            address,
            capacity,
            reference: None,
            renderer,
            codec,
        }
    }

    const fn glyphs(address: u32, capacity: usize) -> Self {
        Self {
            address,
            capacity,
            reference: None,
            renderer: Some(0x8014f800),
            codec: Codec::GameGlyphs,
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
        name: "US AREA030 panel text",
        load: 0x801d0c00,
        size: 75285,
        sha256: "f1ff752e7cac1a5f198aba917c8072910bc1be1625eb1d14af128fa7ca915205",
        consumers: &[0x801e1004, 0x801e1320],
        allocations: &[
            Allocation::bounded_decimal(
                0x801d0c38,
                4,
                Codec::AsciiU16DecimalFormat,
                Some(0x8014f800),
            ),
            Allocation::bounded_decimal(
                0x801d0c8c,
                4,
                Codec::AsciiU16DecimalFormat,
                Some(0x8014f800),
            ),
            Allocation::fixed_literal(0x801e28c4, 12),
            Allocation::fixed_literal(0x801e28d0, 12),
        ],
    },
    Layout {
        name: "US AREA008 decimal formats",
        load: 0x801f2c00,
        size: 10232,
        sha256: "ff37a43f66ba97f97f7c4c7f1a9020a614e95e48e4629fd6c294981945c97237",
        consumers: &[0x801f3c2c, 0x801f3d18, 0x801f437c, 0x801f43c8],
        allocations: &[
            Allocation::bounded_decimal(
                0x801f2c04,
                10,
                Codec::AsciiSignedDecimalPairFormat,
                Some(0x8014ff0c),
            ),
            Allocation::bounded_decimal(
                0x801f2c10,
                4,
                Codec::AsciiU8DecimalFormat,
                Some(0x8014ff0c),
            ),
            Allocation::bounded_decimal(0x801f2c14, 3, Codec::AsciiU7DecimalFormat, None),
        ],
    },
    Layout {
        name: "US START and STATUS labels and default names",
        load: 0x801d0c00,
        size: 117986,
        sha256: "635abc5e22528925464ee7795053eb9d514cd0f7413ab75cfb96ae33748a105a",
        consumers: &[0x801d15c0, 0x801dcf50],
        allocations: &[
            Allocation::counted(0x801d0c7c),
            Allocation::counted(0x801d0c90),
            Allocation::counted(0x801d0ca4),
            Allocation::counted(0x801d0cb8),
            Allocation::counted(0x801d0ccc),
            Allocation::counted(0x801d0ce0),
            Allocation::counted(0x801d0cf4),
            Allocation::counted(0x801d0d08),
            Allocation::counted(0x801d0d1c),
            Allocation::counted(0x801d0d30),
            Allocation::fixed_literal(0x801eb514, 5),
            Allocation::fixed_literal(0x801eb5b8, 5),
            Allocation::fixed_literal(0x801eb65c, 5),
            Allocation::fixed_literal(0x801eb700, 5),
            Allocation::fixed_literal(0x801eb7a4, 5),
            Allocation::fixed_literal(0x801eb848, 5),
            Allocation::fixed_literal(0x801eb8ec, 5),
            Allocation::fixed_literal(0x801eb990, 5),
        ],
    },
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
        name: "US BATE labels",
        load: 0x801d0c00,
        size: 33864,
        sha256: "c4d996aa3fb858d1c8c42342bfa21ad0fc7ae6aa964dd02c48cb4f26ace3c2db",
        consumers: &[0x801d3fb0, 0x801d6f84, 0x801d7600, 0x801d7e8c],
        allocations: &[
            Allocation::counted(0x801d0c50),
            Allocation::counted(0x801d0c64),
            Allocation::counted(0x801d0c78),
            Allocation::counted(0x801d0c8c),
            Allocation::counted(0x801d0ca0),
            Allocation::counted(0x801d0cb4),
            Allocation::counted(0x801d0cc8),
            Allocation::counted(0x801d0cdc),
            Allocation::counted(0x801d0cf0),
            Allocation::counted(0x801d0d04),
            Allocation::literal(0x801d8e24, 5, Some(0x801d8e4c), 0x8014f800),
            Allocation::literal(0x801d8e2c, 7, Some(0x801d8e50), 0x8014f800),
            Allocation::literal(0x801d8e34, 6, Some(0x801d8e54), 0x8014f800),
            Allocation::literal(0x801d8e3c, 7, Some(0x801d8e58), 0x8014f800),
            Allocation::literal(0x801d8e44, 6, Some(0x801d8e5c), 0x8014f800),
            Allocation::literal(0x801d8e60, 5, Some(0x801d8e80), 0x8014f800),
            Allocation::literal(0x801d8e68, 7, Some(0x801d8e84), 0x8014f800),
            Allocation::literal(0x801d8e70, 7, Some(0x801d8e88), 0x8014f800),
            Allocation::literal(0x801d8e78, 6, Some(0x801d8e8c), 0x8014f800),
            Allocation::literal(0x801d8e90, 5, Some(0x801d8eb8), 0x8014f800),
            Allocation::literal(0x801d8e98, 7, Some(0x801d8ebc), 0x8014f800),
            Allocation::literal(0x801d8ea0, 6, Some(0x801d8ec0), 0x8014f800),
            Allocation::literal(0x801d8ea8, 7, Some(0x801d8ec4), 0x8014f800),
            Allocation::literal(0x801d8eb0, 6, Some(0x801d8ec8), 0x8014f800),
        ],
    },
    Layout {
        name: "US SHISU labels",
        load: 0x801d0c00,
        size: 38196,
        sha256: "2696cb852070d78a63f2e2b9784bf1215ae3984a3412166d551715cb623baedf",
        consumers: &[0x801d4f58, 0x801d7f2c, 0x801d85a8, 0x801d8e34],
        allocations: &[
            Allocation::counted(0x801d0c34),
            Allocation::counted(0x801d0c48),
            Allocation::counted(0x801d0c5c),
            Allocation::counted(0x801d0c70),
            Allocation::counted(0x801d0c84),
            Allocation::counted(0x801d0c98),
            Allocation::counted(0x801d0cac),
            Allocation::counted(0x801d0cc0),
            Allocation::counted(0x801d0cd4),
            Allocation::counted(0x801d0ce8),
            Allocation::literal(0x801d9e48, 5, Some(0x801d9e70), 0x8014f800),
            Allocation::literal(0x801d9e50, 7, Some(0x801d9e74), 0x8014f800),
            Allocation::literal(0x801d9e58, 6, Some(0x801d9e78), 0x8014f800),
            Allocation::literal(0x801d9e60, 7, Some(0x801d9e7c), 0x8014f800),
            Allocation::literal(0x801d9e68, 6, Some(0x801d9e80), 0x8014f800),
            Allocation::literal(0x801d9e84, 5, Some(0x801d9ea4), 0x8014f800),
            Allocation::literal(0x801d9e8c, 7, Some(0x801d9ea8), 0x8014f800),
            Allocation::literal(0x801d9e94, 7, Some(0x801d9eac), 0x8014f800),
            Allocation::literal(0x801d9e9c, 6, Some(0x801d9eb0), 0x8014f800),
            Allocation::literal(0x801d9eb4, 5, Some(0x801d9edc), 0x8014f800),
            Allocation::literal(0x801d9ebc, 7, Some(0x801d9ee0), 0x8014f800),
            Allocation::literal(0x801d9ec4, 6, Some(0x801d9ee4), 0x8014f800),
            Allocation::literal(0x801d9ecc, 7, Some(0x801d9ee8), 0x8014f800),
            Allocation::literal(0x801d9ed4, 6, Some(0x801d9eec), 0x8014f800),
        ],
    },
    Layout {
        name: "US SHOP labels",
        load: 0x801d0c00,
        size: 87660,
        sha256: "88846e97d2f8e31ac94627ac123fbf23e72920bc288d63e5f5034ae612c45199",
        consumers: &[0x801d9a58, 0x801dca2c, 0x801dd0a8, 0x801dd934],
        allocations: &[
            Allocation::counted(0x801d0d14),
            Allocation::counted(0x801d0d28),
            Allocation::counted(0x801d0d3c),
            Allocation::counted(0x801d0d50),
            Allocation::counted(0x801d0d64),
            Allocation::counted(0x801d0d78),
            Allocation::counted(0x801d0d8c),
            Allocation::counted(0x801d0da0),
            Allocation::counted(0x801d0db4),
            Allocation::counted(0x801d0dc8),
            Allocation::literal(0x801e5b20, 5, Some(0x801e5b48), 0x8014f800),
            Allocation::literal(0x801e5b28, 7, Some(0x801e5b4c), 0x8014f800),
            Allocation::literal(0x801e5b30, 6, Some(0x801e5b50), 0x8014f800),
            Allocation::literal(0x801e5b38, 7, Some(0x801e5b54), 0x8014f800),
            Allocation::literal(0x801e5b40, 6, Some(0x801e5b58), 0x8014f800),
            Allocation::literal(0x801e5b5c, 5, Some(0x801e5b7c), 0x8014f800),
            Allocation::literal(0x801e5b64, 7, Some(0x801e5b80), 0x8014f800),
            Allocation::literal(0x801e5b6c, 7, Some(0x801e5b84), 0x8014f800),
            Allocation::literal(0x801e5b74, 6, Some(0x801e5b88), 0x8014f800),
            Allocation::literal(0x801e5b8c, 5, Some(0x801e5bb4), 0x8014f800),
            Allocation::literal(0x801e5b94, 7, Some(0x801e5bb8), 0x8014f800),
            Allocation::literal(0x801e5b9c, 6, Some(0x801e5bbc), 0x8014f800),
            Allocation::literal(0x801e5ba4, 7, Some(0x801e5bc0), 0x8014f800),
            Allocation::literal(0x801e5bac, 6, Some(0x801e5bc4), 0x8014f800),
        ],
    },
    Layout {
        name: "US battle messages",
        load: 0x801d0c00,
        size: 118224,
        sha256: "1d1ca57e123bc939b671491799d34b8a871c0ee6a00a8c74a5bcfd614f9bd8f2",
        consumers: &[0x801de94c, 0x801de560, 0x801deaa4, 0x801deac4, 0x801e9538],
        // Indexed records and direct code references, rather than a pointer table.
        // Row 1 appends at most five encoded bytes. Rows 3/4/5 remain unsupported.
        allocations: &[
            Allocation::literal(0x801eb000, 13, None, 0x8014f800),
            Allocation::glyphs(0x801eb00d, 6),
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

impl Document {
    pub(crate) fn glyph_count(&self) -> Result<usize> {
        self.strings.iter().try_fold(0, |count, row| {
            let maximum = row.codec.max_encoded_size(row.capacity)?;
            Ok(count + row.codec.commands(&row.text, maximum)?.len())
        })
    }
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Text {
    index: usize,
    pub address: u32,
    pub file_offset: usize,
    payload_offset: usize,
    pub capacity: usize,
    pub renderer: Option<u32>,
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
            if codec.terminator_bytes() != 0 && !bytes.contains(&0) {
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
        codec.encode(&row.text, codec.max_encoded_size(capacity)?)?;
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
        let maximum = row.codec.max_encoded_size(row.capacity)?;
        let bytes = row.codec.encode(&row.text, maximum)?;
        let span = &mut output[row.file_offset..row.file_offset + row.capacity];
        span.fill(0);
        span[..bytes.len()].copy_from_slice(&bytes);
        if row.codec == Codec::GameCountedLiteral {
            span[row.capacity - 1] = bytes.len() as u8;
        }
    }
    Ok(output)
}

pub fn render(document: &Document) -> Result<Vec<u8>> {
    serde_json::to_vec_pretty(document).map_err(|error| Error::Invalid(error.to_string()))
}
