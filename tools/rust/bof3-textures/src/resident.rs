//! Resident programs at SLUS_004.22@0x8014D878 and cells at 0x8014C6F8.

use std::collections::{BTreeMap, BTreeSet};

use serde::{Deserialize, Serialize};

use crate::{
    Error, Result, animation,
    cell::{self, Cell},
};

pub const SCHEMA: &str = "bof3.resident-animation/v3";
const MAX_PROGRAMS: usize = 65_536;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub groups: usize,
    pub program_indices: usize,
    pub unique_programs: usize,
    pub decoded_programs: usize,
    pub unresolved_programs: usize,
    pub cells: usize,
    pub parts: usize,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Sequence {
    pub offset: usize,
    pub end: usize,
    pub steps: Vec<animation::Step>,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(tag = "status", rename_all = "kebab-case", deny_unknown_fields)]
pub enum Program {
    Decoded(Sequence),
    Unresolved {
        offset: usize,
        end: usize,
        reason: String,
    },
}

impl Program {
    pub fn offset(&self) -> usize {
        match self {
            Self::Decoded(sequence) => sequence.offset,
            Self::Unresolved { offset, .. } => *offset,
        }
    }

    pub fn end(&self) -> usize {
        match self {
            Self::Decoded(sequence) => sequence.end,
            Self::Unresolved { end, .. } => *end,
        }
    }

    pub fn sequence(&self) -> Result<&Sequence> {
        match self {
            Self::Decoded(sequence) => Ok(sequence),
            Self::Unresolved { offset, reason, .. } => Err(invalid(&format!(
                "program at byte {offset} is unresolved: {reason}"
            ))),
        }
    }
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Group {
    pub offset: usize,
    pub programs: Vec<Program>,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Document {
    pub schema: String,
    pub groups: Vec<Group>,
    pub cells: Vec<Cell>,
}

impl Document {
    pub fn info(&self) -> Info {
        Info {
            groups: self.groups.len(),
            program_indices: self.groups.iter().map(|group| group.programs.len()).sum(),
            unique_programs: self
                .groups
                .iter()
                .flat_map(|group| group.programs.iter().map(Program::offset))
                .collect::<BTreeSet<_>>()
                .len(),
            decoded_programs: self
                .groups
                .iter()
                .flat_map(|group| &group.programs)
                .filter_map(|program| match program {
                    Program::Decoded(sequence) => Some(sequence.offset),
                    _ => None,
                })
                .collect::<BTreeSet<_>>()
                .len(),
            unresolved_programs: self
                .groups
                .iter()
                .flat_map(|group| &group.programs)
                .filter_map(|program| match program {
                    Program::Unresolved { offset, .. } => Some(*offset),
                    _ => None,
                })
                .collect::<BTreeSet<_>>()
                .len(),
            cells: self.cells.len(),
            parts: self.cells.iter().map(|cell| cell.parts.len()).sum(),
        }
    }
}

fn invalid(reason: &str) -> Error {
    Error::Codec(format!("resident animation: {reason}"))
}

fn extent(offset: usize, starts: &BTreeSet<usize>, end: usize) -> usize {
    starts.range((offset + 1)..).next().copied().unwrap_or(end)
}

fn decode_sequence(
    source: &[u8],
    offset: usize,
    end: usize,
) -> Result<(Sequence, BTreeMap<usize, Cell>)> {
    let program = animation::decode(source, offset, end)?;
    program.start(0)?;
    for token in program
        .steps()
        .iter()
        .filter(|token| token.duration >= 0x80)
    {
        program.start(usize::from(token.operand))?;
    }
    let mut cells = BTreeMap::new();
    cell::decode_cells(source, offset, end, &program, &mut cells)?;
    Ok((
        Sequence {
            offset,
            end,
            steps: program.steps().to_vec(),
        },
        cells,
    ))
}

/// Recognize the first-offset-delimited resident table variant, within 1 MiB.
/// Preserve table order, repeated program pointers, cell extents and opaque bits.
pub fn decode(source: &[u8]) -> Result<Document> {
    if source.len() > 1024 * 1024 {
        return Err(invalid("source exceeds 1 MiB"));
    }
    let first = source
        .get(..4)
        .ok_or_else(|| invalid("missing group table"))?;
    let first = u32::from_le_bytes(first.try_into().unwrap()) as usize;
    if first == 0 || !first.is_multiple_of(4) || first / 4 > MAX_PROGRAMS {
        return Err(invalid("invalid group table extent"));
    }
    let table = source
        .get(..first)
        .ok_or_else(|| invalid("truncated group table"))?;
    let offsets: Vec<_> = table
        .as_chunks::<4>()
        .0
        .iter()
        .map(|bytes| u32::from_le_bytes(*bytes) as usize)
        .collect();
    if offsets
        .iter()
        .any(|offset| *offset < first || *offset >= source.len())
    {
        return Err(invalid("group pointer exceeds its payload extent"));
    }
    let starts: BTreeSet<_> = offsets.iter().copied().collect();
    let mut groups = Vec::new();
    let mut cells = BTreeMap::new();
    let mut total = 0;
    for offset in offsets {
        let end = extent(offset, &starts, source.len());
        let bytes = source
            .get(offset..end)
            .ok_or_else(|| invalid("invalid group extent"))?;
        let header = bytes
            .get(..2)
            .ok_or_else(|| invalid("missing program table"))?;
        let first = usize::from(u16::from_le_bytes(header.try_into().unwrap()));
        if first == 0 || !first.is_multiple_of(2) || first / 2 > 256 {
            return Err(invalid("program table exceeds its byte selector"));
        }
        total += first / 2;
        if total > MAX_PROGRAMS {
            return Err(invalid("more than 65536 program indices"));
        }
        let table = bytes
            .get(..first)
            .ok_or_else(|| invalid("truncated program table"))?;
        let offsets: Vec<_> = table
            .as_chunks::<2>()
            .0
            .iter()
            .map(|bytes| offset + usize::from(u16::from_le_bytes(*bytes)))
            .collect();
        if offsets
            .iter()
            .any(|start| *start < offset + first || *start >= end)
        {
            return Err(invalid("program pointer exceeds its group extent"));
        }
        let starts: BTreeSet<_> = offsets.iter().copied().collect();
        let mut programs = Vec::new();
        for offset in offsets {
            let end = extent(offset, &starts, end);
            match decode_sequence(source, offset, end) {
                Ok((sequence, decoded_cells)) => {
                    for (address, cell) in decoded_cells {
                        if cells
                            .get(&address)
                            .is_some_and(|existing| existing != &cell)
                        {
                            return Err(invalid(
                                "aliased cells disagree on their allocation or contents",
                            ));
                        }
                        cells.insert(address, cell);
                    }
                    programs.push(Program::Decoded(sequence));
                }
                Err(error) => programs.push(Program::Unresolved {
                    offset,
                    end,
                    reason: error.to_string(),
                }),
            }
        }
        groups.push(Group { offset, programs });
    }
    Ok(Document {
        schema: SCHEMA.into(),
        groups,
        cells: cells.into_values().collect(),
    })
}

/// Resolve the resident branch at SLUS_004.22@0x8014D878 before initialization.
/// The caller must bind source to the base at 0x801459F0 and supply the same
/// entity's u16 group at +0x2C and byte program at +0x4B. No direction is inferred.
pub fn select(source: &[u8], group_index: u16, program_index: u8) -> Result<Sequence> {
    let document = decode(source)?;
    let group = document
        .groups
        .get(usize::from(group_index))
        .ok_or_else(|| invalid(&format!("selected group {group_index} exceeds its table")))?;
    group
        .programs
        .get(usize::from(program_index))
        .ok_or_else(|| {
            invalid(&format!(
                "selected program {program_index} exceeds group {group_index}'s table"
            ))
        })?
        .sequence()
        .cloned()
}

/// Change existing forward step pairs and cell records. Unchanged program aliases
/// do not override edits; changed aliases must agree on the complete step list.
pub fn encode(source: &[u8], edited: &Document) -> Result<Vec<u8>> {
    let original = decode(source)?;
    if edited.schema != SCHEMA || edited.groups.len() != original.groups.len() {
        return Err(invalid("edit changes the schema or group allocation"));
    }
    let mut edits = crate::aliases::Aliases::new();
    for (group_index, (group, raw)) in edited.groups.iter().zip(&original.groups).enumerate() {
        if group.offset != raw.offset || group.programs.len() != raw.programs.len() {
            return Err(invalid("edit changes a group or program table"));
        }
        for (program_index, (program, raw)) in group.programs.iter().zip(&raw.programs).enumerate()
        {
            if program.offset() != raw.offset() || program.end() != raw.end() {
                return Err(invalid("edit changes a program allocation"));
            }
            let (Program::Decoded(program), Program::Decoded(raw)) = (program, raw) else {
                if program != raw {
                    return Err(invalid(
                        "unresolved programs and their original findings are read-only",
                    ));
                }
                continue;
            };
            if program.steps == raw.steps {
                continue;
            }
            edits.add(
                program.offset,
                program,
                format!("group-{group_index}/program-{program_index}"),
            );
        }
    }
    let edits = edits.finish("resident program")?;
    let mut output = source.to_vec();
    cell::encode_cells(&mut output, &original.cells, &edited.cells)?;
    for program in edits.values() {
        let encoded = animation::encode_steps(source, program.offset, program.end, &program.steps)?;
        let start = program.offset + 2;
        let end = start + program.steps.len() * 2;
        output[start..end].copy_from_slice(&encoded[start..end]);
    }
    for program in original.groups.iter().flat_map(|group| &group.programs) {
        match program {
            Program::Decoded(sequence) => animation::decode(source, sequence.offset, sequence.end)?
                .validate_preserved(source, &output)?,
            Program::Unresolved { offset, end, .. } => {
                if source[*offset..*end] != output[*offset..*end] {
                    return Err(invalid(
                        "edit overlaps an unresolved program's immutable source extent",
                    ));
                }
            }
        }
    }
    decode(&output)?;
    Ok(output)
}
