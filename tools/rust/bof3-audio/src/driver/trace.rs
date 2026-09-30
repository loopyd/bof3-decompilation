//! Shared read-only original-runtime evidence for development probes.
use crate::{
    driver::closure,
    machine::{
        cpu::Cpu,
        executable::{ram_offset, Executable},
        execution::Execution,
        firmware::Image,
        interconnect::Interconnect,
    },
    Result,
};
use std::collections::BTreeMap;

#[derive(Default)]
pub struct Trace {
    pub pcs: BTreeMap<u32, u64>,
    pub regions: BTreeMap<(String, &'static str), u64>,
    pub services: BTreeMap<(String, u32, u32), u64>,
    pub indirect: BTreeMap<(u32, u32), u64>,
    pub reentries: BTreeMap<(u32, u32), u64>,
    pub memory: BTreeMap<(u32, u32, u8, &'static str), u64>,
    previous: Option<u32>,
}

impl Trace {
    /// Separate host-directed calls so they cannot create inferred re-entry edges.
    pub fn begin_call(&mut self) {
        self.previous = None;
    }

    pub fn observe(&mut self, exe: &Executable, stage: &str, e: &Execution) -> Result<()> {
        self.observe_cpu(exe, stage, &e.cpu, &e.bus)
    }

    /// Observe a supplied pre-execution boundary without changing CPU or bus.
    /// The caller must identify synthetic kernel dispatch and timing separately.
    pub fn observe_cpu(
        &mut self,
        exe: &Executable,
        stage: &str,
        cpu: &Cpu,
        bus: &Interconnect,
    ) -> Result<()> {
        let pc = cpu.pc();
        let location = closure::location(exe, pc);
        let region = if let Some(offset) = location.file_offset {
            if let Some(previous) = self.previous {
                if closure::location(exe, previous).file_offset.is_none() {
                    *self.reentries.entry((previous, pc)).or_default() += 1;
                }
            }
            let ram = ram_offset(pc, 4)?;
            if bus.ram().bytes()[ram..ram + 4] != exe.bytes()[offset..offset + 4] {
                return Err(format!("executed original instruction changed at {pc:#x}").into());
            }
            *self.pcs.entry(pc).or_default() += 1;
            let word = u32::from_le_bytes(exe.bytes()[offset..offset + 4].try_into()?);
            let access = match word >> 26 {
                0x20 | 0x24 => Some((1, "read")),
                0x21 | 0x25 => Some((2, "read")),
                0x23 => Some((4, "read")),
                0x22 | 0x26 => Some((4, "merge read")),
                0x28 => Some((1, "write")),
                0x29 => Some((2, "write")),
                0x2b => Some((4, "write")),
                0x2a | 0x2e => Some((4, "merge read/write")),
                _ => None,
            };
            if let Some((width, kind)) = access {
                let mut address = cpu
                    .register(((word >> 21) & 31) as usize)
                    .wrapping_add(word as u16 as i16 as i32 as u32);
                if kind.starts_with("merge") {
                    address &= !3;
                }
                *self.memory.entry((pc, address, width, kind)).or_default() += 1;
            }
            if word >> 26 == 0 && matches!(word & 63, 8 | 9) {
                let target = cpu.register(((word >> 21) & 31) as usize);
                *self.indirect.entry((pc, target)).or_default() += 1;
            }
            "executable"
        } else if Image::offset(pc).is_some() {
            "bios_rom"
        } else {
            "other_ram"
        };
        self.previous = Some(pc);
        *self.regions.entry((stage.into(), region)).or_default() += 1;
        if let Ok(vector) = ram_offset(pc, 4) {
            if matches!(vector, 0xa0 | 0xb0 | 0xc0) {
                *self
                    .services
                    .entry((stage.into(), vector as u32, cpu.register(9)))
                    .or_default() += 1;
            }
        }
        Ok(())
    }
}
