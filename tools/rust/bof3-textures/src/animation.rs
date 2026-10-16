//! Program tables and forward consumers at SLUS_004.22@0x8014D910/0x8014D978/0x8014DAEC.

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
}

impl Program {
    pub fn steps(&self) -> &[Step] {
        &self.steps
    }

    pub fn cell_offsets(&self) -> &[u16] {
        &self.cell_offsets
    }

    fn fetch(&self, step: usize) -> Result<(u8, usize)> {
        let token = self
            .steps
            .get(step)
            .ok_or_else(|| Error::Codec(format!("animation step {step} exceeds its program")))?;
        let offset = self
            .cell_offsets
            .get(usize::from(token.operand))
            .ok_or_else(|| {
                Error::Codec(format!(
                    "animation step {step} selects missing cell {} during direct fetch",
                    token.operand
                ))
            })?;
        Ok((token.duration, self.cell_table + usize::from(*offset)))
    }

    /// Initialization fetches a token directly, without testing its control bit.
    pub fn start(&self, step: usize) -> Result<Playback<'_>> {
        let (remaining, cell) = self.fetch(step)?;
        Ok(Playback {
            program: self,
            next_step: step + 1,
            remaining,
            cell,
            stopped: false,
        })
    }
}

/// Parse this bounded table variant in decoded-bank byte coordinates.
/// The caller supplies the qualified program offset and containing extent.
/// Cell payloads remain opaque; their addresses must lie after the offset table.
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
    for (index, token) in steps.iter().enumerate() {
        let limit = if token.duration < 0x80 { cells } else { count };
        if usize::from(token.operand) >= limit {
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
pub enum ForwardMode {
    Loop,
    Stop,
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
    next_step: usize,
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

    /// One consumer update, whose frequency depends on the game caller.
    /// The unsigned counter wraps: a fetched zero duration lasts 256 updates.
    pub fn advance(&mut self, mode: ForwardMode) -> Result<Advance> {
        if self.stopped && mode == ForwardMode::Stop {
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
        let mut target = self.next_step;
        let mut boundary = false;
        if target == self.program.steps.len() {
            if mode == ForwardMode::Stop {
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
        } else {
            let token = self.program.steps[target];
            if token.duration >= 0x80 {
                target = usize::from(token.operand);
                boundary = true;
            }
        }
        let (remaining, cell) = self.program.fetch(target)?;
        self.remaining = remaining;
        self.cell = cell;
        self.next_step = target + 1;
        Ok(Advance {
            refreshed: true,
            boundary,
            stopped: false,
        })
    }
}
