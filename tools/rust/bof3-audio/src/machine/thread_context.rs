//! Guest PCB/TCB exception contexts; BIOS boot allocation and scheduling are separate.
use super::{
    bus::{Bus, Width},
    cpu::Cpu,
    executable::ram_offset,
};
use crate::Result;

const BYTES: usize = 0xc0;
const USED: u32 = 0x4000;

#[derive(Clone, Copy, Debug)]
pub struct Context {
    address: u32,
}

impl Context {
    /// Resolve the live PCB pointer into the guest's allocated TCB table.
    /// No default BIOS allocation, thread count or initial contents are invented.
    pub fn current(bus: &mut impl Bus) -> Result<Self> {
        let pcb = bus.read(0x108, Width::Word)?;
        let pcb_bytes = bus.read(0x10c, Width::Word)?;
        let base = bus.read(0x110, Width::Word)?;
        let bytes = bus.read(0x114, Width::Word)? as usize;
        if pcb == 0
            || base == 0
            || pcb & 3 != 0
            || base & 3 != 0
            || pcb_bytes != 4
            || bytes == 0
            || !bytes.is_multiple_of(BYTES)
        {
            return Err("BIOS thread context: missing or malformed PCB/TCB descriptors".into());
        }
        let pcb_offset = ram_offset(pcb, 4)?;
        let table_offset = ram_offset(base, bytes)?;
        if overlaps(pcb_offset, 4, table_offset, bytes)
            || overlaps(pcb_offset, 4, 0x108, 16)
            || overlaps(table_offset, bytes, 0x108, 16)
        {
            return Err("BIOS thread context: overlapping PCB/TCB metadata".into());
        }
        let address = bus.read(pcb, Width::Word)?;
        if address == 0 || address & 3 != 0 {
            return Err("BIOS thread context: invalid current TCB pointer".into());
        }
        let offset = ram_offset(address, BYTES)?;
        let slot = offset
            .checked_sub(table_offset)
            .filter(|&n| n < bytes && n.is_multiple_of(BYTES))
            .ok_or("BIOS thread context: current TCB is outside the allocated slots")?;
        if slot + BYTES > bytes || bus.read(address, Width::Word)? != USED {
            return Err("BIOS thread context: current TCB is not an allocated used thread".into());
        }
        Ok(Self { address })
    }

    pub fn address(&self) -> u32 {
        self.address
    }

    /// Capture after architectural exception delivery, before any handler runs.
    /// Bus write faults are terminal; this is not an atomic memory transaction.
    pub fn save_exception(&self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<()> {
        if !matches!(cpu.pc(), 0x8000_0080 | 0xbfc0_0180) {
            return Err(
                "BIOS thread context: capture requires an architectural exception vector".into(),
            );
        }
        let epc = cpu.cop0().read(14)?;
        let cause = cpu.cop0().read(13)?;
        if epc & 3 != 0 {
            return Err("BIOS thread context: unaligned saved PC unsupported".into());
        }
        ram_offset(epc, 4)?;
        if cause & 0x7c == 0 && (bus.read(epc, Width::Word)? >> 24) & 0xfe == 0x4a {
            return Err(
                "BIOS thread context: COP2 interrupt PC adjustment requires GTE execution evidence"
                    .into(),
            );
        }
        cpu.synchronize_load();
        for register in 1..32 {
            if register != 26 {
                bus.write(
                    self.address + 8 + register as u32 * 4,
                    Width::Word,
                    cpu.register(register),
                )?;
            }
        }
        for (offset, value) in [
            (0x88, epc),
            (0x8c, cpu.hi()),
            (0x90, cpu.lo()),
            (0x94, cpu.cop0().status()),
            (0x98, cause),
        ] {
            bus.write(self.address + offset, Width::Word, value)?;
        }
        Ok(())
    }

    /// Read the whole saved context before changing CPU state. Cause is retained
    /// by COP0, not reloaded from the diagnostic copy in the TCB.
    pub fn restore(&self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<()> {
        let mut registers = [0; 32];
        for (index, value) in registers.iter_mut().enumerate().skip(1) {
            if index != 26 {
                *value = bus.read(self.address + 8 + index as u32 * 4, Width::Word)?;
            }
        }
        let pc = bus.read(self.address + 0x88, Width::Word)?;
        let hi = bus.read(self.address + 0x8c, Width::Word)?;
        let lo = bus.read(self.address + 0x90, Width::Word)?;
        let status = bus.read(self.address + 0x94, Width::Word)?;
        if pc & 3 != 0 {
            return Err("BIOS thread context: unaligned return PC".into());
        }
        ram_offset(pc, 4)?;
        cpu.restore_kernel_context(&registers, hi, lo, status, pc)?;
        Ok(())
    }
}

fn overlaps(a: usize, size: usize, b: usize, other: usize) -> bool {
    a < b + other && b < a + size
}
