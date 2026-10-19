//! Program tables and forward/reverse consumers at SLUS_004.22@0x8014D910..0x8014DAEC.

use serde::{Deserialize, Serialize};

use crate::{Error, Result};

#[derive(Clone, Copy, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Step {
    pub duration: u8,
    pub operand: u8,
}

#[derive(Clone, Debug, Serialize, PartialEq, Eq)]
pub struct Program {
    offset: usize,
    cell_table: usize,
    steps: Vec<Step>,
    cell_offsets: Vec<u16>,
    metadata_cells: Vec<MetadataCell>,
}

#[derive(Clone, Debug, Serialize, PartialEq, Eq)]
pub struct MetadataCell {
    pub operand: u8,
    pub lookup_offset: usize,
    pub relative_offset: u16,
    pub cell_offset: usize,
    pub part_count: u8,
}

impl Program {
    pub fn steps(&self) -> &[Step] {
        &self.steps
    }

    pub fn cell_offsets(&self) -> &[u16] {
        &self.cell_offsets
    }

    pub fn metadata_cells(&self) -> &[MetadataCell] {
        &self.metadata_cells
    }

    pub(crate) fn validate_preserved(&self, source: &[u8], output: &[u8]) -> Result<()> {
        if self.metadata_cells.is_empty() {
            return Ok(());
        }
        let metadata_end = self.cell_table + self.cell_offsets.len() * 2;
        if source.get(self.offset..metadata_end) != output.get(self.offset..metadata_end) {
            return Err(Error::Codec(
                "animation metadata alias requires unchanged header, steps and declared offsets"
                    .into(),
            ));
        }
        for alias in &self.metadata_cells {
            if output.get(alias.lookup_offset..alias.lookup_offset + 2)
                != Some(alias.relative_offset.to_le_bytes().as_slice())
                || output.get(alias.cell_offset) != Some(&alias.part_count)
            {
                return Err(Error::Codec(
                    "animation edit changes an original metadata-alias lookup or zero-part cell"
                        .into(),
                ));
            }
        }
        Ok(())
    }

    fn fetch(&self, step: usize) -> Result<(u8, usize)> {
        let token = self
            .steps
            .get(step)
            .ok_or_else(|| Error::Codec(format!("animation step {step} exceeds its program")))?;
        let offset = self
            .cell_offsets
            .get(usize::from(token.operand))
            .map(|offset| self.cell_table + usize::from(*offset))
            .or_else(|| {
                if token.duration >= 0x80 {
                    return None;
                }
                self.metadata_cells
                    .iter()
                    .find(|alias| alias.operand == token.operand)
                    .map(|alias| alias.cell_offset)
            })
            .ok_or_else(|| {
                Error::Codec(format!(
                    "animation step {step} selects missing cell {} during direct fetch",
                    token.operand
                ))
            })?;
        Ok((token.duration, offset))
    }

    /// Initialization fetches a token directly, without testing its control bit.
    pub fn start(&self, step: usize) -> Result<Playback<'_>> {
        let (remaining, cell) = self.fetch(step)?;
        Ok(Playback {
            program: self,
            cursor: step + 1,
            remaining,
            cell,
            stopped: false,
        })
    }
}

/// Parse this bounded table variant in decoded-bank byte coordinates.
/// The caller supplies the qualified program offset and containing extent.
/// Declared cells follow the offset table. Normal tokens may additionally resolve
/// a zero-part cell inside that immutable table through an out-of-table lookup.
pub fn decode(source: &[u8], offset: usize, end: usize) -> Result<Program> {
    if source.len() > 1024 * 1024 || end > source.len() {
        return Err(Error::Codec(
            "animation source exceeds its decoded-bank bound".into(),
        ));
    }
    let bytes = source
        .get(offset..end)
        .ok_or_else(|| Error::Codec("animation program exceeds its source extent".into()))?;
    let header = bytes
        .get(..2)
        .ok_or_else(|| Error::Codec("animation program lacks its two-byte header".into()))?;
    let cells = usize::from(header[0]);
    let count = usize::from(header[1]);
    if cells == 0 || count == 0 {
        return Err(Error::Codec(
            "animation step and cell tables must be nonempty".into(),
        ));
    }
    let cell_table = 2 + count * 2;
    let metadata_end = cell_table + cells * 2;
    let metadata = bytes
        .get(..metadata_end)
        .ok_or_else(|| Error::Codec("animation step or cell-offset table is truncated".into()))?;
    let steps: Vec<_> = metadata[2..cell_table]
        .as_chunks::<2>()
        .0
        .iter()
        .map(|pair| Step {
            duration: pair[0],
            operand: pair[1],
        })
        .collect();
    let cell_offsets: Vec<_> = metadata[cell_table..]
        .as_chunks::<2>()
        .0
        .iter()
        .map(|pair| u16::from_le_bytes([pair[0], pair[1]]))
        .collect();
    if usize::from(cell_offsets[0]) != cells * 2 {
        return Err(Error::Codec(
            "animation first cell offset disagrees with its table size".into(),
        ));
    }
    for relative in &cell_offsets {
        let address = cell_table + usize::from(*relative);
        if address < metadata_end || address >= bytes.len() {
            return Err(Error::Codec(
                "animation cell offset exceeds its payload extent".into(),
            ));
        }
    }
    let mut metadata_cells: Vec<MetadataCell> = Vec::new();
    for (index, token) in steps.iter().enumerate() {
        let limit = if token.duration < 0x80 { cells } else { count };
        if usize::from(token.operand) >= limit {
            if token.duration < 0x80 {
                let lookup = cell_table + usize::from(token.operand) * 2;
                let word = bytes.get(lookup..lookup + 2).ok_or_else(|| {
                    Error::Codec("animation metadata lookup exceeds its program".into())
                })?;
                let relative_offset = u16::from_le_bytes(word.try_into().unwrap());
                let target = cell_table + usize::from(relative_offset);
                if !(cell_table..metadata_end).contains(&target) || bytes[target] != 0 {
                    return Err(Error::Codec(
                        "animation metadata alias must select a zero-part cell in its declared offset table".into(),
                    ));
                }
                if !metadata_cells
                    .iter()
                    .any(|alias| alias.operand == token.operand)
                {
                    metadata_cells.push(MetadataCell {
                        operand: token.operand,
                        lookup_offset: offset + lookup,
                        relative_offset,
                        cell_offset: offset + target,
                        part_count: bytes[target],
                    });
                }
                continue;
            }
            return Err(Error::Codec(format!(
                "animation step {index} operand {} exceeds its {} table",
                token.operand,
                if token.duration < 0x80 {
                    "cell"
                } else {
                    "control-target"
                }
            )));
        }
    }
    Ok(Program {
        offset,
        cell_table: offset + cell_table,
        steps,
        cell_offsets,
        metadata_cells,
    })
}

/// Change only existing step pairs, preserving header, cell offsets and payloads.
/// Keep each token's forward frame/control role and the original allocation.
pub fn encode_steps(source: &[u8], offset: usize, end: usize, steps: &[Step]) -> Result<Vec<u8>> {
    let original = decode(source, offset, end)?;
    if steps.len() != original.steps.len() {
        return Err(Error::Codec(
            "animation edit changes its step allocation".into(),
        ));
    }
    if !original.metadata_cells.is_empty() && steps != original.steps {
        return Err(Error::Codec(
            "animation metadata-alias program steps are read-only".into(),
        ));
    }
    if steps
        .iter()
        .zip(&original.steps)
        .any(|(edited, raw)| (edited.duration < 0x80) != (raw.duration < 0x80))
    {
        return Err(Error::Codec(
            "animation edit changes a token's forward control role".into(),
        ));
    }
    let mut output = source.to_vec();
    for (index, step) in steps.iter().enumerate() {
        let position = original.offset + 2 + index * 2;
        output[position..position + 2].copy_from_slice(&[step.duration, step.operand]);
    }
    original.validate_preserved(source, &output)?;
    let edited = decode(&output, offset, end)?;
    // A forward control branches once, then fetches its destination as a frame.
    // Repeated high tokens must retain this direct-fetch behavior, not recurse.
    for token in &edited.steps {
        if token.duration >= 0x80 {
            edited.fetch(usize::from(token.operand))?;
        }
    }
    edited.fetch(0)?;
    Ok(output)
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PlaybackMode {
    ForwardLoop,
    ForwardStop,
    ReverseLoop,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Advance {
    pub refreshed: bool,
    pub boundary: bool,
    pub stopped: bool,
}

#[derive(Debug)]
pub struct Playback<'a> {
    program: &'a Program,
    cursor: usize,
    remaining: u8,
    cell: usize,
    stopped: bool,
}

impl Playback<'_> {
    pub fn cell_address(&self) -> usize {
        self.cell
    }

    pub fn remaining(&self) -> u8 {
        self.remaining
    }

    /// Program-relative byte cursor measured from the first step pair.
    /// Direction changes retain this cursor, as the original consumers do.
    pub fn cursor(&self) -> usize {
        self.cursor * 2
    }

    /// One consumer update, whose frequency depends on the game caller.
    /// The unsigned counter wraps: a fetched zero duration lasts 256 updates.
    pub fn advance(&mut self, mode: PlaybackMode) -> Result<Advance> {
        if self.stopped && mode == PlaybackMode::ForwardStop {
            return Ok(Advance {
                refreshed: false,
                boundary: true,
                stopped: true,
            });
        }
        self.stopped = false;
        self.remaining = self.remaining.wrapping_sub(1);
        if self.remaining != 0 {
            return Ok(Advance {
                refreshed: false,
                boundary: false,
                stopped: false,
            });
        }
        let mut target = self.cursor;
        let mut boundary = false;
        let next_cursor;
        if mode == PlaybackMode::ReverseLoop {
            if target == 0 {
                target = self.program.steps.len();
                boundary = true;
            }
            target -= 1;
            next_cursor = target;
        } else if target == self.program.steps.len() {
            if mode == PlaybackMode::ForwardStop {
                self.remaining = 1;
                self.stopped = true;
                return Ok(Advance {
                    refreshed: false,
                    boundary: true,
                    stopped: true,
                });
            }
            target = 0;
            boundary = true;
            next_cursor = 1;
        } else {
            let token = self.program.steps[target];
            if token.duration >= 0x80 {
                target = usize::from(token.operand);
                boundary = true;
            }
            next_cursor = target + 1;
        }
        let (remaining, cell) = self.program.fetch(target)?;
        self.remaining = remaining;
        self.cell = cell;
        self.cursor = next_cursor;
        Ok(Advance {
            refreshed: true,
            boundary,
            stopped: false,
        })
    }
}
