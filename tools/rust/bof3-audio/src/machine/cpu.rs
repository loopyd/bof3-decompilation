//! R3000 integer execution with branch/load delay state. This core deliberately
//! does not claim cycle timing or full BIOS/GTE/device support. Unsupported
//! execution stops with the exact instruction and delay-slot context.
//!
//! ISA evidence: https://psx-spx.consoledev.net/cpuspecifications/

use super::bus::{Bus, BusError, Width};
use super::cop0::Cop0;
use std::fmt;

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum FaultKind {
    Bus(BusError),
    Alignment { address: u32, store: bool },
    Overflow,
    Syscall,
    Break,
    UnsupportedInstruction,
    UnsupportedSystemControl(&'static str),
    BranchInDelaySlot,
    InstructionLimit(u64),
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Fault {
    pub pc: u32,
    pub instruction: Option<u32>,
    pub in_delay_slot: bool,
    pub kind: FaultKind,
}

impl fmt::Display for Fault {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "R3000 at 0x{:08X} (instruction {:?}, delay slot {}): {:?}",
            self.pc,
            self.instruction.map(|word| format!("0x{word:08X}")),
            self.in_delay_slot,
            self.kind
        )
    }
}
impl std::error::Error for Fault {}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct RegisterWrite {
    pub register: usize,
    pub value: u32,
}

#[derive(Clone, Debug)]
pub struct Step {
    pub pc: u32,
    pub instruction: u32,
    pub in_delay_slot: bool,
    pub committed_load: Option<RegisterWrite>,
    pub register_write: Option<RegisterWrite>,
    pub scheduled_load: Option<RegisterWrite>,
    pub branch_target: Option<u32>,
}

#[derive(Default)]
struct Effects {
    write: Option<RegisterWrite>,
    load: Option<RegisterWrite>,
    branch: bool,
    target: Option<u32>,
    hi: Option<u32>,
    lo: Option<u32>,
    cop0: Option<Cop0>,
}

#[derive(Clone, Debug)]
pub struct Cpu {
    registers: [u32; 32],
    hi: u32,
    lo: u32,
    pc: u32,
    next_pc: u32,
    delay_slot: bool,
    branch_taken: bool,
    cop0: Cop0,
    load: Option<RegisterWrite>,
    instructions: u64,
}

impl Cpu {
    /// CPU register seed from the pinned PCSX-Redux reset implementation.
    /// This does not initialize device timing, cache contents or a BIOS kernel.
    pub fn pcsx_redux_reset() -> Self {
        Self {
            cop0: Cop0::pcsx_redux_reset(),
            ..Self::new(0xbfc0_0000)
        }
    }

    pub fn new(pc: u32) -> Self {
        Self {
            registers: [0; 32],
            hi: 0,
            lo: 0,
            pc,
            next_pc: pc.wrapping_add(4),
            delay_slot: false,
            branch_taken: false,
            cop0: Cop0::default(),
            load: None,
            instructions: 0,
        }
    }
    pub fn pc(&self) -> u32 {
        self.pc
    }
    pub fn register(&self, register: usize) -> u32 {
        self.registers[register]
    }
    pub fn set_register(&mut self, register: usize, value: u32) {
        if register != 0 {
            self.registers[register] = value;
        }
    }
    pub fn hi(&self) -> u32 {
        self.hi
    }
    pub fn lo(&self) -> u32 {
        self.lo
    }
    pub fn instructions(&self) -> u64 {
        self.instructions
    }
    pub fn cop0(&self) -> &Cop0 {
        &self.cop0
    }
    pub fn cop0_mut(&mut self) -> &mut Cop0 {
        &mut self.cop0
    }

    /// Bounded BIOS ReturnFromException restoration after all guest reads pass.
    pub(crate) fn restore_kernel_context(
        &mut self,
        registers: &[u32; 32],
        hi: u32,
        lo: u32,
        status: u32,
        pc: u32,
    ) -> Result<(), &'static str> {
        self.synchronize_load();
        self.registers = *registers;
        self.registers[0] = 0;
        self.registers[26] = pc; // BIOS uses K0 as the return target, not saved K0.
        self.hi = hi;
        self.lo = lo;
        self.cop0.write(12, status)?;
        self.cop0.return_from_exception();
        self.resume_at(pc);
        Ok(())
    }

    /// Explicit architectural exception delivery; unsupported machine behavior
    /// stays a host error rather than becoming an emulated reserved instruction.
    pub fn enter_exception(&mut self, fault: &Fault) -> Result<(), &'static str> {
        if fault.pc != self.pc || fault.in_delay_slot != self.delay_slot {
            return Err("exception does not describe the current CPU position");
        }
        let (code, address) = match fault.kind {
            FaultKind::Syscall => (8, None),
            FaultKind::Break => (9, None),
            FaultKind::Overflow => (12, None),
            FaultKind::Alignment { address, store } => (if store { 5 } else { 4 }, Some(address)),
            _ => return Err("fault has no supported architectural exception delivery"),
        };
        self.raise_exception(code, address);
        Ok(())
    }

    /// The execution scheduler must call this at an instruction boundary.
    /// No CPU-cycle or device-time advancement is implied.
    pub fn take_interrupt(&mut self, external: bool) -> bool {
        self.cop0.set_external_interrupt(external);
        if !self.cop0.interrupt_pending() {
            return false;
        }
        self.raise_exception(0, None);
        true
    }

    fn raise_exception(&mut self, code: u32, bad_address: Option<u32>) {
        let vector = self.cop0.enter(
            code,
            self.pc,
            self.delay_slot,
            self.branch_taken,
            self.next_pc,
            bad_address,
        );
        self.resume_at(vector);
    }

    /// Finish a pending load before a modeled BIOS service consumes caller state.
    pub fn synchronize_load(&mut self) {
        if let Some(load) = self.load.take() {
            self.set_register(load.register, load.value);
        }
    }

    /// Kernel-controlled transfer. Game jumps must execute their instructions.
    pub fn resume_at(&mut self, pc: u32) {
        self.synchronize_load();
        self.pc = pc;
        self.next_pc = pc.wrapping_add(4);
        self.delay_slot = false;
        self.branch_taken = false;
    }

    fn fault(&self, instruction: Option<u32>, kind: FaultKind) -> Fault {
        Fault {
            pc: self.pc,
            instruction,
            in_delay_slot: self.delay_slot,
            kind,
        }
    }

    pub fn step(&mut self, bus: &mut impl Bus) -> Result<Step, Fault> {
        if self.cop0.status() & 2 != 0 {
            return Err(self.fault(
                None,
                FaultKind::UnsupportedSystemControl("user mode execution"),
            ));
        }
        bus.set_cache_isolation(self.cop0.status() & (1 << 16) != 0)
            .map_err(|detail| self.fault(None, FaultKind::UnsupportedSystemControl(detail)))?;
        let result = self.step_body(bus);
        // Isolation applies to this CPU transaction, not to host/DMA reads.
        let clear = bus.set_cache_isolation(false);
        result.and_then(|step| {
            clear
                .map(|()| step)
                .map_err(|detail| self.fault(None, FaultKind::UnsupportedSystemControl(detail)))
        })
    }

    fn step_body(&mut self, bus: &mut impl Bus) -> Result<Step, Fault> {
        let instruction = self.fetch(bus)?;
        let result = self.execute(bus, instruction).and_then(|effects| {
            if self.delay_slot && effects.branch {
                Err(FaultKind::BranchInDelaySlot)
            } else {
                Ok(effects)
            }
        });
        let effects = match result {
            Ok(effects) => effects,
            Err(kind) => {
                // An older load is not undone by a fault in the following instruction.
                if let Some(load) = self.load.take() {
                    self.set_register(load.register, load.value);
                }
                return Err(self.fault(Some(instruction), kind));
            }
        };
        let committed_load = self.load.take().filter(|old| {
            old.register != 0
                && effects
                    .write
                    .is_none_or(|write| write.register != old.register)
                && effects
                    .load
                    .is_none_or(|load| load.register != old.register)
        });
        if let Some(write) = committed_load {
            self.set_register(write.register, write.value);
        }
        if let Some(write) = effects.write {
            self.set_register(write.register, write.value);
        }
        if let Some(hi) = effects.hi {
            self.hi = hi;
        }
        if let Some(lo) = effects.lo {
            self.lo = lo;
        }
        if let Some(cop0) = effects.cop0 {
            self.cop0 = cop0;
        }
        let step = Step {
            pc: self.pc,
            instruction,
            in_delay_slot: self.delay_slot,
            committed_load,
            register_write: effects.write.filter(|write| write.register != 0),
            scheduled_load: effects.load.filter(|load| load.register != 0),
            branch_target: effects.target,
        };
        self.load = step.scheduled_load;
        self.pc = self.next_pc;
        self.next_pc = effects
            .target
            .unwrap_or_else(|| self.next_pc.wrapping_add(4));
        self.delay_slot = effects.branch;
        self.branch_taken = effects.target.is_some();
        self.instructions += 1;
        Ok(step)
    }

    /// A bound is mandatory; reaching a caller's sentinel is not general runtime acceptance.
    pub fn run_until(
        &mut self,
        bus: &mut impl Bus,
        stop_pc: u32,
        limit: u64,
    ) -> Result<u64, Fault> {
        let start = self.instructions;
        while self.pc != stop_pc {
            if self.instructions - start >= limit {
                return Err(self.fault(None, FaultKind::InstructionLimit(limit)));
            }
            self.step(bus)?;
        }
        Ok(self.instructions - start)
    }

    fn fetch(&mut self, bus: &mut impl Bus) -> Result<u32, Fault> {
        let result = aligned(self.pc, Width::Word, false)
            .and_then(|()| bus.fetch(self.pc).map_err(FaultKind::Bus));
        result.map_err(|kind| {
            if let Some(load) = self.load.take() {
                self.set_register(load.register, load.value);
            }
            self.fault(None, kind)
        })
    }

    fn execute(&self, bus: &mut impl Bus, word: u32) -> Result<Effects, FaultKind> {
        let opcode = word >> 26;
        let rs = ((word >> 21) & 31) as usize;
        let rt = ((word >> 16) & 31) as usize;
        let rd = ((word >> 11) & 31) as usize;
        let shift = (word >> 6) & 31;
        let a = self.registers[rs];
        let b = self.registers[rt];
        let immediate = (word as u16 as i16 as i32) as u32;
        let mut effects = Effects::default();
        let mut write = |register, value| effects.write = Some(RegisterWrite { register, value });
        match opcode {
            0 => match word & 63 {
                0x00 => write(rd, b << shift),
                0x02 => write(rd, b >> shift),
                0x03 => write(rd, ((b as i32) >> shift) as u32),
                0x04 => write(rd, b << (a & 31)),
                0x06 => write(rd, b >> (a & 31)),
                0x07 => write(rd, ((b as i32) >> (a & 31)) as u32),
                0x08 | 0x09 => {
                    if word & 63 == 9 {
                        write(rd, self.pc.wrapping_add(8));
                    }
                    effects.branch = true;
                    effects.target = Some(a);
                }
                0x0c => return Err(FaultKind::Syscall),
                0x0d => return Err(FaultKind::Break),
                0x10 => write(rd, self.hi),
                0x11 => effects.hi = Some(a),
                0x12 => write(rd, self.lo),
                0x13 => effects.lo = Some(a),
                0x18 | 0x19 => {
                    let product = if word & 63 == 0x18 {
                        ((a as i32 as i64) * (b as i32 as i64)) as u64
                    } else {
                        u64::from(a) * u64::from(b)
                    };
                    effects.hi = Some((product >> 32) as u32);
                    effects.lo = Some(product as u32);
                }
                0x1a | 0x1b => {
                    let (lo, hi) = divide(a, b, word & 63 == 0x1a);
                    effects.hi = Some(hi);
                    effects.lo = Some(lo);
                }
                0x20 => write(
                    rd,
                    (a as i32)
                        .checked_add(b as i32)
                        .ok_or(FaultKind::Overflow)? as u32,
                ),
                0x21 => write(rd, a.wrapping_add(b)),
                0x22 => write(
                    rd,
                    (a as i32)
                        .checked_sub(b as i32)
                        .ok_or(FaultKind::Overflow)? as u32,
                ),
                0x23 => write(rd, a.wrapping_sub(b)),
                0x24 => write(rd, a & b),
                0x25 => write(rd, a | b),
                0x26 => write(rd, a ^ b),
                0x27 => write(rd, !(a | b)),
                0x2a => write(rd, u32::from((a as i32) < (b as i32))),
                0x2b => write(rd, u32::from(a < b)),
                _ => return Err(FaultKind::UnsupportedInstruction),
            },
            1 => {
                if ![0, 1, 16, 17].contains(&rt) {
                    return Err(FaultKind::UnsupportedInstruction);
                }
                let taken = ((a as i32) < 0) == (rt & 1 == 0);
                if rt & 16 != 0 {
                    write(31, self.pc.wrapping_add(8));
                }
                effects.branch = true;
                if taken {
                    effects.target = Some(self.pc.wrapping_add(4).wrapping_add(immediate << 2));
                }
            }
            2 | 3 => {
                if opcode == 3 {
                    write(31, self.pc.wrapping_add(8));
                }
                effects.branch = true;
                effects.target =
                    Some((self.pc.wrapping_add(4) & 0xf000_0000) | ((word & 0x03ff_ffff) << 2));
            }
            4..=7 => {
                let taken = match opcode {
                    4 => a == b,
                    5 => a != b,
                    6 => (a as i32) <= 0,
                    _ => (a as i32) > 0,
                };
                effects.branch = true;
                if taken {
                    effects.target = Some(self.pc.wrapping_add(4).wrapping_add(immediate << 2));
                }
            }
            8 => write(
                rt,
                (a as i32)
                    .checked_add(immediate as i32)
                    .ok_or(FaultKind::Overflow)? as u32,
            ),
            9 => write(rt, a.wrapping_add(immediate)),
            10 => write(rt, u32::from((a as i32) < (immediate as i32))),
            11 => write(rt, u32::from(a < immediate)),
            12 => write(rt, a & (word & 0xffff)),
            13 => write(rt, a | (word & 0xffff)),
            14 => write(rt, a ^ (word & 0xffff)),
            15 => write(rt, word << 16),
            0x10 => {
                let mut cop0 = self.cop0.clone();
                match rs {
                    0 if word & 0x7ff == 0 => {
                        let value = cop0.read(rd).map_err(FaultKind::UnsupportedSystemControl)?;
                        effects.load = Some(RegisterWrite {
                            register: rt,
                            value,
                        });
                    }
                    4 if word & 0x7ff == 0 => {
                        cop0.write(rd, b)
                            .map_err(FaultKind::UnsupportedSystemControl)?;
                        effects.cop0 = Some(cop0);
                    }
                    16 if word == 0x4200_0010 => {
                        cop0.return_from_exception();
                        effects.cop0 = Some(cop0);
                    }
                    _ => return Err(FaultKind::UnsupportedInstruction),
                }
            }
            0x20..=0x26 => {
                let address = a.wrapping_add(immediate);
                let value = match opcode {
                    0x20 | 0x24 => {
                        let byte = bus.read(address, Width::Byte).map_err(FaultKind::Bus)?;
                        if opcode == 0x20 {
                            byte as u8 as i8 as i32 as u32
                        } else {
                            byte & 0xff
                        }
                    }
                    0x21 | 0x25 => {
                        aligned(address, Width::Half, false)?;
                        let half = bus.read(address, Width::Half).map_err(FaultKind::Bus)?;
                        if opcode == 0x21 {
                            half as u16 as i16 as i32 as u32
                        } else {
                            half & 0xffff
                        }
                    }
                    0x23 => {
                        aligned(address, Width::Word, false)?;
                        bus.read(address, Width::Word).map_err(FaultKind::Bus)?
                    }
                    0x22 | 0x26 => {
                        let memory = bus
                            .read(address & !3, Width::Word)
                            .map_err(FaultKind::Bus)?;
                        let old = self
                            .load
                            .filter(|load| load.register == rt)
                            .map_or(b, |load| load.value);
                        let shift = (address & 3) * 8;
                        if opcode == 0x22 {
                            let shift = 24 - shift;
                            (old & !(u32::MAX << shift)) | (memory << shift)
                        } else {
                            (old & !(u32::MAX >> shift)) | (memory >> shift)
                        }
                    }
                    _ => unreachable!(),
                };
                effects.load = Some(RegisterWrite {
                    register: rt,
                    value,
                });
            }
            0x28 | 0x29 | 0x2b => {
                let width = match opcode {
                    0x28 => Width::Byte,
                    0x29 => Width::Half,
                    _ => Width::Word,
                };
                let address = a.wrapping_add(immediate);
                aligned(address, width, true)?;
                bus.write(address, width, b).map_err(FaultKind::Bus)?;
            }
            0x2a | 0x2e => {
                let address = a.wrapping_add(immediate);
                let lane = address & 3;
                let (value, lanes) = if opcode == 0x2a {
                    (b >> (24 - lane * 8), (1u8 << (lane + 1)) - 1)
                } else {
                    (b << (lane * 8), (15u8 << lane) & 15)
                };
                bus.write_masked(address & !3, value, lanes)
                    .map_err(FaultKind::Bus)?;
            }
            _ => return Err(FaultKind::UnsupportedInstruction),
        }
        Ok(effects)
    }
}

fn aligned(address: u32, width: Width, store: bool) -> Result<(), FaultKind> {
    if address as usize & (width.bytes() - 1) != 0 {
        Err(FaultKind::Alignment { address, store })
    } else {
        Ok(())
    }
}

fn divide(a: u32, b: u32, signed: bool) -> (u32, u32) {
    if b == 0 {
        return (
            if signed && (a as i32) < 0 {
                1
            } else {
                u32::MAX
            },
            a,
        );
    }
    if signed {
        if a == 0x8000_0000 && b == u32::MAX {
            return (a, 0);
        }
        (
            ((a as i32) / (b as i32)) as u32,
            ((a as i32) % (b as i32)) as u32,
        )
    } else {
        (a / b, a % b)
    }
}
