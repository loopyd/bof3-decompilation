//! Five-byte sprite parts consumed by SLUS_004.22@0x8014C6F8.
use crate::{Error, Result, animation};
use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, BTreeSet};
fn invalid(reason: &str) -> Error {
    Error::Codec(format!("sprite cell: {reason}"))
}
fn extent(offset: usize, starts: &BTreeSet<usize>, end: usize) -> usize {
    starts.range(offset + 1..).next().copied().unwrap_or(end)
}
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Part {
    pub size_code: u8,
    pub horizontal_flip: bool,
    pub opaque_flags: u8,
    pub x: i8,
    pub y: i8,
    pub u: u8,
    pub v: u8,
}

impl Part {
    /// Original width/height table at SLUS_004.22@0x8017F53C.
    pub fn dimensions(&self) -> Result<(u8, u8)> {
        if self.size_code > 15 {
            return Err(invalid("cell size code exceeds 15"));
        }
        Ok((
            8 * (1 + (self.size_code & 3)),
            8 * (1 + (self.size_code >> 2)),
        ))
    }
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Cell {
    pub offset: usize,
    pub end: usize,
    pub parts: Vec<Part>,
}

pub(crate) fn decode_cells(
    source: &[u8],
    offset: usize,
    end: usize,
    program: &animation::Program,
    cells: &mut BTreeMap<usize, Cell>,
) -> Result<()> {
    let table = offset + 2 + program.steps().len() * 2;
    let starts: BTreeSet<_> = program
        .cell_offsets()
        .iter()
        .map(|relative| table + usize::from(*relative))
        .collect();
    for &offset in &starts {
        let end = extent(offset, &starts, end);
        if let Some(cell) = cells.get(&offset) {
            if cell.end != end {
                return Err(invalid("aliased cells disagree on their allocation"));
            }
            continue;
        }
        let count = usize::from(source[offset]);
        let record_end = offset + 1 + count * 5;
        if record_end > end {
            return Err(invalid(&format!(
                "cell at byte {offset} has {count} parts beyond its extent"
            )));
        }
        let parts = source[offset + 1..record_end]
            .as_chunks::<5>()
            .0
            .iter()
            .map(|bytes| Part {
                size_code: bytes[0] & 15,
                horizontal_flip: bytes[0] & 0x80 != 0,
                opaque_flags: bytes[0] & 0x70,
                x: bytes[1] as i8,
                y: bytes[2] as i8,
                u: bytes[3],
                v: bytes[4],
            })
            .collect();
        cells.insert(offset, Cell { offset, end, parts });
    }
    Ok(())
}

pub(crate) fn encode_cells(output: &mut [u8], original: &[Cell], edited: &[Cell]) -> Result<()> {
    if edited.len() != original.len() {
        return Err(invalid("edit changes the cell allocation"));
    }
    for (cell, raw) in edited.iter().zip(original) {
        if cell.offset != raw.offset || cell.end != raw.end || cell.parts.len() != raw.parts.len() {
            return Err(invalid("edit changes a cell extent or part count"));
        }
        for (index, (part, raw)) in cell.parts.iter().zip(&raw.parts).enumerate() {
            part.dimensions()?;
            if part.opaque_flags != raw.opaque_flags {
                return Err(invalid("edit changes opaque cell flag bits"));
            }
            let start = cell.offset + 1 + index * 5;
            output[start..start + 5].copy_from_slice(&[
                part.size_code | (u8::from(part.horizontal_flip) << 7) | part.opaque_flags,
                part.x as u8,
                part.y as u8,
                part.u,
                part.v,
            ]);
        }
    }
    Ok(())
}
