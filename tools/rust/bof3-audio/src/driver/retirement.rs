//! Retired instruction and byte-lane evidence, including ROM and kernel code.

use crate::{
    driver::closure,
    machine::{
        cpu::{Cpu, Step},
        executable::{ram_offset, Executable},
        firmware::Image,
    },
    Result,
};
use serde::Serialize;
use std::collections::BTreeMap;

/// Capture before stepping: a delayed load can change the address register when
/// the instruction retires, so post-step registers cannot reconstruct an access.
pub struct Before {
    pc: u32,
    registers: [u32; 32],
    isolated: bool,
}

impl Before {
    pub fn capture(cpu: &Cpu) -> Self {
        Self {
            pc: cpu.pc(),
            registers: std::array::from_fn(|n| cpu.register(n)),
            isolated: cpu.cop0().status() & 0x10000 != 0,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
pub struct Instruction {
    pub pc: u32,
    pub word: u32,
    pub region: &'static str,
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
pub struct Access {
    pub instruction: Instruction,
    pub effective_address: u32,
    pub word_address: u32,
    pub bus_address: u32,
    pub bus_width: u8,
    /// Consumed/written bytes relative to the aligned word. Merged loads still
    /// read the whole bus word; these value lanes do not remove read side effects.
    pub byte_lanes: u8,
    pub operation: &'static str,
    pub cache_isolated: bool,
}

#[derive(Default)]
pub struct Retirement {
    pub instructions: BTreeMap<Instruction, u64>,
    pub memory: BTreeMap<Access, u64>,
}

impl Retirement {
    /// Call only with a successful step, before any next instruction. This uses
    /// the actual fetched word from Step, not a speculative read of backing RAM.
    pub fn observe(&mut self, exe: &Executable, before: &Before, step: &Step) -> Result<()> {
        if before.pc != step.pc {
            return Err("retirement observation has a mismatched pre-step PC".into());
        }
        let location = closure::location(exe, step.pc);
        let region = if let Some(offset) = location.file_offset {
            let original = u32::from_le_bytes(exe.bytes()[offset..offset + 4].try_into()?);
            if step.instruction != original {
                return Err(
                    format!("retired original instruction differs at {:#x}", step.pc).into(),
                );
            }
            "executable"
        } else if Image::offset(step.pc).is_some() {
            "bios_rom"
        } else if ram_offset(step.pc, 4).is_ok() {
            "other_ram"
        } else {
            return Err(format!(
                "retired instruction has an unsupported source {:#x}",
                step.pc
            )
            .into());
        };
        let instruction = Instruction {
            pc: step.pc,
            word: step.instruction,
            region,
        };
        let word = step.instruction;
        let address = before.registers[((word >> 21) & 31) as usize]
            .wrapping_add(word as u16 as i16 as i32 as u32);
        let lane = (address & 3) as u8;
        let access = match word >> 26 {
            0x20 | 0x24 => Some((1 << lane, "read")),
            0x21 | 0x25 => Some((3 << lane, "read")),
            0x23 => Some((15, "read")),
            0x22 => Some(((1 << (lane + 1)) - 1, "read")),
            0x26 => Some((15 & (15 << lane), "read")),
            0x28 => Some((1 << lane, "write")),
            0x29 => Some((3 << lane, "write")),
            0x2b => Some((15, "write")),
            0x2a => Some(((1 << (lane + 1)) - 1, "write")),
            0x2e => Some((15 & (15 << lane), "write")),
            _ => None,
        };
        if let Some((byte_lanes, operation)) = access {
            let opcode = word >> 26;
            let merged = matches!(opcode, 0x22 | 0x26 | 0x2a | 0x2e);
            let bus_width = match opcode {
                0x20 | 0x24 | 0x28 => 1,
                0x21 | 0x25 | 0x29 => 2,
                _ => 4,
            };
            *self
                .memory
                .entry(Access {
                    instruction: instruction.clone(),
                    effective_address: address,
                    word_address: address & !3,
                    bus_address: if merged { address & !3 } else { address },
                    bus_width,
                    byte_lanes,
                    operation,
                    cache_isolated: before.isolated,
                })
                .or_default() += 1;
        }
        *self.instructions.entry(instruction).or_default() += 1;
        Ok(())
    }

    pub fn report(&self) -> serde_json::Value {
        serde_json::json!({
            "schema":"bof3.audio.driver-retirement/v1",
            "instructions":self.instructions.iter().map(|(instruction,count)|serde_json::json!({"instruction":instruction,"count":count})).collect::<Vec<_>>(),
            "memory":self.memory.iter().map(|(access,count)|serde_json::json!({"access":access,"count":count})).collect::<Vec<_>>(),
            "limitations":["Successful CPU steps only; faulting instructions, HLE dispatch, host accesses and DMA transfers are not retired CPU accesses.",
                "Byte lanes describe consumed/stored values; merged loads still read a whole bus word. Cache-isolated accesses must not be treated as ordinary RAM reads/writes.",
                "BIOS identity belongs to the surrounding boot evidence. Observed instruction words and addresses do not establish complete data ownership or pruning safety."]
        })
    }
}
