//! Conditional area-cell records consumed by SLUS_004.22@0x80154294/0x80155560.

use std::collections::BTreeSet;

use serde::{Deserialize, Serialize};

use crate::{Error, Result, digest::sha256_hex};

pub const SCHEMA: &str = "bof3.area-surface/v1";
const MAX_BYTES: usize = 1024 * 1024;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct SourceRequest {
    pub index: u16,
    pub slot: u16,
    pub lba: u32,
}

// Original EXE table 0x80182444, slots 683..882. Each selected original-disc
// extent equals its complete qualified AREA archive, across WORLD directories.
const AREA_LBAS: [u32; 200] = [
    0x1acb8, 0x1ae50, 0x1afdd, 0x1b166, 0x1b2e0, 0x1b490, 0x1b5e9, 0x1b76e, 0x1b8d6, 0x1ba56,
    0x1bbcc, 0x1bd57, 0x1bece, 0x1c031, 0x1c199, 0x1c2fc, 0x1c447, 0x1c5d3, 0x1c712, 0x1c899,
    0x1c9d4, 0x1cb56, 0x1ccd7, 0x1ce76, 0x1d01e, 0x1d1d3, 0x1d351, 0x1d4e9, 0x1d684, 0x1d820,
    0x1d9a7, 0x1db93, 0x1dd0a, 0x1dea9, 0x1e035, 0x1e17a, 0x1e2d8, 0x1e408, 0x1e5a8, 0x1e735,
    0x1e8c9, 0x1ea53, 0x1ebea, 0x1ed70, 0x1eee4, 0x1f081, 0x1f20f, 0x1f39d, 0x1f514, 0x1f6b5,
    0x1f851, 0x1f9c5, 0x1fb2c, 0x1fcbe, 0x1fded, 0x1ff7b, 0x200f4, 0x2028a, 0x20427, 0x205bc,
    0x206f0, 0x2087a, 0x209f5, 0x20b73, 0x20cf6, 0x20e80, 0x21004, 0x21148, 0x212ec, 0x21443,
    0x21597, 0x2172c, 0x218c2, 0x21a4a, 0x21bdc, 0x21d73, 0x21efe, 0x2208e, 0x22226, 0x223be,
    0x2255b, 0x226f7, 0x2288f, 0x22a3a, 0x22bde, 0x22d7e, 0x22f15, 0x23053, 0x231de, 0x2331f,
    0x2350b, 0x23698, 0x237ee, 0x23933, 0x23aa9, 0x23c48, 0x23da9, 0x23f0a, 0x24093, 0x24224,
    0x243c9, 0x24545, 0x24681, 0x247c2, 0x2495d, 0x24aec, 0x24c6e, 0x24df1, 0x24f6a, 0x2511c,
    0x252ab, 0x2542a, 0x255c7, 0x25751, 0x258ca, 0x25a47, 0x25bd2, 0x25d0a, 0x25e63, 0x25fbf,
    0x26167, 0x262f4, 0x2648a, 0x265ff, 0x26774, 0x268f9, 0x26a7c, 0x26c10, 0x26da9, 0x26f23,
    0x2710f, 0x27257, 0x273eb, 0x27584, 0x27719, 0x278ac, 0x27a59, 0x27bfb, 0x27d91, 0x27ef3,
    0x2805c, 0x281c9, 0x2833a, 0x284d8, 0x2865d, 0x287ca, 0x28965, 0x28aa9, 0x28c3f, 0x28de6,
    0x28f57, 0x290e9, 0x29276, 0x293ff, 0x2958c, 0x29716, 0x298a5, 0x29a20, 0x29b51, 0x29c79,
    0x29d9a, 0x29ebf, 0x29fe8, 0x2a117, 0x2a23a, 0x2a358, 0x2a485, 0x2a5ac, 0x2a751, 0x2a8c6,
    0x2aa09, 0x2aba5, 0x2acfc, 0x2ae81, 0x2b012, 0x2b1b9, 0x2b355, 0x2b4e7, 0x2b67a, 0x2b80d,
    0x2b9a0, 0x2bb33, 0x2bcc6, 0x2be59, 0x2bfed, 0x2c181, 0x2c315, 0x2c482, 0x2c5f9, 0x2c794,
    0x2c925, 0x2caab, 0x2cbe0, 0x2cd67, 0x2cee1, 0x2d020, 0x2d1a9, 0x2d2f0, 0x2d499, 0x2d5fb,
];

/// Called only for already qualified original type-0/load-0x80104000 sources.
/// The original GAME path requests current (s0 & 0xffff) + 683 conditionally;
/// these known entries do not establish a bound on that unchecked argument.
pub(crate) fn source_request(source: &str) -> Option<SourceRequest> {
    if !source.starts_with("BIN/WORLD") {
        return None;
    }
    let number = source
        .rsplit('/')
        .next()?
        .strip_prefix("AREA")?
        .strip_suffix(".EMI")?;
    if number.len() != 3 || !number.bytes().all(|byte| byte.is_ascii_digit()) {
        return None;
    }
    let index: u16 = number.parse().ok()?;
    let lba = *AREA_LBAS.get(usize::from(index))?;
    Some(SourceRequest {
        index,
        slot: 683 + index,
        lba,
    })
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub width: u8,
    pub height: u8,
    pub grid_cells: usize,
    pub selected_records: usize,
    pub coordinate_spans: usize,
    pub request: Option<SourceRequest>,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Record {
    pub offset: usize,
    pub word: u32,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Document {
    pub schema: String,
    pub width: u8,
    pub height: u8,
    pub grid_offset: usize,
    pub grid_sha256: String,
    pub records: Vec<Record>,
}

/// Observed source read, not a writable allocation or complete table extent.
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct Span {
    pub offset: usize,
    pub size: usize,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct View {
    pub uv: [[u8; 2]; 4],
    pub clut: u16,
    pub normal_page: u16,
    pub alternate_page: u16,
    pub gray_rgb: u8,
    pub semi_transparency: bool,
    pub coordinate_read: Option<Span>,
}

fn invalid(reason: &str) -> Error {
    Error::Codec(format!("area surface: {reason}"))
}

fn bytes(source: &[u8], offset: usize, size: usize) -> Result<&[u8]> {
    let end = offset
        .checked_add(size)
        .ok_or_else(|| invalid("source extent overflows"))?;
    source
        .get(offset..end)
        .ok_or_else(|| invalid("selected record exceeds its payload"))
}

fn half(source: &[u8], offset: usize) -> Result<usize> {
    Ok(usize::from(u16::from_le_bytes(
        bytes(source, offset, 2)?.try_into().unwrap(),
    )))
}

/// Decode source-selected coordinate fields. Page alternatives require the
/// original getter branch; neither alternative binds physical texels or a CLUT.
pub fn resolve(source: &[u8], word: u32) -> Result<View> {
    if source.len() > MAX_BYTES {
        return Err(invalid("source exceeds 1 MiB"));
    }
    let selector = (word >> 28) & 3;
    let (mut uv, coordinate_read) = if word & 0xf00 == 0 {
        let u = ((word & 15) * 16) as u8;
        let v = (word & 0xf0) as u8;
        ([[u, v], [u | 15, v], [u, v | 15], [u | 15, v | 15]], None)
    } else {
        let paired = word & 0x800 != 0;
        let size = if paired { 8 } else { 4 };
        let header = if paired { 6 } else { 4 } + 4 * selector as usize;
        let index = (word & if paired { 0x7ff } else { 0xff }) as usize;
        let offset = half(source, header)? * 4 + index * size;
        let record = bytes(source, offset, size)?;
        let uv = if paired {
            [
                [record[3], record[2]],
                [record[1], record[0]],
                [record[7], record[6]],
                [record[5], record[4]],
            ]
        } else {
            let u = record[3];
            let v = record[2];
            let end_u = u.wrapping_add(record[1]);
            let end_v = v.wrapping_add(record[0]);
            [[u, v], [end_u, v], [u, end_v], [end_u, end_v]]
        };
        (uv, Some(Span { offset, size }))
    };
    for (bit, permutation) in [(18, [2, 0, 3, 1]), (16, [1, 0, 3, 2]), (17, [2, 3, 0, 1])] {
        if word & (1 << bit) != 0 {
            uv = permutation.map(|index| uv[index]);
        }
    }
    let indexed_8 = u32::from(word & 0x8000_0000 == 0);
    let palette = (word >> 24) & 15;
    let clut = ((0x1e3 + palette * indexed_8) << 6) | if indexed_8 == 0 { palette } else { 0 };
    Ok(View {
        uv,
        clut: clut as u16,
        normal_page: ((indexed_8 << 7) | ((word >> 7) & 0x60) | 0x10 | (5 + 2 * selector)) as u16,
        alternate_page: ((indexed_8 << 9) | ((word >> 5) & 0x180) | 0x20 | (5 + 2 * selector))
            as u16,
        gray_rgb: ((word >> 16) & 0xf8) as u8,
        semi_transparency: word & 0x8000 != 0,
        coordinate_read,
    })
}

pub fn decode(source: &[u8]) -> Result<Document> {
    if source.len() > MAX_BYTES {
        return Err(invalid("source exceeds 1 MiB"));
    }
    let header = bytes(source, 0, 22)?;
    let width = header[0];
    let height = header[1];
    if width == 0 || height == 0 {
        return Err(invalid("zero grid dimensions"));
    }
    let cells = usize::from(width) * usize::from(height);
    let grid_offset = half(source, 2)? * 4;
    let grid = bytes(source, grid_offset, 2 * cells)?;
    bytes(source, half(source, 20)? * 4, cells)?;
    let records_offset = grid_offset + 4 * cells.div_ceil(2);
    let ids: BTreeSet<_> = grid
        .as_chunks::<2>()
        .0
        .iter()
        .map(|pair| u16::from_le_bytes(*pair))
        .filter(|id| *id != 0)
        .collect();
    let mut records = Vec::with_capacity(ids.len());
    for id in ids {
        let offset = records_offset + 4 * usize::from(id);
        let word = u32::from_le_bytes(bytes(source, offset, 4)?.try_into().unwrap());
        resolve(source, word)?;
        records.push(Record { offset, word });
    }
    Ok(Document {
        schema: SCHEMA.into(),
        width,
        height,
        grid_offset,
        grid_sha256: sha256_hex(grid),
        records,
    })
}

impl Document {
    pub fn coordinate_reads(&self, source: &[u8]) -> Result<BTreeSet<Span>> {
        let mut reads = BTreeSet::new();
        for record in &self.records {
            if let Some(span) = resolve(source, record.word)?.coordinate_read {
                reads.insert(span);
            }
        }
        Ok(reads)
    }

    pub fn info(&self, source: &[u8]) -> Result<Info> {
        Ok(Info {
            width: self.width,
            height: self.height,
            grid_cells: usize::from(self.width) * usize::from(self.height),
            selected_records: self.records.len(),
            coordinate_spans: self.coordinate_reads(source)?.len(),
            request: None,
        })
    }
}

/// Only original directly selected words are editable. All other bytes remain
/// original, including source coordinates that alias a selected word.
pub fn encode(original: &[u8], document: &Document) -> Result<Vec<u8>> {
    let before = decode(original)?;
    let mut shape = document.clone();
    for (record, old) in shape.records.iter_mut().zip(&before.records) {
        record.word = old.word;
    }
    if shape != before {
        return Err(invalid(
            "preserve schema, dimensions, grid identity and record locators",
        ));
    }
    let reads = before.coordinate_reads(original)?;
    let mut protected = vec![false; original.len()];
    let mut allowed_coordinates = vec![false; original.len()];
    for span in &reads {
        allowed_coordinates[span.offset..span.offset + span.size].fill(true);
    }
    for (offset, size) in [
        (0, 22),
        (
            before.grid_offset,
            usize::from(before.width) * usize::from(before.height) * 2,
        ),
    ]
    .into_iter()
    .chain(reads.iter().map(|span| (span.offset, span.size)))
    .chain(std::iter::once((
        half(original, 20)? * 4,
        usize::from(before.width) * usize::from(before.height),
    ))) {
        protected[offset..offset + size].fill(true);
    }
    let mut edited = original.to_vec();
    for record in &document.records {
        let encoded = record.word.to_le_bytes();
        for (index, byte) in encoded.into_iter().enumerate() {
            let offset = record.offset + index;
            if protected[offset] && byte != original[offset] {
                return Err(invalid("packed-word edit overlaps immutable source data"));
            }
            edited[offset] = byte;
        }
    }
    // Resolve the complete edit together, never against stale per-word inputs.
    for record in &document.records {
        if let Some(span) = resolve(&edited, record.word)?.coordinate_read
            && !allowed_coordinates[span.offset..span.offset + span.size]
                .iter()
                .all(|allowed| *allowed)
        {
            return Err(invalid(
                "edit selects coordinates outside original qualified read spans",
            ));
        }
    }
    if decode(&edited)? != *document {
        return Err(invalid(
            "edited surface metadata does not re-decode to its intent",
        ));
    }
    Ok(edited)
}
