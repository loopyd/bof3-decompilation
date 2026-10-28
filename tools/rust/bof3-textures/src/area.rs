//! Conditional area-cell records consumed by SLUS_004.22@0x80154294/0x80155560.

use std::collections::BTreeSet;

use serde::{Deserialize, Serialize};

use crate::{Error, Result, digest::sha256_hex};

pub const SCHEMA: &str = "bof3.area-surface/v1";
const MAX_BYTES: usize = 1024 * 1024;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub width: u8,
    pub height: u8,
    pub grid_cells: usize,
    pub selected_records: usize,
    pub coordinate_spans: usize,
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
