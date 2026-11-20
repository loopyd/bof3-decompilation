//! Source-bound text allocations inside pinned US overlay code payloads.

use std::collections::BTreeSet;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result, invalid, literal_byte, literal_char};
use crate::records::{decode_name, encode_name};

pub const SCHEMA: &str = "bof3.inline-text/v43";

#[derive(Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub(crate) enum Codec {
    GameLiteral,
    GameFixedLiteral,
    GameExactLiteral,
    GameKeyboardCharacter,
    GameCountedLiteral,
    GameGlyphs,
    ShiftJisDigit,
    ShiftJisTitleFormat,
    AsciiLiteral,
    AsciiCreditsLiteral,
    AsciiFormatLiteral,
    AsciiDigitGlyphLiteral,
    AsciiDashGlyphLiteral,
    AsciiDecimalFormat,
    AsciiI32DecimalFormat,
    AsciiI32WideDecimalFormat,
    AsciiI16PlusOneDecimalFormat,
    AsciiI8DecimalFormat,
    AsciiSignedDecimalPairFormat,
    AsciiI8PlusOneU8DecimalPairFormat,
    AsciiU8DecimalFormat,
    AsciiU8PlusOneDecimalFormat,
    AsciiU8DecimalFieldFormat,
    AsciiU8DecimalPairFormat,
    AsciiU8DecimalTripleFormat,
    AsciiU7DecimalFormat,
    AsciiU16DecimalFormat,
    AsciiU16DecimalFieldFormat,
    AsciiPriceDecimalFormat,
    AsciiTotalDecimalFormat,
}

impl Codec {
    pub(crate) fn encode(self, text: &str, capacity: usize) -> Result<Vec<u8>> {
        if self == Self::ShiftJisTitleFormat {
            return encode_title(text, capacity);
        }
        if self == Self::ShiftJisDigit {
            let mut characters = text.chars();
            let digit = match (characters.next(), characters.next()) {
                (Some(digit @ '０'..='９'), None) if capacity == 2 => {
                    (digit as u32 - '０' as u32) as u8
                }
                _ => {
                    return invalid("Shift-JIS digit requires exactly one full-width digit ０..９");
                }
            };
            return Ok(vec![0x82, 0x4f + digit]);
        }
        if self == Self::AsciiCreditsLiteral {
            if text.len() != capacity
                || !text
                    .bytes()
                    .all(|byte| byte.is_ascii_alphabetic() || byte == b' ')
            {
                return invalid(format!(
                    "credits text requires exactly {capacity} ASCII letters or spaces; pad shorter wording with spaces"
                ));
            }
            return Ok(text.as_bytes().to_vec());
        }
        if self == Self::GameExactLiteral {
            let bytes = encode_name(text, capacity)?;
            if bytes.len() != capacity {
                return invalid(format!(
                    "counted text requires exactly {capacity} literal bytes; pad shorter wording with spaces"
                ));
            }
            return Ok(bytes);
        }
        if self == Self::GameKeyboardCharacter {
            let bytes = encode_name(text, capacity)?;
            if capacity != 1 || bytes.len() != 1 || !keyboard_character(bytes[0]) {
                return invalid("keyboard cell requires exactly one active US keyboard character");
            }
            return Ok(bytes);
        }
        if matches!(
            self,
            Self::GameLiteral | Self::GameFixedLiteral | Self::GameCountedLiteral
        ) {
            return encode_name(text, capacity);
        }
        if self == Self::GameGlyphs {
            return encode_glyphs(text, capacity);
        }
        if self == Self::AsciiLiteral {
            if text.len() > capacity || !text.bytes().all(|byte| (0x20..=0x7e).contains(&byte)) {
                return invalid(
                    "ASCII literal must fit its allocation and contain only printable ASCII",
                );
            }
            return Ok(text.as_bytes().to_vec());
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
        if self == Self::ShiftJisTitleFormat {
            return Ok(Some(encode_title(text, 64)?.len()));
        }
        if self == Self::AsciiDashGlyphLiteral {
            if text.is_empty() || !text.bytes().all(|byte| byte == b' ' || byte == b'-') {
                return invalid("dash glyph literal requires one or more spaces or '-' glyphs");
            }
            return Ok(Some(text.len()));
        }
        if self == Self::AsciiDigitGlyphLiteral {
            if text.is_empty()
                || !text
                    .bytes()
                    .all(|byte| byte == b' ' || (b'0'..=b':').contains(&byte))
            {
                return invalid(
                    "digit glyph literal requires spaces, digits or the original ':' glyph index",
                );
            }
            return Ok(Some(text.len()));
        }
        if self == Self::AsciiFormatLiteral && (text.is_empty() || text.contains('%')) {
            return invalid(
                "ASCII format literal must be nonempty and contain no percent sequences",
            );
        }
        let (arguments, limit, max_width): (&[usize], usize, usize) = match self {
            Self::AsciiFormatLiteral => (&[], 7, 4),
            Self::AsciiI32DecimalFormat => (&[11], 11, 7),
            Self::AsciiI32WideDecimalFormat => (&[11], 11, 8),
            Self::AsciiI16PlusOneDecimalFormat => (&[6], 6, 4),
            Self::AsciiI8DecimalFormat => (&[4], 4, 4),
            Self::AsciiSignedDecimalPairFormat => (&[4, 4], 9, 4),
            Self::AsciiI8PlusOneU8DecimalPairFormat => (&[4, 3], 8, 4),
            Self::AsciiU8DecimalPairFormat => (&[3, 3], 7, 4),
            Self::AsciiU8DecimalTripleFormat => (&[3, 3, 3], 16, 4),
            Self::AsciiU8DecimalFormat
            | Self::AsciiU8PlusOneDecimalFormat
            | Self::AsciiU7DecimalFormat => (&[3], 3, 4),
            Self::AsciiU8DecimalFieldFormat => (&[3], 4, 4),
            Self::AsciiU16DecimalFormat => (&[5], 5, 4),
            Self::AsciiU16DecimalFieldFormat => (&[5], 9, 4),
            Self::AsciiPriceDecimalFormat => (&[7], 8, 7),
            Self::AsciiTotalDecimalFormat => (&[9], 10, 9),
            _ => return Ok(None),
        };
        let size = decimal_format_size(text, arguments, max_width)?;
        if size > limit {
            return invalid(format!(
                "ASCII decimal format exceeds its {limit}-byte output bound"
            ));
        }
        Ok(Some(size))
    }

    fn decode(self, bytes: &[u8], file_offset: usize) -> Result<String> {
        if self == Self::ShiftJisTitleFormat {
            return decode_title(bytes, file_offset);
        }
        if self == Self::ShiftJisDigit {
            if bytes.len() != 3
                || bytes[0] != 0x82
                || !(0x4f..=0x58).contains(&bytes[1])
                || bytes[2] != 0
            {
                return invalid(format!("invalid Shift-JIS digit at {file_offset:#x}"));
            }
            return Ok(char::from_u32('０' as u32 + u32::from(bytes[1] - 0x4f))
                .unwrap()
                .to_string());
        }
        if self == Self::AsciiCreditsLiteral {
            let text =
                std::str::from_utf8(bytes).map_err(|error| Error::Invalid(error.to_string()))?;
            self.encode(text, bytes.len())?;
            return Ok(text.into());
        }
        if matches!(self, Self::GameKeyboardCharacter | Self::GameExactLiteral) {
            let text = decode_name(bytes, file_offset)?;
            self.encode(&text, bytes.len())?;
            return Ok(text);
        }
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
            Error::Invalid(format!("unterminated ASCII text at {file_offset:#x}"))
        })?;
        if bytes[end..].iter().any(|byte| *byte != 0) {
            return invalid("nonzero bytes after ASCII terminator");
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
            Self::GameExactLiteral => "game-exact-literal",
            Self::GameKeyboardCharacter => "game-keyboard-character",
            Self::GameCountedLiteral => "game-counted-literal",
            Self::GameGlyphs => "game-glyphs",
            Self::ShiftJisDigit => "shift-jis-digit",
            Self::ShiftJisTitleFormat => "shift-jis-title-format",
            Self::AsciiLiteral => "ascii-literal",
            Self::AsciiCreditsLiteral => "ascii-credits-literal",
            Self::AsciiFormatLiteral => "ascii-format-literal",
            Self::AsciiDigitGlyphLiteral => "ascii-digit-glyph-literal",
            Self::AsciiDashGlyphLiteral => "ascii-dash-glyph-literal",
            Self::AsciiDecimalFormat => "ascii-decimal-format",
            Self::AsciiI32DecimalFormat => "ascii-i32-decimal-format",
            Self::AsciiI32WideDecimalFormat => "ascii-i32-wide-decimal-format",
            Self::AsciiI16PlusOneDecimalFormat => "ascii-i16-plus-one-decimal-format",
            Self::AsciiI8DecimalFormat => "ascii-i8-decimal-format",
            Self::AsciiSignedDecimalPairFormat => "ascii-signed-decimal-pair-format",
            Self::AsciiI8PlusOneU8DecimalPairFormat => "ascii-i8-plus-one-u8-decimal-pair-format",
            Self::AsciiU8DecimalFormat => "ascii-u8-decimal-format",
            Self::AsciiU8PlusOneDecimalFormat => "ascii-u8-plus-one-decimal-format",
            Self::AsciiU8DecimalFieldFormat => "ascii-u8-decimal-field-format",
            Self::AsciiU8DecimalPairFormat => "ascii-u8-decimal-pair-format",
            Self::AsciiU8DecimalTripleFormat => "ascii-u8-decimal-triple-format",
            Self::AsciiU7DecimalFormat => "ascii-u7-decimal-format",
            Self::AsciiU16DecimalFormat => "ascii-u16-decimal-format",
            Self::AsciiU16DecimalFieldFormat => "ascii-u16-decimal-field-format",
            Self::AsciiPriceDecimalFormat => "ascii-price-decimal-format",
            Self::AsciiTotalDecimalFormat => "ascii-total-decimal-format",
        }
    }

    pub(crate) const fn terminator_bytes(self) -> usize {
        match self {
            Self::GameFixedLiteral
            | Self::AsciiCreditsLiteral
            | Self::GameExactLiteral
            | Self::GameCountedLiteral
            | Self::GameKeyboardCharacter => 0,
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

fn keyboard_character(byte: u8) -> bool {
    byte.is_ascii_alphanumeric() || matches!(byte, 0x3c..=0x3e | 0x5c | 0x5d | 0xff)
}

fn encode_title(text: &str, capacity: usize) -> Result<Vec<u8>> {
    let mut characters = text.chars();
    let mut bytes = Vec::new();
    let mut arguments = 0;
    while let Some(character) = characters.next() {
        let pair = match character {
            '%' if characters.next() == Some('s') => {
                arguments += 1;
                *b"%s"
            }
            '　' => [0x81, 0x40],
            '（' => [0x81, 0x69],
            '）' => [0x81, 0x6a],
            '．' => [0x81, 0x44],
            '０'..='９' => [0x82, 0x4f + (character as u32 - '０' as u32) as u8],
            'Ａ'..='Ｚ' => [0x82, 0x60 + (character as u32 - 'Ａ' as u32) as u8],
            'ａ'..='ｚ' => [0x82, 0x81 + (character as u32 - 'ａ' as u32) as u8],
            _ => {
                return invalid(
                    "Shift-JIS title requires full-width Latin letters, digits, space, parentheses or full stop, and plain %s conversions",
                );
            }
        };
        bytes.extend_from_slice(&pair);
        if bytes.len() > capacity {
            return invalid("Shift-JIS title format exceeds its allocation");
        }
    }
    if arguments != 7 || bytes.len() != 64 {
        return invalid(
            "Shift-JIS title must retain seven %s conversions and exactly 64 encoded bytes; pad with full-width spaces",
        );
    }
    Ok(bytes)
}

fn decode_title(bytes: &[u8], file_offset: usize) -> Result<String> {
    if bytes.len() != 65 || bytes[64] != 0 {
        return invalid(format!("invalid Shift-JIS title at {file_offset:#x}"));
    }
    let mut text = String::new();
    for pair in bytes[..64].chunks_exact(2) {
        let scalar = match (pair[0], pair[1]) {
            (b'%', b's') => {
                text.push_str("%s");
                continue;
            }
            (0x81, 0x40) => '　' as u32,
            (0x81, 0x69) => '（' as u32,
            (0x81, 0x6a) => '）' as u32,
            (0x81, 0x44) => '．' as u32,
            (0x82, trail @ 0x4f..=0x58) => '０' as u32 + u32::from(trail - 0x4f),
            (0x82, trail @ 0x60..=0x79) => 'Ａ' as u32 + u32::from(trail - 0x60),
            (0x82, trail @ 0x81..=0x9a) => 'ａ' as u32 + u32::from(trail - 0x81),
            _ => {
                return invalid(format!(
                    "unsupported Shift-JIS title bytes at {file_offset:#x}"
                ));
            }
        };
        text.push(char::from_u32(scalar).unwrap());
    }
    encode_title(&text, 64)?;
    Ok(text)
}

fn decimal_format_size(text: &str, arguments: &[usize], max_width: usize) -> Result<usize> {
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
            Some(byte @ b'1'..=b'9') if usize::from(byte - b'0') <= max_width => {
                usize::from(bytes.next().unwrap() - b'0')
            }
            _ if !padded => 0,
            _ => {
                return invalid(format!(
                    "zero padding requires a decimal width from 1 to {max_width}"
                ));
            }
        };
        if bytes.next() != Some(b'd') || argument >= arguments.len() {
            return invalid(format!(
                "decimal format requires its ordered %d arguments and optional width 1..{max_width}",
            ));
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
    references: &'static [u32],
    renderers: &'static [u32],
    codec: Codec,
}

impl Allocation {
    const fn shift_jis_digit(address: u32, references: &'static [u32]) -> Self {
        Self {
            address,
            capacity: 3,
            references,
            renderers: &[],
            codec: Codec::ShiftJisDigit,
        }
    }

    const fn keyboard_character(address: u32) -> Self {
        Self {
            address,
            capacity: 1,
            references: &[],
            renderers: &[0x8014f800],
            codec: Codec::GameKeyboardCharacter,
        }
    }

    const fn ascii_literal(address: u32, capacity: usize) -> Self {
        Self {
            address,
            capacity,
            references: &[],
            renderers: &[0x80150098],
            codec: Codec::AsciiLiteral,
        }
    }

    const fn credits_literal(address: u32, capacity: usize, references: &'static [u32]) -> Self {
        Self {
            address,
            capacity,
            references,
            renderers: &[0x801f91f0],
            codec: Codec::AsciiCreditsLiteral,
        }
    }

    const fn fixed_literal(address: u32, capacity: usize, renderers: &'static [u32]) -> Self {
        Self {
            address,
            capacity,
            references: &[],
            renderers,
            codec: Codec::GameFixedLiteral,
        }
    }

    const fn exact_literal(
        address: u32,
        capacity: usize,
        references: &'static [u32],
        renderers: &'static [u32],
    ) -> Self {
        Self {
            address,
            capacity,
            references,
            renderers,
            codec: Codec::GameExactLiteral,
        }
    }

    const fn counted(address: u32) -> Self {
        Self {
            address,
            capacity: 8,
            references: &[],
            renderers: &[0x8014fc90],
            codec: Codec::GameCountedLiteral,
        }
    }

    const fn literal(
        address: u32,
        capacity: usize,
        references: &'static [u32],
        renderers: &'static [u32],
    ) -> Self {
        Self {
            address,
            capacity,
            references,
            renderers,
            codec: Codec::GameLiteral,
        }
    }

    const fn decimal(address: u32, capacity: usize) -> Self {
        Self {
            address,
            capacity,
            references: &[],
            renderers: &[0x80150098],
            codec: Codec::AsciiDecimalFormat,
        }
    }

    const fn bounded_format(
        address: u32,
        capacity: usize,
        codec: Codec,
        renderers: &'static [u32],
    ) -> Self {
        Self {
            address,
            capacity,
            references: &[],
            renderers,
            codec,
        }
    }

    const fn glyphs(address: u32, capacity: usize) -> Self {
        Self {
            address,
            capacity,
            references: &[],
            renderers: &[0x8014f800],
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
        name: "US GAME category labels",
        load: crate::game::LOAD,
        size: crate::game::PAYLOAD_SIZE,
        sha256: crate::game::PAYLOAD_SHA256,
        consumers: &[0x80199d00],
        allocations: &[
            Allocation::literal(0x801c7c34, 5, &[0x801c7c5c], &[0x8014f800]),
            Allocation::literal(0x801c7c3c, 7, &[0x801c7c60], &[0x8014f800]),
            Allocation::literal(0x801c7c44, 6, &[0x801c7c64], &[0x8014f800]),
            Allocation::literal(0x801c7c4c, 7, &[0x801c7c68], &[0x8014f800]),
            Allocation::literal(0x801c7c54, 6, &[0x801c7c6c], &[0x8014f800]),
        ],
    },
    Layout {
        name: "US COMMU03 audio menu labels",
        load: 0x801d0c00,
        size: 3404,
        sha256: "e72a4ab5d560e64cfaeebc0964eb8ee67e196a5a6106d135af8bc5732e5dd280",
        consumers: &[
            0x801d1430, 0x801d1468, 0x801d1490, 0x801d14b8, 0x801d14d8, 0x801d1500,
        ],
        allocations: &[
            Allocation {
                address: 0x801d0c1c,
                capacity: 15,
                references: &[0x801d190c],
                renderers: &[0x801d1714],
                codec: Codec::AsciiLiteral,
            },
            Allocation {
                address: 0x801d0c2c,
                capacity: 5,
                references: &[0x801d1908],
                renderers: &[0x801d1714],
                codec: Codec::AsciiLiteral,
            },
            Allocation {
                address: 0x801d0c34,
                capacity: 7,
                references: &[0x801d1904],
                renderers: &[0x801d1714],
                codec: Codec::AsciiLiteral,
            },
            Allocation::literal(0x801d18f4, 4, &[], &[0x8014fc90]),
            Allocation::literal(0x801d18f8, 2, &[], &[0x8014fc90]),
            Allocation::literal(0x801d18fc, 2, &[], &[0x8014fc90]),
        ],
    },
    Layout {
        name: "US COMMU02 menu labels",
        load: 0x801d0c00,
        size: 42208,
        sha256: "32946ade160f8d4b59a27eb862ee6d0f2457cd26a89c11c8d4ebd18268088c9f",
        consumers: &[
            0x801d2cd0, 0x801d320c, 0x801d3334, 0x801d9868, 0x801d98cc, 0x801d322c, 0x801d33a8,
            0x801d33d4, 0x801d3418, 0x801d3444, 0x801d3488, 0x801d34b4, 0x801d34e8, 0x801d4bbc,
            0x801d4bf0, 0x801d4dcc, 0x801d4e00, 0x801d4e54, 0x801d96cc, 0x801d4c44, 0x801d4d24,
            0x801d3f80, 0x801d3f90, 0x801d3fc8, 0x801d4c84, 0x801d4cb4, 0x801d4d64, 0x801d4d94,
            0x801da57c,
        ],
        allocations: &[
            Allocation::fixed_literal(0x801dac54, 4, &[0x8014f800]),
            Allocation::fixed_literal(0x801dac58, 3, &[0x8014f800]),
            Allocation::fixed_literal(0x801dac5c, 3, &[0x8014f800]),
            Allocation::fixed_literal(0x801daca8, 4, &[0x8014f800]),
            Allocation::fixed_literal(0x801dacb0, 3, &[0x8014f800]),
            Allocation::literal(0x801db010, 3, &[0x801db038], &[0x8014fc90]),
            Allocation::literal(0x801db014, 4, &[0x801db03c], &[0x8014fc90]),
            Allocation::literal(0x801db018, 5, &[0x801db040], &[0x8014fc90]),
            Allocation::literal(0x801db020, 5, &[0x801db044], &[0x8014fc90]),
            Allocation::literal(0x801db028, 4, &[0x801db048], &[0x8014fc90]),
            Allocation::literal(0x801db034, 2, &[0x801db054], &[0x8014fc90]),
            Allocation::bounded_format(0x801d0c04, 4, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(0x801d0c08, 5, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(0x801d0c10, 5, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(0x801d0c18, 5, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(0x801dad60, 4, Codec::AsciiFormatLiteral, &[0x8014ff0c]),
            Allocation::bounded_format(0x801dad64, 2, Codec::AsciiFormatLiteral, &[0x8014ff0c]),
            Allocation::ascii_literal(0x801d0c2c, 2),
            Allocation::literal(0x801d0c3c, 5, &[], &[0x8014f800]),
            Allocation::literal(0x801d0c41, 5, &[], &[0x8014f800]),
            Allocation::literal(0x801d0c46, 5, &[], &[0x8014f800]),
            Allocation::literal(0x801d0c4b, 4, &[], &[0x8014f800]),
            Allocation::bounded_format(
                0x801d0c24,
                7,
                Codec::AsciiU8DecimalTripleFormat,
                &[0x8014ff0c],
            ),
            Allocation::bounded_format(0x801d0c20, 3, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::keyboard_character(0x801dab6c),
            Allocation::keyboard_character(0x801dab6d),
            Allocation::keyboard_character(0x801dab6e),
            Allocation::keyboard_character(0x801dab6f),
            Allocation::keyboard_character(0x801dab70),
            Allocation::keyboard_character(0x801dab71),
            Allocation::keyboard_character(0x801dab72),
            Allocation::keyboard_character(0x801dab73),
            Allocation::keyboard_character(0x801dab74),
            Allocation::keyboard_character(0x801dab75),
            Allocation::keyboard_character(0x801dab76),
            Allocation::keyboard_character(0x801dab77),
            Allocation::keyboard_character(0x801dab78),
            Allocation::keyboard_character(0x801dab79),
            Allocation::keyboard_character(0x801dab7a),
            Allocation::keyboard_character(0x801dab7b),
            Allocation::keyboard_character(0x801dab7c),
            Allocation::keyboard_character(0x801dab7d),
            Allocation::keyboard_character(0x801dab7e),
            Allocation::keyboard_character(0x801dab7f),
            Allocation::keyboard_character(0x801dab80),
            Allocation::keyboard_character(0x801dab81),
            Allocation::keyboard_character(0x801dab82),
            Allocation::keyboard_character(0x801dab83),
            Allocation::keyboard_character(0x801dab84),
            Allocation::keyboard_character(0x801dab85),
            Allocation::keyboard_character(0x801dab86),
            Allocation::keyboard_character(0x801dab87),
            Allocation::keyboard_character(0x801dab88),
            Allocation::keyboard_character(0x801dab89),
            Allocation::keyboard_character(0x801dab8e),
            Allocation::keyboard_character(0x801dab8f),
            Allocation::keyboard_character(0x801dab90),
            Allocation::keyboard_character(0x801dab91),
            Allocation::keyboard_character(0x801dab92),
            Allocation::keyboard_character(0x801dab93),
            Allocation::keyboard_character(0x801dab94),
            Allocation::keyboard_character(0x801dab95),
            Allocation::keyboard_character(0x801dab96),
            Allocation::keyboard_character(0x801dab97),
            Allocation::keyboard_character(0x801dab98),
            Allocation::keyboard_character(0x801dab99),
            Allocation::keyboard_character(0x801dab9c),
            Allocation::keyboard_character(0x801dab9d),
            Allocation::keyboard_character(0x801dab9e),
            Allocation::keyboard_character(0x801dab9f),
            Allocation::keyboard_character(0x801daba0),
            Allocation::keyboard_character(0x801daba1),
            Allocation::keyboard_character(0x801daba2),
            Allocation::keyboard_character(0x801daba3),
            Allocation::keyboard_character(0x801daba4),
            Allocation::keyboard_character(0x801daba5),
            Allocation::keyboard_character(0x801daba8),
            Allocation::keyboard_character(0x801daba9),
            Allocation::keyboard_character(0x801dabaa),
            Allocation::keyboard_character(0x801dabab),
            Allocation::keyboard_character(0x801dabac),
            Allocation::keyboard_character(0x801dabad),
            Allocation::keyboard_character(0x801dabb4),
            Allocation::keyboard_character(0x801dabb5),
            Allocation::keyboard_character(0x801dabb6),
            Allocation::keyboard_character(0x801dabb7),
            Allocation::keyboard_character(0x801dabb8),
            Allocation::keyboard_character(0x801dabb9),
            Allocation::keyboard_character(0x801dabba),
            Allocation::keyboard_character(0x801dabbb),
            Allocation::keyboard_character(0x801dabbc),
            Allocation::keyboard_character(0x801dabbd),
        ],
    },
    Layout {
        name: "US COMMU00 names and numeric format",
        load: 0x801eec00,
        size: 15696,
        sha256: "c32b898b62336437f97fb135fa3197d1ce1405ec6cc43228d4bd6217b832902e",
        consumers: &[0x801d6a1c, 0x801d693c, 0x801d83d0, 0x801f1318, 0x801f13b8],
        allocations: &[
            Allocation::fixed_literal(0x801f2700, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2709, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2712, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f271b, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2724, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f272d, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2736, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f273f, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2748, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2751, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f275a, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2763, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f276c, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2775, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f277e, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2787, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2790, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2799, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27a2, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27ab, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27b4, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27bd, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27c6, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27cf, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27d8, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27e1, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27ea, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27f3, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f27fc, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2805, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f280e, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2817, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2820, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2829, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2832, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f283b, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2844, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f284d, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2856, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f285f, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2868, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2871, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f287a, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2883, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f288c, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2895, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f289e, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28a7, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28b0, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28b9, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28c2, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28cb, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28d4, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28dd, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28e6, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28ef, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f28f8, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2901, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f290a, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801f2913, 5, &[0x8014f800]),
            Allocation::bounded_format(0x801eec44, 3, Codec::AsciiI32DecimalFormat, &[]),
        ],
    },
    Layout {
        name: "US COMMU01 list and footer text",
        load: 0x801d0c00,
        size: 16948,
        sha256: "78ff172036bc1f7be63f0750c280da5a921f78b68175d329fe562053aeac7710",
        consumers: &[0x801d3038, 0x801d4178, 0x801d44e0],
        allocations: &[
            Allocation::bounded_format(0x801d0c10, 4, Codec::AsciiI8DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(0x801d0c14, 3, Codec::AsciiU8DecimalFormat, &[0x8014ff0c]),
            Allocation::literal(0x801d4c30, 8, &[], &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d34, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d3c, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d44, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d4c, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d54, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d5c, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d64, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d6c, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d74, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d7c, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d84, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d8c, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d94, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4d9c, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4da4, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4dac, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4db4, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4dbc, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4dc4, 8, &[0x8014f800]),
            Allocation::fixed_literal(0x801d4dcc, 8, &[0x8014f800]),
        ],
    },
    Layout {
        name: "US COMMU05 faery labels and counts",
        load: 0x801d0c00,
        size: 3576,
        sha256: "6198548df37a48b467055356db0754306c2a9fe1bfc0ebbd9cf49e03696a2ec4",
        consumers: &[0x801d0d4c, 0x801d1048, 0x801d129c],
        allocations: &[
            Allocation::bounded_format(
                0x801d0c04,
                6,
                Codec::AsciiI8PlusOneU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0c0c, 4, Codec::AsciiU8DecimalFormat, &[0x8014ff0c]),
            Allocation::literal(0x801d195c, 8, &[], &[0x8014f800]),
            Allocation::literal(0x801d1964, 6, &[], &[0x8014f800]),
            Allocation::literal(0x801d196c, 8, &[], &[0x8014f800]),
            Allocation::literal(0x801d1974, 6, &[], &[0x8014f800]),
        ],
    },
    Layout {
        name: "US SCENA17 credits text",
        load: 0x801f6c00,
        size: 12764,
        sha256: "38459e1bee02f415e6bcf37b17ffe359b6dac2e492d6972b3dac12379beb6cc0",
        consumers: &[0x801f8110, 0x801f8170, 0x801f938c],
        allocations: &[
            Allocation::credits_literal(0x801f6c04, 16, &[0x801f9d94]),
            Allocation::credits_literal(0x801f6c18, 10, &[0x801f9d64]),
            Allocation::credits_literal(0x801f6c24, 10, &[0x801f9d60]),
            Allocation::credits_literal(0x801f6c32, 8, &[]),
            Allocation::credits_literal(0x801f6c3c, 19, &[0x801f9d3c]),
            Allocation::credits_literal(0x801f6c52, 18, &[]),
            Allocation::credits_literal(0x801f6c68, 21, &[0x801f9d28]),
            Allocation::credits_literal(0x801f6c82, 16, &[]),
            Allocation::credits_literal(0x801f6c94, 20, &[0x801f9d14]),
            Allocation::credits_literal(0x801f6cae, 21, &[]),
            Allocation::credits_literal(0x801f6cc4, 11, &[0x801f9cfc]),
            Allocation::credits_literal(0x801f6cd0, 24, &[0x801f9cf8]),
            Allocation::credits_literal(0x801f6cec, 5, &[0x801f9cf0]),
            Allocation::credits_literal(0x801f6cf3, 8, &[]),
            Allocation::credits_literal(0x801f6cfc, 5, &[0x801f9cec]),
            Allocation::credits_literal(0x801f6d03, 8, &[]),
            Allocation::credits_literal(0x801f6d0c, 5, &[0x801f9ce8]),
            Allocation::credits_literal(0x801f6d13, 6, &[]),
            Allocation::credits_literal(0x801f6d1c, 5, &[0x801f9ce4]),
            Allocation::credits_literal(0x801f6d23, 6, &[]),
            Allocation::credits_literal(0x801f6d2c, 5, &[0x801f9ce0]),
            Allocation::credits_literal(0x801f6d33, 4, &[]),
            Allocation::credits_literal(0x801f6d38, 10, &[0x801f9cdc]),
            Allocation::credits_literal(0x801f6d44, 19, &[0x801f9cd0]),
            Allocation::credits_literal(0x801f6d58, 22, &[0x801f9ccc]),
            Allocation::credits_literal(0x801f6d70, 18, &[0x801f9cc8]),
            Allocation::credits_literal(0x801f6d84, 7, &[0x801f9cc0]),
            Allocation::credits_literal(0x801f6d8c, 11, &[0x801f9cb4]),
            Allocation::credits_literal(0x801f6d9b, 9, &[]),
            Allocation::credits_literal(0x801f6da8, 19, &[0x801f9cb0]),
            Allocation::credits_literal(0x801f6dbc, 17, &[0x801f9cac]),
            Allocation::credits_literal(0x801f6dd0, 11, &[0x801f9ca8]),
            Allocation::credits_literal(0x801f6ddc, 12, &[0x801f9ca0]),
            Allocation::credits_literal(0x801f6dec, 15, &[0x801f9c9c]),
            Allocation::credits_literal(0x801f6dfc, 11, &[0x801f9c98]),
            Allocation::credits_literal(0x801f6e08, 20, &[0x801f9c94]),
            Allocation::credits_literal(0x801f6e20, 17, &[0x801f9c90]),
            Allocation::credits_literal(0x801f6e34, 9, &[0x801f9c8c]),
            Allocation::credits_literal(0x801f6e40, 21, &[0x801f9c84]),
            Allocation::credits_literal(0x801f6e58, 17, &[0x801f9c80]),
            Allocation::credits_literal(0x801f6e6c, 18, &[0x801f9c7c]),
            Allocation::credits_literal(0x801f6e80, 5, &[0x801f9c78]),
            Allocation::credits_literal(0x801f6e88, 8, &[0x801f9c70]),
            Allocation::credits_literal(0x801f6e92, 3, &[]),
            Allocation::credits_literal(0x801f6e98, 18, &[0x801f9c6c]),
            Allocation::credits_literal(0x801f6eac, 17, &[0x801f9c68]),
            Allocation::credits_literal(0x801f6ec0, 9, &[0x801f9c64]),
            Allocation::credits_literal(0x801f6ecc, 21, &[0x801f9c58]),
            Allocation::credits_literal(0x801f6ee4, 17, &[0x801f9c54, 0x801f9cc4]),
            Allocation::credits_literal(0x801f6ef8, 19, &[0x801f9c50]),
            Allocation::credits_literal(0x801f6f0c, 17, &[0x801f9c4c]),
            Allocation::credits_literal(0x801f6f20, 17, &[0x801f9c48]),
            Allocation::credits_literal(0x801f6f34, 17, &[0x801f9c44]),
            Allocation::credits_literal(0x801f6f48, 5, &[0x801f9c38]),
            Allocation::credits_literal(0x801f6f51, 14, &[]),
            Allocation::credits_literal(0x801f6f64, 13, &[0x801f9c1c]),
            Allocation::credits_literal(0x801f6f74, 18, &[0x801f9c18]),
            Allocation::credits_literal(0x801f6f88, 16, &[0x801f9c14]),
            Allocation::credits_literal(0x801f6f9c, 18, &[0x801f9c10]),
            Allocation::credits_literal(0x801f6fb0, 14, &[0x801f9c0c]),
            Allocation::credits_literal(0x801f6fc0, 17, &[0x801f9c08]),
            Allocation::credits_literal(0x801f6fd4, 23, &[0x801f9c00]),
            Allocation::credits_literal(0x801f6fec, 4, &[0x801f9bfc]),
            Allocation::credits_literal(0x801f6ff4, 18, &[]),
            Allocation::credits_literal(0x801f700c, 21, &[0x801f9bdc]),
            Allocation::credits_literal(0x801f7024, 15, &[0x801f9bd8]),
            Allocation::credits_literal(0x801f7034, 15, &[0x801f9bd4]),
            Allocation::credits_literal(0x801f7044, 16, &[0x801f9bd0]),
            Allocation::credits_literal(0x801f7058, 15, &[0x801f9bcc]),
            Allocation::credits_literal(0x801f7068, 16, &[0x801f9bc8]),
            Allocation::credits_literal(0x801f707c, 20, &[0x801f9bc4]),
            Allocation::credits_literal(0x801f7094, 16, &[0x801f9bc0]),
            Allocation::credits_literal(0x801f70a8, 5, &[0x801f9bb4]),
            Allocation::credits_literal(0x801f70b1, 12, &[]),
            Allocation::credits_literal(0x801f70c0, 21, &[0x801f9b9c]),
            Allocation::credits_literal(0x801f70d8, 17, &[0x801f9b98]),
            Allocation::credits_literal(0x801f70ec, 2, &[0x801f9b88]),
            Allocation::credits_literal(0x801f70f0, 1, &[]),
            Allocation::credits_literal(0x801f70f3, 18, &[]),
            Allocation::credits_literal(0x801f7108, 16, &[0x801f9b7c]),
            Allocation::credits_literal(0x801f711a, 6, &[]),
            Allocation::credits_literal(0x801f7124, 7, &[0x801f9b70]),
            Allocation::credits_literal(0x801f712f, 10, &[]),
            Allocation::credits_literal(0x801f713c, 5, &[0x801f9b6c]),
            Allocation::credits_literal(0x801f7145, 12, &[]),
            Allocation::credits_literal(0x801f7154, 21, &[0x801f9b5c]),
            Allocation::credits_literal(0x801f716c, 12, &[0x801f9b58]),
            Allocation::credits_literal(0x801f717c, 16, &[0x801f9b4c, 0x801f9b8c]),
            Allocation::credits_literal(0x801f7190, 15, &[0x801f9b48, 0x801f9b80, 0x801f9b90]),
            Allocation::credits_literal(0x801f71a0, 17, &[0x801f9b44]),
            Allocation::credits_literal(0x801f71b4, 16, &[0x801f9b30]),
            Allocation::credits_literal(0x801f71c8, 19, &[0x801f9b2c]),
            Allocation::credits_literal(0x801f71dc, 8, &[0x801f9b20]),
            Allocation::credits_literal(0x801f71e8, 10, &[0x801f9b1c]),
            Allocation::credits_literal(0x801f71f4, 16, &[0x801f9b14]),
            Allocation::credits_literal(0x801f7208, 10, &[0x801f9b10]),
            Allocation::credits_literal(0x801f7214, 8, &[]),
            Allocation::credits_literal(0x801f7220, 16, &[0x801f9af8]),
            Allocation::credits_literal(0x801f7234, 20, &[0x801f9aec]),
            Allocation::credits_literal(0x801f724c, 17, &[0x801f9ae8, 0x801f9afc]),
            Allocation::credits_literal(0x801f7260, 18, &[0x801f9ae4]),
            Allocation::credits_literal(0x801f7274, 14, &[0x801f9ae0]),
            Allocation::credits_literal(0x801f7284, 19, &[0x801f9adc]),
            Allocation::credits_literal(0x801f7298, 16, &[0x801f9ad8]),
            Allocation::credits_literal(0x801f72ac, 18, &[0x801f9acc]),
            Allocation::credits_literal(0x801f72c0, 17, &[0x801f9ac0, 0x801f9ad4]),
            Allocation::credits_literal(0x801f72d4, 4, &[0x801f9abc]),
            Allocation::credits_literal(0x801f72da, 14, &[]),
            Allocation::credits_literal(0x801f72ec, 14, &[0x801f9aa4]),
            Allocation::credits_literal(0x801f72fc, 18, &[0x801f9aa0]),
            Allocation::credits_literal(0x801f7310, 17, &[0x801f9a9c]),
            Allocation::credits_literal(0x801f7324, 16, &[0x801f9a98]),
            Allocation::credits_literal(0x801f7338, 19, &[0x801f9a94]),
            Allocation::credits_literal(0x801f734c, 16, &[0x801f9a90]),
            Allocation::credits_literal(0x801f7360, 15, &[0x801f9a84]),
            Allocation::credits_literal(0x801f7370, 15, &[0x801f9a80]),
            Allocation::credits_literal(0x801f7380, 8, &[0x801f9a7c]),
            Allocation::credits_literal(0x801f738c, 23, &[0x801f9a78]),
            Allocation::credits_literal(0x801f73a4, 18, &[0x801f9a6c]),
            Allocation::credits_literal(0x801f73b8, 17, &[0x801f9a68]),
            Allocation::credits_literal(0x801f73cc, 16, &[0x801f9a64]),
            Allocation::credits_literal(0x801f73e0, 19, &[0x801f9a60]),
            Allocation::credits_literal(0x801f73f4, 15, &[0x801f9a54]),
            Allocation::credits_literal(0x801f7404, 4, &[0x801f9a3c]),
            Allocation::credits_literal(0x801f740a, 16, &[]),
            Allocation::credits_literal(0x801f741c, 19, &[0x801f9a2c]),
            Allocation::credits_literal(0x801f7430, 4, &[0x801f9a1c]),
            Allocation::credits_literal(0x801f7436, 11, &[]),
            Allocation::credits_literal(0x801f7444, 21, &[0x801f9a00]),
            Allocation::credits_literal(0x801f745c, 25, &[0x801f99ec]),
            Allocation::credits_literal(0x801f7478, 9, &[0x801f99dc]),
            Allocation::credits_literal(0x801f7483, 13, &[]),
            Allocation::credits_literal(0x801f7494, 19, &[0x801f99c4, 0x801f99f0, 0x801f9be0]),
            Allocation::credits_literal(0x801f74a8, 17, &[0x801f99c0]),
            Allocation::credits_literal(0x801f74bc, 25, &[0x801f99b0]),
            Allocation::credits_literal(0x801f74d8, 17, &[0x801f99a4]),
            Allocation::credits_literal(0x801f74ec, 10, &[0x801f9998]),
            Allocation::credits_literal(0x801f74f8, 20, &[0x801f9984, 0x801f99b4, 0x801f9a08]),
            Allocation::credits_literal(0x801f7510, 18, &[0x801f997c]),
            Allocation::credits_literal(0x801f7524, 13, &[0x801f9978]),
            Allocation::credits_literal(0x801f7534, 18, &[0x801f9968]),
            Allocation::credits_literal(0x801f7548, 13, &[0x801f9958]),
            Allocation::credits_literal(0x801f7558, 10, &[0x801f9940, 0x801f9a04]),
            Allocation::credits_literal(0x801f7564, 5, &[0x801f993c]),
            Allocation::credits_literal(0x801f756b, 17, &[]),
            Allocation::credits_literal(0x801f7580, 18, &[0x801f992c, 0x801f99f4]),
            Allocation::credits_literal(0x801f7594, 17, &[0x801f9928]),
            Allocation::credits_literal(0x801f75a8, 19, &[0x801f9918, 0x801f9a40]),
            Allocation::credits_literal(0x801f75bc, 18, &[0x801f9914]),
            Allocation::credits_literal(0x801f75d0, 15, &[0x801f9908, 0x801f9b18]),
            Allocation::credits_literal(0x801f75e0, 18, &[0x801f9904]),
            Allocation::credits_literal(0x801f75f4, 15, &[0x801f98f8, 0x801f999c, 0x801f9a30]),
            Allocation::credits_literal(0x801f7604, 14, &[0x801f98f4, 0x801f995c, 0x801f9a20]),
            Allocation::credits_literal(0x801f7614, 13, &[0x801f98f0]),
            Allocation::credits_literal(0x801f7624, 21, &[0x801f98d8, 0x801f9a5c]),
            Allocation::credits_literal(0x801f763e, 16, &[]),
            Allocation::credits_literal(0x801f7650, 15, &[0x801f98c8, 0x801f9ad0]),
            Allocation::credits_literal(0x801f7662, 16, &[]),
            Allocation::credits_literal(0x801f7674, 18, &[0x801f98b8, 0x801f9a58]),
            Allocation::credits_literal(0x801f768a, 13, &[]),
            Allocation::credits_literal(
                0x801f7698,
                23,
                &[0x801f98a8, 0x801f991c, 0x801f996c, 0x801f9980],
            ),
            Allocation::credits_literal(0x801f76b2, 15, &[]),
            Allocation::credits_literal(0x801f76c4, 18, &[0x801f9898, 0x801f99a0, 0x801f99e0]),
            Allocation::credits_literal(0x801f76da, 8, &[]),
            Allocation::credits_literal(0x801f76e4, 9, &[0x801f9888]),
            Allocation::credits_literal(0x801f76f1, 5, &[]),
            Allocation::credits_literal(0x801f76fc, 20, &[0x801f985c]),
        ],
    },
    Layout {
        name: "US SCENA10 numeric text",
        load: 0x801f6c00,
        size: 32224,
        sha256: "e53877204d30b2225a709a5683fe360b65a3a50b108de6ed788c86e207877465",
        consumers: &[0x801f719c, 0x801f7ad4],
        allocations: &[
            Allocation::bounded_format(0x801f6c08, 5, Codec::AsciiU16DecimalFormat, &[0x8014f800]),
            Allocation::bounded_format(
                0x801f6c10,
                5,
                Codec::AsciiU8DecimalFieldFormat,
                &[0x80150098],
            ),
        ],
    },
    Layout {
        name: "US SCENA12 counter text",
        load: 0x801f6c00,
        size: 26250,
        sha256: "9b84ae69ac1d5e40d08c1ae382df0160a057bb5571395effb7f1502131cfa232",
        consumers: &[0x801f75ec],
        allocations: &[Allocation::bounded_format(
            0x801f6c04,
            5,
            Codec::AsciiU8DecimalFormat,
            &[0x8014ff0c],
        )],
    },
    Layout {
        name: "US BOSS025 turn-count text",
        load: 0x800c1800,
        size: 6844,
        sha256: "a8ce67f13802cef17556859ddfd2bb9f48fa3f42f097662b3765ab00ad4195d2",
        consumers: &[0x800c3088],
        allocations: &[
            Allocation::bounded_format(0x800c184c, 4, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::literal(0x800c32a8, 11, &[], &[0x8014f800]),
        ],
    },
    Layout {
        name: "US AREA030 panel text",
        load: 0x801d0c00,
        size: 75285,
        sha256: "e14e20ddaf9e850e97a841db08af9d447e4b57bd881bbe7916452f8fb87d33cd",
        consumers: &[
            0x801d159c, 0x801d36f8, 0x801d3824, 0x801d3898, 0x801d39b8, 0x801d3a54, 0x801d3c74,
            0x801d3f40, 0x801d5d88, 0x801d6f34, 0x801d72e4, 0x801d7b64, 0x801d7c10, 0x801d7eb0,
            0x801d8364, 0x801d88e0, 0x801e1004, 0x801e1320,
        ],
        allocations: &[
            Allocation::bounded_format(
                0x801d0c04,
                4,
                Codec::AsciiI16PlusOneDecimalFormat,
                &[0x8014f800, 0x8014ff0c],
            ),
            Allocation::bounded_format(
                0x801d0c08,
                5,
                Codec::AsciiU16DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0c10, 3, Codec::AsciiDigitGlyphLiteral, &[0x8014ff0c]),
            Allocation::bounded_format(
                0x801d0c14,
                5,
                Codec::AsciiU8DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0c38, 4, Codec::AsciiU16DecimalFormat, &[0x8014f800]),
            Allocation::bounded_format(0x801d0c8c, 4, Codec::AsciiU16DecimalFormat, &[0x8014f800]),
            Allocation::fixed_literal(0x801e28c4, 12, &[0x8014f800]),
            Allocation::fixed_literal(0x801e28d0, 12, &[0x8014f800]),
            Allocation::fixed_literal(0x801e21c0, 4, &[0x8014f800]),
            Allocation::fixed_literal(0x801e21c4, 4, &[0x8014f800]),
            Allocation::fixed_literal(0x801e21c8, 4, &[0x8014f800]),
            Allocation::fixed_literal(0x801e21cc, 3, &[0x8014f800]),
            Allocation::fixed_literal(0x801e21d0, 4, &[0x8014f800]),
            Allocation::fixed_literal(0x801e21d4, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801e21d9, 5, &[0x8014f800]),
            Allocation::exact_literal(0x801e1dac, 10, &[0x801e1e7c], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(
                0x801e1db8,
                17,
                &[0x801e1e88, 0x801e1f0c],
                &[0x8014f800, 0x801d9740],
            ),
            Allocation::exact_literal(0x801e1dcc, 15, &[0x801e1e94], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1ddc, 12, &[0x801e1ea0], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1de8, 29, &[0x801e1eac], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1e08, 13, &[0x801e1eb8], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1e18, 2, &[0x801e1ec4], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1e1c, 2, &[0x801e1ed0], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1e20, 25, &[0x801e1edc], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1e3c, 22, &[0x801e1ee8], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1e54, 30, &[0x801e1ef4], &[0x8014f800, 0x801d9740]),
            Allocation::exact_literal(0x801e1e74, 1, &[0x801e1f00], &[0x8014f800, 0x801d9740]),
        ],
    },
    Layout {
        name: "US AREA008 decimal formats",
        load: 0x801f2c00,
        size: 10232,
        sha256: "ff37a43f66ba97f97f7c4c7f1a9020a614e95e48e4629fd6c294981945c97237",
        consumers: &[0x801f3c2c, 0x801f3d18, 0x801f437c, 0x801f43c8],
        allocations: &[
            Allocation::bounded_format(
                0x801f2c04,
                10,
                Codec::AsciiSignedDecimalPairFormat,
                &[0x8014ff0c],
            ),
            Allocation::bounded_format(0x801f2c10, 4, Codec::AsciiU8DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(0x801f2c14, 3, Codec::AsciiU7DecimalFormat, &[]),
        ],
    },
    Layout {
        name: "US START and STATUS labels and default names",
        load: 0x801d0c00,
        size: 117986,
        sha256: "06aeadccf2ed4299a9b315f6b9eb5c924eac7a458bbabfcde0f29afff5046908",
        consumers: &[
            0x801d15c0, 0x801dcf50, 0x801e4ad8, 0x801e6680, 0x801e692c, 0x801dbc88, 0x801dbcfc,
            0x801dbd64, 0x801dbdc0, 0x801dc90c, 0x801dc934, 0x801dc95c, 0x801dc978, 0x801de6ec,
            0x801df1ac, 0x801db880, 0x801db8a8, 0x801dbca4, 0x801dbcd0, 0x801dbd18, 0x801dbd38,
            0x801dbd80, 0x801dbda0, 0x801dbddc, 0x801dbdf4, 0x801dc804, 0x801dc838, 0x801dc84c,
            0x801dc870, 0x801dc884, 0x801dc8a8, 0x801dc8bc, 0x801dc8d4, 0x801de720, 0x801de748,
            0x801de758, 0x801de774, 0x801de784, 0x801de7a0, 0x801de7b0, 0x801de7cc, 0x801de8e0,
            0x801de904, 0x801df050, 0x801df07c, 0x801df0b8, 0x801df0e0, 0x801df0f0, 0x801df10c,
            0x801df11c, 0x801df138, 0x801dff24, 0x801e05a0, 0x801e0e2c, 0x801e2cdc, 0x801e2d18,
        ],
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
            Allocation::fixed_literal(0x801eb514, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801eb5b8, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801eb65c, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801eb700, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801eb7a4, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801eb848, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801eb8ec, 5, &[0x8014f800]),
            Allocation::fixed_literal(0x801eb990, 5, &[0x8014f800]),
            Allocation::keyboard_character(0x801ece7c),
            Allocation::keyboard_character(0x801ece7d),
            Allocation::keyboard_character(0x801ece7e),
            Allocation::keyboard_character(0x801ece7f),
            Allocation::keyboard_character(0x801ece80),
            Allocation::keyboard_character(0x801ece81),
            Allocation::keyboard_character(0x801ece82),
            Allocation::keyboard_character(0x801ece83),
            Allocation::keyboard_character(0x801ece84),
            Allocation::keyboard_character(0x801ece85),
            Allocation::keyboard_character(0x801ece86),
            Allocation::keyboard_character(0x801ece87),
            Allocation::keyboard_character(0x801ece88),
            Allocation::keyboard_character(0x801ece89),
            Allocation::keyboard_character(0x801ece8a),
            Allocation::keyboard_character(0x801ece8b),
            Allocation::keyboard_character(0x801ece8c),
            Allocation::keyboard_character(0x801ece8d),
            Allocation::keyboard_character(0x801ece8e),
            Allocation::keyboard_character(0x801ece8f),
            Allocation::keyboard_character(0x801ece90),
            Allocation::keyboard_character(0x801ece91),
            Allocation::keyboard_character(0x801ece92),
            Allocation::keyboard_character(0x801ece93),
            Allocation::keyboard_character(0x801ece94),
            Allocation::keyboard_character(0x801ece95),
            Allocation::keyboard_character(0x801ece96),
            Allocation::keyboard_character(0x801ece97),
            Allocation::keyboard_character(0x801ece98),
            Allocation::keyboard_character(0x801ece99),
            Allocation::keyboard_character(0x801ece9e),
            Allocation::keyboard_character(0x801ece9f),
            Allocation::keyboard_character(0x801ecea0),
            Allocation::keyboard_character(0x801ecea1),
            Allocation::keyboard_character(0x801ecea2),
            Allocation::keyboard_character(0x801ecea3),
            Allocation::keyboard_character(0x801ecea4),
            Allocation::keyboard_character(0x801ecea5),
            Allocation::keyboard_character(0x801ecea6),
            Allocation::keyboard_character(0x801ecea7),
            Allocation::keyboard_character(0x801ecea8),
            Allocation::keyboard_character(0x801ecea9),
            Allocation::keyboard_character(0x801eceac),
            Allocation::keyboard_character(0x801ecead),
            Allocation::keyboard_character(0x801eceae),
            Allocation::keyboard_character(0x801eceaf),
            Allocation::keyboard_character(0x801eceb0),
            Allocation::keyboard_character(0x801eceb1),
            Allocation::keyboard_character(0x801eceb2),
            Allocation::keyboard_character(0x801eceb3),
            Allocation::keyboard_character(0x801eceb4),
            Allocation::keyboard_character(0x801eceb5),
            Allocation::keyboard_character(0x801eceb8),
            Allocation::keyboard_character(0x801eceb9),
            Allocation::keyboard_character(0x801eceba),
            Allocation::keyboard_character(0x801ecebb),
            Allocation::keyboard_character(0x801ecebc),
            Allocation::keyboard_character(0x801ecebd),
            Allocation::keyboard_character(0x801ecec4),
            Allocation::keyboard_character(0x801ecec5),
            Allocation::keyboard_character(0x801ecec6),
            Allocation::keyboard_character(0x801ecec7),
            Allocation::keyboard_character(0x801ecec8),
            Allocation::keyboard_character(0x801ecec9),
            Allocation::keyboard_character(0x801ececa),
            Allocation::keyboard_character(0x801ececb),
            Allocation::keyboard_character(0x801ececc),
            Allocation::keyboard_character(0x801ececd),
            Allocation::literal(0x801ed190, 3, &[0x801ed1b8], &[0x8014fc90]),
            Allocation::literal(0x801ed194, 4, &[0x801ed1bc], &[0x8014fc90]),
            Allocation::literal(0x801ed198, 5, &[0x801ed1c0], &[0x8014fc90]),
            Allocation::literal(0x801ed1a0, 5, &[0x801ed1c4], &[0x8014fc90]),
            Allocation::literal(0x801ec940, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ec944, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ec948, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ec94c, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ec99c, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ec9a2, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ec9a8, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ec9ae, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ecb0c, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ecb12, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ecb18, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ecb1e, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ecb2c, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ecb32, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801ecb38, 4, &[], &[0x8014f800]),
            Allocation::bounded_format(0x801d0c4c, 4, Codec::AsciiU16DecimalFormat, &[0x80150098]),
            Allocation::literal(0x801ecc3c, 5, &[0x801ecc64], &[0x8014f800]),
            Allocation::literal(0x801ecc44, 7, &[0x801ecc68], &[0x8014f800]),
            Allocation::literal(0x801ecc4c, 6, &[0x801ecc6c], &[0x8014f800]),
            Allocation::literal(0x801ecc54, 7, &[0x801ecc70], &[0x8014f800]),
            Allocation::literal(0x801ecc5c, 6, &[0x801ecc74], &[0x8014f800]),
            Allocation::literal(0x801ecc78, 5, &[0x801ecc98], &[0x8014f800]),
            Allocation::literal(0x801ecc80, 7, &[0x801ecc9c], &[0x8014f800]),
            Allocation::literal(0x801ecc88, 7, &[0x801ecca0], &[0x8014f800]),
            Allocation::literal(0x801ecc90, 6, &[0x801ecca4], &[0x8014f800]),
            Allocation::literal(0x801eccb0, 7, &[0x801eccd4], &[0x8014f800]),
            Allocation::literal(0x801eccb8, 6, &[0x801eccd8], &[0x8014f800]),
            Allocation::literal(0x801eccc0, 7, &[0x801eccdc], &[0x8014f800]),
            Allocation::fixed_literal(0x801ecd26, 4, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd2e, 4, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd36, 4, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd3e, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd46, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd4e, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd56, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd5e, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd66, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd6e, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd76, 1, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd7e, 6, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd86, 4, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd8e, 6, &[0x8014f800, 0x8014fc90]),
            Allocation::fixed_literal(0x801ecd96, 4, &[0x8014f800, 0x8014fc90]),
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
            Allocation::literal(0x801d4038, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d403c, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d4040, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d4044, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d4048, 5, &[], &[0x8014fc90]),
            Allocation::literal(0x801d4050, 5, &[], &[0x8014fc90]),
        ],
    },
    Layout {
        name: "US battle categories",
        load: 0x80096800,
        size: 133316,
        sha256: "3df942ff3b7822e4582cb809cec769712592d7059f39e06fef1e4e057be02c24",
        consumers: &[0x800b28c8, 0x800b2f70],
        allocations: &[
            Allocation::literal(0x800b6e60, 5, &[0x800b6e88], &[0x8014f800]),
            Allocation::literal(0x800b6e68, 7, &[0x800b6e8c], &[0x8014f800]),
            Allocation::literal(0x800b6e70, 6, &[0x800b6e90], &[0x8014f800]),
            Allocation::literal(0x800b6e78, 7, &[0x800b6e94], &[0x8014f800]),
            Allocation::literal(0x800b6e80, 6, &[0x800b6e98], &[0x8014f800]),
            Allocation::literal(0x800b6e9c, 5, &[0x800b6ec4], &[0x8014f800]),
            Allocation::literal(0x800b6ea4, 7, &[0x800b6ec8], &[0x8014f800]),
            Allocation::literal(0x800b6eac, 7, &[0x800b6ecc], &[0x8014f800]),
            Allocation::literal(0x800b6eb4, 6, &[0x800b6ed0], &[0x8014f800]),
            Allocation::literal(0x800b6ebc, 7, &[0x800b6ed4], &[0x8014f800]),
        ],
    },
    Layout {
        name: "US BATE labels and numeric formats",
        load: 0x801d0c00,
        size: 33864,
        sha256: "04de0ab074178f161111a03fe1ea1d257919d9a6e33feafcac378d578795474b",
        consumers: &[
            0x801d28e0, 0x801d2908, 0x801d2ce8, 0x801d2d04, 0x801d2d30, 0x801d2d5c, 0x801d2d78,
            0x801d2d98, 0x801d2dc4, 0x801d2de0, 0x801d2e00, 0x801d2e20, 0x801d2e3c, 0x801d2e54,
            0x801d3864, 0x801d3898, 0x801d38ac, 0x801d38d0, 0x801d38e4, 0x801d3908, 0x801d391c,
            0x801d3934, 0x801d396c, 0x801d3994, 0x801d39bc, 0x801d39d8, 0x801d3a48, 0x801d3a6c,
            0x801d3ac4, 0x801d3ae4, 0x801d3b10, 0x801d3fb0, 0x801d48d4, 0x801d48fc, 0x801d4a4c,
            0x801d4a74, 0x801d4a88, 0x801d4aa4, 0x801d574c, 0x801d5780, 0x801d57a8, 0x801d57b8,
            0x801d57d4, 0x801d57e4, 0x801d5800, 0x801d5810, 0x801d582c, 0x801d5940, 0x801d5964,
            0x801d60b0, 0x801d60dc, 0x801d6118, 0x801d6140, 0x801d6150, 0x801d616c, 0x801d617c,
            0x801d6198, 0x801d620c, 0x801d6f84, 0x801d6fc0, 0x801d7600, 0x801d7e8c, 0x801d7eac,
            0x801d8394, 0x801d83b8, 0x801d83d0, 0x801d83f8, 0x801d841c, 0x801d8438,
        ],
        allocations: &[
            Allocation::bounded_format(0x801d0c20, 4, Codec::AsciiU16DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(
                0x801d0c44,
                4,
                Codec::AsciiI32DecimalFormat,
                &[0x8014ff0c, 0x80150098],
            ),
            Allocation::bounded_format(0x801d0c48, 8, Codec::AsciiFormatLiteral, &[0x80150098]),
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
            Allocation::bounded_format(0x801d0d1c, 5, Codec::AsciiU8DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(
                0x801d0d5c,
                8,
                Codec::AsciiU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0d80, 4, Codec::AsciiU8DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(
                0x801d0d84,
                10,
                Codec::AsciiU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::ascii_literal(0x801d0d90, 4),
            Allocation::literal(0x801d8b28, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8b2c, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8b30, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8b34, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8b84, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8b8a, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8b90, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8b96, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8cf4, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8cfa, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8d00, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8d06, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8d14, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8d1a, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8d20, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d8e24, 5, &[0x801d8e4c], &[0x8014f800]),
            Allocation::literal(0x801d8e2c, 7, &[0x801d8e50], &[0x8014f800]),
            Allocation::literal(0x801d8e34, 6, &[0x801d8e54], &[0x8014f800]),
            Allocation::literal(0x801d8e3c, 7, &[0x801d8e58], &[0x8014f800]),
            Allocation::literal(0x801d8e44, 6, &[0x801d8e5c], &[0x8014f800]),
            Allocation::literal(0x801d8e60, 5, &[0x801d8e80], &[0x8014f800]),
            Allocation::literal(0x801d8e68, 7, &[0x801d8e84], &[0x8014f800]),
            Allocation::literal(0x801d8e70, 7, &[0x801d8e88], &[0x8014f800]),
            Allocation::literal(0x801d8e78, 6, &[0x801d8e8c], &[0x8014f800]),
            Allocation::literal(0x801d8e90, 5, &[0x801d8eb8], &[0x8014f800]),
            Allocation::literal(0x801d8e98, 7, &[0x801d8ebc], &[0x8014f800]),
            Allocation::literal(0x801d8ea0, 6, &[0x801d8ec0], &[0x8014f800]),
            Allocation::literal(0x801d8ea8, 7, &[0x801d8ec4], &[0x8014f800]),
            Allocation::literal(0x801d8eb0, 6, &[0x801d8ec8], &[0x8014f800]),
        ],
    },
    Layout {
        name: "US SHISU labels and numeric formats",
        load: 0x801d0c00,
        size: 38196,
        sha256: "98d777a6fc806f6ae1b341c37e6f4c807d738d395a66ca6d44a012a567f5d7b0",
        consumers: &[
            0x801d3888, 0x801d38b0, 0x801d3c90, 0x801d3cac, 0x801d3cd8, 0x801d3d04, 0x801d3d20,
            0x801d3d40, 0x801d3d6c, 0x801d3d88, 0x801d3da8, 0x801d3dc8, 0x801d3de4, 0x801d3dfc,
            0x801d480c, 0x801d4840, 0x801d4854, 0x801d4878, 0x801d488c, 0x801d48b0, 0x801d48c4,
            0x801d48dc, 0x801d4914, 0x801d493c, 0x801d4964, 0x801d4980, 0x801d49f0, 0x801d4a14,
            0x801d4a6c, 0x801d4a8c, 0x801d4ab8, 0x801d4f58, 0x801d587c, 0x801d58a4, 0x801d59f4,
            0x801d5a1c, 0x801d5a30, 0x801d5a4c, 0x801d66f4, 0x801d6728, 0x801d6750, 0x801d6760,
            0x801d677c, 0x801d678c, 0x801d67a8, 0x801d67b8, 0x801d67d4, 0x801d68e8, 0x801d690c,
            0x801d7058, 0x801d7084, 0x801d70c0, 0x801d70e8, 0x801d70f8, 0x801d7114, 0x801d7124,
            0x801d7140, 0x801d71b4, 0x801d7f2c, 0x801d7f68, 0x801d85a8, 0x801d8e34, 0x801d8e54,
            0x801d933c, 0x801d9360, 0x801d9378, 0x801d93a0, 0x801d93c4, 0x801d93e0,
        ],
        allocations: &[
            Allocation::bounded_format(0x801d0c04, 4, Codec::AsciiU16DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(
                0x801d0c28,
                4,
                Codec::AsciiI32DecimalFormat,
                &[0x8014ff0c, 0x80150098],
            ),
            Allocation::bounded_format(0x801d0c2c, 8, Codec::AsciiFormatLiteral, &[0x80150098]),
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
            Allocation::bounded_format(0x801d0d00, 5, Codec::AsciiU8DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(
                0x801d0d40,
                8,
                Codec::AsciiU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0d64, 4, Codec::AsciiU8DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(
                0x801d0d68,
                10,
                Codec::AsciiU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::ascii_literal(0x801d0d74, 4),
            Allocation::literal(0x801d9b4c, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9b50, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9b54, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9b58, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9ba8, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9bae, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9bb4, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9bba, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9d18, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9d1e, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9d24, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9d2a, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9d38, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9d3e, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9d44, 4, &[], &[0x8014f800]),
            Allocation::literal(0x801d9e48, 5, &[0x801d9e70], &[0x8014f800]),
            Allocation::literal(0x801d9e50, 7, &[0x801d9e74], &[0x8014f800]),
            Allocation::literal(0x801d9e58, 6, &[0x801d9e78], &[0x8014f800]),
            Allocation::literal(0x801d9e60, 7, &[0x801d9e7c], &[0x8014f800]),
            Allocation::literal(0x801d9e68, 6, &[0x801d9e80], &[0x8014f800]),
            Allocation::literal(0x801d9e84, 5, &[0x801d9ea4], &[0x8014f800]),
            Allocation::literal(0x801d9e8c, 7, &[0x801d9ea8], &[0x8014f800]),
            Allocation::literal(0x801d9e94, 7, &[0x801d9eac], &[0x8014f800]),
            Allocation::literal(0x801d9e9c, 6, &[0x801d9eb0], &[0x8014f800]),
            Allocation::literal(0x801d9eb4, 5, &[0x801d9edc], &[0x8014f800]),
            Allocation::literal(0x801d9ebc, 7, &[0x801d9ee0], &[0x8014f800]),
            Allocation::literal(0x801d9ec4, 6, &[0x801d9ee4], &[0x8014f800]),
            Allocation::literal(0x801d9ecc, 7, &[0x801d9ee8], &[0x8014f800]),
            Allocation::literal(0x801d9ed4, 6, &[0x801d9eec], &[0x8014f800]),
        ],
    },
    Layout {
        name: "US SHOP labels and numeric formats",
        load: 0x801d0c00,
        size: 87660,
        sha256: "b57c4e56e2f0509284aa79755dbd0ebb2888551e37c9b972b1cfa685f9d39e1a",
        consumers: &[
            0x801d5894, 0x801d9a58, 0x801dca2c, 0x801dd0a8, 0x801dd934, 0x801ddfb0, 0x801dee64,
            0x801df00c, 0x801df6a8, 0x801e112c, 0x801e1e80, 0x801e2f04, 0x801e3284, 0x801e37f4,
            0x801da47c, 0x801dc6fc, 0x801dd578, 0x801ddcb0, 0x801d826c, 0x801d86d4, 0x801d8c58,
            0x801d9254, 0x801da2f4, 0x801db0dc, 0x801db9f0, 0x801d3b98, 0x801d71f0,
        ],
        allocations: &[
            Allocation::bounded_format(0x801d0c68, 20, Codec::AsciiU8DecimalTripleFormat, &[]),
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
            Allocation::literal(0x801e5b20, 5, &[0x801e5b48], &[0x8014f800]),
            Allocation::literal(0x801e5b28, 7, &[0x801e5b4c], &[0x8014f800]),
            Allocation::literal(0x801e5b30, 6, &[0x801e5b50], &[0x8014f800]),
            Allocation::literal(0x801e5b38, 7, &[0x801e5b54], &[0x8014f800]),
            Allocation::literal(0x801e5b40, 6, &[0x801e5b58], &[0x8014f800]),
            Allocation::literal(0x801e5b5c, 5, &[0x801e5b7c], &[0x8014f800]),
            Allocation::literal(0x801e5b64, 7, &[0x801e5b80], &[0x8014f800]),
            Allocation::literal(0x801e5b6c, 7, &[0x801e5b84], &[0x8014f800]),
            Allocation::literal(0x801e5b74, 6, &[0x801e5b88], &[0x8014f800]),
            Allocation::literal(0x801e5b8c, 5, &[0x801e5bb4], &[0x8014f800]),
            Allocation::literal(0x801e5b94, 7, &[0x801e5bb8], &[0x8014f800]),
            Allocation::literal(0x801e5b9c, 6, &[0x801e5bbc], &[0x8014f800]),
            Allocation::literal(0x801e5ba4, 7, &[0x801e5bc0], &[0x8014f800]),
            Allocation::literal(0x801e5bac, 6, &[0x801e5bc4], &[0x8014f800]),
            Allocation::bounded_format(0x801d0e58, 2, Codec::AsciiDigitGlyphLiteral, &[0x8014ff0c]),
            Allocation::bounded_format(0x801d0e5c, 2, Codec::AsciiDigitGlyphLiteral, &[0x8014ff0c]),
            Allocation::bounded_format(0x801d0e60, 3, Codec::AsciiU8DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(0x801d0e64, 4, Codec::AsciiU8DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(
                0x801d0e68,
                8,
                Codec::AsciiU16DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(
                0x801d0e70,
                8,
                Codec::AsciiU16DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(
                0x801d0e7c,
                8,
                Codec::AsciiU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0e84, 4, Codec::AsciiU8DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(
                0x801d0e88,
                5,
                Codec::AsciiPriceDecimalFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(
                0x801d0e90,
                5,
                Codec::AsciiTotalDecimalFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(
                0x801d0e98,
                5,
                Codec::AsciiU8DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0de0, 5, Codec::AsciiU8DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(
                0x801d0e20,
                8,
                Codec::AsciiU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0e44, 4, Codec::AsciiU8DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(
                0x801d0e48,
                10,
                Codec::AsciiU8DecimalPairFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(0x801d0ce4, 4, Codec::AsciiU16DecimalFormat, &[0x80150098]),
            Allocation::bounded_format(
                0x801d0ce8,
                8,
                Codec::AsciiU16DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(
                0x801d0cf0,
                8,
                Codec::AsciiU16DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::bounded_format(
                0x801d0cf8,
                4,
                Codec::AsciiI32WideDecimalFormat,
                &[0x8014ff0c],
            ),
            Allocation::bounded_format(
                0x801d0d08,
                4,
                Codec::AsciiI32DecimalFormat,
                &[0x8014ff0c, 0x80150098],
            ),
            Allocation::bounded_format(0x801d0cfc, 9, Codec::AsciiDashGlyphLiteral, &[0x8014ff0c]),
            Allocation::bounded_format(0x801d0d0c, 8, Codec::AsciiDashGlyphLiteral, &[0x80150098]),
            Allocation::bounded_format(0x801d0e54, 4, Codec::AsciiDigitGlyphLiteral, &[0x80150098]),
            Allocation::bounded_format(0x801d0c1c, 3, Codec::AsciiU16DecimalFormat, &[]),
            Allocation::shift_jis_digit(0x801e540c, &[0x801e5434]),
            Allocation::shift_jis_digit(0x801e5410, &[0x801e5438]),
            Allocation::shift_jis_digit(0x801e5414, &[0x801e543c]),
            Allocation::shift_jis_digit(0x801e5418, &[0x801e5440]),
            Allocation::shift_jis_digit(0x801e541c, &[0x801e5444]),
            Allocation::shift_jis_digit(0x801e5420, &[0x801e5448]),
            Allocation::shift_jis_digit(0x801e5424, &[0x801e544c]),
            Allocation::shift_jis_digit(0x801e5428, &[0x801e5450]),
            Allocation::shift_jis_digit(0x801e542c, &[0x801e5454]),
            Allocation::shift_jis_digit(0x801e5430, &[0x801e5458]),
            Allocation::bounded_format(0x801d0c98, 65, Codec::ShiftJisTitleFormat, &[]),
        ],
    },
    Layout {
        name: "US battle-results text",
        load: 0x801eec00,
        size: 8417,
        sha256: "2fd669faf11a0033b8d21079204c1bbc56a255a177425d14248fb8f5212db1c0",
        consumers: &[
            0x801eecd0, 0x801ef140, 0x801ef348, 0x801efe5c, 0x801f07e0, 0x801f09f4, 0x801f0a74,
            0x801eed50, 0x801ef1f8, 0x801ef48c, 0x801efed0, 0x801eff70, 0x801f0018, 0x801f00c0,
            0x801f0168, 0x801f0210, 0x801f08a0, 0x801f08b4, 0x801f08f4, 0x801f0908, 0x801f0a24,
            0x801f0a38, 0x801f0a58, 0x801f0b58, 0x801f0b78,
        ],
        allocations: &[
            Allocation::bounded_format(0x801eec04, 3, Codec::AsciiI32DecimalFormat, &[0x8014f800]),
            Allocation::bounded_format(0x801eec40, 3, Codec::AsciiU16DecimalFormat, &[0x8014f800]),
            Allocation::bounded_format(0x801eec44, 4, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(
                0x801eec48,
                4,
                Codec::AsciiU8PlusOneDecimalFormat,
                &[0x8014ff0c],
            ),
            Allocation::bounded_format(0x801eec4c, 4, Codec::AsciiI32DecimalFormat, &[0x8014ff0c]),
            Allocation::bounded_format(
                0x801eec50,
                5,
                Codec::AsciiU8DecimalFieldFormat,
                &[0x80150098],
            ),
            Allocation::literal(0x801f0cd0, 2, &[], &[0x8014f800]),
        ],
    },
    Layout {
        name: "US battle messages",
        load: 0x801d0c00,
        size: 118224,
        sha256: "af7fa06e57f9d420f3486607e7e82e23faaba5e39391d150c43ef3ac3f1d0b29",
        consumers: &[
            0x801de94c, 0x801de560, 0x801deaa4, 0x801deac4, 0x801e9538, 0x801d9524, 0x801d9544,
            0x801d96cc,
        ],
        // Indexed records and direct code references, rather than a pointer table.
        // Row 1 appends at most five encoded bytes. Row 3 remains unsupported.
        allocations: &[
            Allocation::literal(0x801eb000, 13, &[], &[0x8014f800]),
            Allocation::glyphs(0x801eb00d, 6),
            Allocation::literal(0x801eb01a, 13, &[], &[0x8014f800]),
            Allocation::literal(0x801eb034, 13, &[], &[]),
            Allocation::literal(0x801eb041, 13, &[], &[]),
            Allocation::literal(0x801eb04e, 13, &[], &[0x8014f800]),
            Allocation::literal(0x801eb05b, 13, &[], &[0x8014f800]),
            Allocation::literal(0x801eb068, 13, &[], &[0x8014f800]),
            Allocation::literal(0x801eb075, 13, &[], &[0x8014f800]),
            Allocation::literal(0x801eb082, 13, &[], &[0x8014f800]),
            Allocation::literal(0x801eb08f, 13, &[], &[0x8014f800]),
            Allocation::bounded_format(0x801d0c70, 2, Codec::AsciiDigitGlyphLiteral, &[0x801d94d4]),
            Allocation::bounded_format(0x801d0c74, 4, Codec::AsciiU16DecimalFormat, &[0x801d94d4]),
            Allocation::bounded_format(0x801d0c78, 4, Codec::AsciiU16DecimalFormat, &[0x801d9684]),
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
    pub renderers: Vec<u32>,
    pub(crate) codec: Codec,
    pub(crate) references: Vec<u32>,
    text: String,
}

fn layout_for(data: &[u8], entry: Entry, payload: &[u8]) -> Option<&'static Layout> {
    LAYOUTS.iter().find(|layout| {
        if entry.ram_ptr != layout.load || payload.len() != layout.size {
            return false;
        }
        let mut masked = payload.to_vec();
        if layout.load == crate::game::LOAD && layout.size == crate::game::PAYLOAD_SIZE {
            if entry.index != 0 || entry.offset != 0x800 || !crate::game::supports_archive(data) {
                return false;
            }
            crate::game::mask_text(&mut masked);
        }
        for allocation in layout.allocations {
            let at = (allocation.address - layout.load) as usize;
            masked[at..at + allocation.capacity].fill(0);
        }
        sha256_hex(&masked) == layout.sha256
    })
}

pub fn supports(data: &[u8], entry: Entry, payload: &[u8]) -> bool {
    layout_for(data, entry, payload).is_some()
}

pub fn extract(path: &Path, data: &[u8], entry: Entry, payload: &[u8]) -> Result<Document> {
    let layout = layout_for(data, entry, payload)
        .ok_or_else(|| Error::Invalid("unsupported inline text layout".into()))?;
    let strings = layout
        .allocations
        .iter()
        .enumerate()
        .map(|(index, allocation)| {
            let Allocation {
                address,
                capacity,
                references,
                renderers,
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
                renderers: renderers.to_vec(),
                codec,
                references: references.to_vec(),
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
            references,
            renderers,
            codec,
        } = *allocation;
        let offset = (address - layout.load) as usize;
        if !seen.insert(row.index)
            || row.address != address
            || row.payload_offset != offset
            || document.entry_offset.checked_add(offset) != Some(row.file_offset)
            || row.capacity != capacity
            || row.renderers != renderers
            || row.codec != codec
            || row.references != references
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
        || !layout_for(data, entry, payload).is_some_and(|layout| layout.name == checked.layout)
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
