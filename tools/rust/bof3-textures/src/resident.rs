//! Resident program selection at SLUS_004.22@0x8014D878 and forward step edits.

use std::collections::{BTreeMap, BTreeSet};

use serde::{Deserialize, Serialize};

use crate::{Error, Result, animation};

pub const SCHEMA: &str = "bof3.resident-animation/v1";
const MAX_PROGRAMS: usize = 65_536;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub groups: usize,
    pub program_indices: usize,
    pub unique_programs: usize,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Sequence {
    pub offset: usize,
    pub end: usize,
    pub steps: Vec<animation::Step>,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Group {
    pub offset: usize,
    pub programs: Vec<Sequence>,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Document {
    pub schema: String,
    pub groups: Vec<Group>,
}

impl Document {
    pub fn info(&self) -> Info {
        Info {
            groups: self.groups.len(),
            program_indices: self.groups.iter().map(|group| group.programs.len()).sum(),
            unique_programs: self
                .groups
                .iter()
                .flat_map(|group| group.programs.iter().map(|program| program.offset))
                .collect::<BTreeSet<_>>()
                .len(),
        }
    }
}

fn invalid(reason: &str) -> Error {
    Error::Codec(format!("resident animation: {reason}"))
}

fn extent(offset: usize, starts: &BTreeSet<usize>, end: usize) -> usize {
    starts.range((offset + 1)..).next().copied().unwrap_or(end)
}

/// Recognize the first-offset-delimited resident table variant, within 1 MiB.
/// Table order and repeated program pointers are preserved. Cell bytes are opaque.
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
            let program = animation::decode(source, offset, end)?;
            // Initialization and each forward branch fetch once, even for high tokens.
            program.start(0)?;
            for token in program
                .steps()
                .iter()
                .filter(|token| token.duration >= 0x80)
            {
                program.start(usize::from(token.operand))?;
            }
            programs.push(Sequence {
                offset,
                end,
                steps: program.steps().to_vec(),
            });
        }
        groups.push(Group { offset, programs });
    }
    Ok(Document {
        schema: SCHEMA.into(),
        groups,
    })
}

/// Change existing forward step pairs only. Unchanged aliases do not override an
/// edit; changed aliases must agree on the complete program's requested steps.
pub fn encode(source: &[u8], edited: &Document) -> Result<Vec<u8>> {
    let original = decode(source)?;
    if edited.schema != SCHEMA || edited.groups.len() != original.groups.len() {
        return Err(invalid("edit changes the schema or group allocation"));
    }
    let mut edits = BTreeMap::new();
    for (group, raw) in edited.groups.iter().zip(&original.groups) {
        if group.offset != raw.offset || group.programs.len() != raw.programs.len() {
            return Err(invalid("edit changes a group or program table"));
        }
        for (program, raw) in group.programs.iter().zip(&raw.programs) {
            if program.offset != raw.offset || program.end != raw.end {
                return Err(invalid("edit changes a program allocation"));
            }
            if program.steps == raw.steps {
                continue;
            }
            if let Some(previous) = edits.insert(program.offset, program)
                && previous.steps != program.steps
            {
                return Err(invalid(&format!(
                    "conflicting aliases for program at byte {}",
                    program.offset
                )));
            }
        }
    }
    let mut output = source.to_vec();
    for program in edits.values() {
        let encoded = animation::encode_steps(source, program.offset, program.end, &program.steps)?;
        let start = program.offset + 2;
        let end = start + program.steps.len() * 2;
        output[start..end].copy_from_slice(&encoded[start..end]);
    }
    decode(&output)?;
    Ok(output)
}
