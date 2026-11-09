//! Explicit frozen adjacent data for isolated original-instruction evaluation.
use super::Reference;
use crate::{digest::sha256_hex, machine::executable::Executable, Result};
use serde::Serialize;
use std::ops::Range;

pub(super) const ADJACENT_RANGE: Range<u32> = 0x8018_4406..0x8018_445c;

#[derive(Clone, Debug, Serialize)]
pub struct RuntimeContext {
    pub ram_sha256: String,
    pub adjacent_data_sha256: String,
    pub pitch_routine_sha256: String,
    pub pitch_table_sha256: &'static str,
    pub scope: &'static str,
}

impl Reference {
    /// The caller identifies the capture's history. Bytes establish only this
    /// frozen context, not initialization, natural reachability or invariance.
    /// Only the verified 86-byte gap is imported; code and tables stay original.
    pub fn from_runtime_ram(exe: &Executable, runtime_ram: &[u8]) -> Result<Self> {
        let mut reference = Self::from_executable(exe)?;
        if runtime_ram.len() != 2 * 1024 * 1024 {
            return Err("tuning context: expected exactly 2 MiB of PSX RAM".into());
        }
        let original = reference.ram.bytes();
        let mut routine_hash = String::new();
        for (address, size) in [
            (reference.profile.pitch_entry, 252),
            (reference.profile.pitch_table, 386),
            (reference.profile.spu_pitch_table, 386),
        ] {
            let start = crate::machine::executable::ram_offset(address, size)?;
            let range = start..start + size;
            if runtime_ram[range.clone()] != original[range.clone()] {
                return Err(format!(
                    "tuning context: original routine/table bytes differ at 0x{address:08X}"
                )
                .into());
            }
            if address == reference.profile.pitch_entry {
                routine_hash = sha256_hex(&runtime_ram[range]);
            }
        }
        let start = crate::machine::executable::ram_offset(ADJACENT_RANGE.start, 86)?;
        let adjacent = &runtime_ram[start..start + 86];
        for (i, &value) in adjacent.iter().enumerate() {
            use crate::machine::bus::{Bus, Width};
            reference.ram.write(
                ADJACENT_RANGE.start + i as u32,
                Width::Byte,
                u32::from(value),
            )?;
        }
        reference.runtime_context = Some(RuntimeContext {
            ram_sha256: sha256_hex(runtime_ram),
            adjacent_data_sha256: sha256_hex(adjacent),
            pitch_routine_sha256: routine_hash,
            pitch_table_sha256: crate::machine::profile::US_PITCH_SHA256,
            scope: "isolated original routine with a frozen supplied runtime gap; capture history, natural reachability, concurrent scheduling and PCM fidelity are unverified",
        });
        Ok(reference)
    }

    pub fn runtime_context(&self) -> Option<&RuntimeContext> {
        self.runtime_context.as_ref()
    }

    pub fn from_input(exe: &Executable, runtime_ram: Option<&[u8]>) -> Result<Self> {
        match runtime_ram {
            Some(ram) => Self::from_runtime_ram(exe, ram),
            None => Self::from_executable(exe),
        }
    }
}
