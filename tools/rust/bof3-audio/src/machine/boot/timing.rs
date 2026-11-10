//! PCSX-Redux interpreter instruction costs shared by owned and original code.
//! Original-ROM calls also dispatch root counters at the native update frontier.
//! Host service latency, GPU commands and bus arbitration remain separate.
use super::super::{
    bus::{Bus, BusError, Width},
    cpu::{Cpu, Fault, MemoryRequest, PreparedStep, Step},
    interconnect::Interconnect,
};

#[derive(Default)]
pub(crate) struct Clock {
    cycles: u64,
    muldiv_ready: u64,
    branch_boundary: bool,
}

/// A fetched instruction and its clock charges awaiting one completion.
/// The owner must preserve CPU and clock state until it consumes this handle.
pub(crate) struct PreparedInstruction {
    step: PreparedStep,
    cycles: u64,
    muldiv_ready: u64,
    access_cycle: u64,
}

impl PreparedInstruction {
    pub(crate) fn request(&self) -> Option<&MemoryRequest> {
        self.step.request()
    }

    pub(crate) fn request_cycle(&self) -> Option<u64> {
        self.request().map(|_| self.access_cycle + 1)
    }

    pub(crate) fn isolated(&self) -> bool {
        self.step.isolated()
    }
}

impl Clock {
    pub(crate) fn cycles(&self) -> u64 {
        self.cycles
    }

    pub(crate) fn admit_interrupt(&self, cpu: &mut Cpu, bus: &Interconnect) -> bool {
        self.branch_boundary && cpu.take_interrupt(bus.interrupts().pending())
    }

    pub(crate) fn step(&mut self, cpu: &mut Cpu, bus: &mut Interconnect) -> Result<Step, Fault> {
        let result = self.step_instruction(cpu, bus);
        bus.advance_reference_time(self.cycles, self.branch_boundary);
        result
    }

    pub(crate) fn step_instruction(
        &mut self,
        cpu: &mut Cpu,
        bus: &mut Interconnect,
    ) -> Result<Step, Fault> {
        let prepared = self.prepare_instruction(cpu, bus)?;
        self.complete_instruction(prepared, cpu, bus)
    }

    /// Fetch once and retain all instruction costs before a possible bus wait.
    /// Successful preparation leaves clock and architectural retirement pending.
    pub(crate) fn prepare_instruction(
        &mut self,
        cpu: &mut Cpu,
        bus: &mut Interconnect,
    ) -> Result<PreparedInstruction, Fault> {
        let pc = cpu.pc();
        let registers: [u32; 32] = std::array::from_fn(|i| cpu.register(i));
        let mut counted = Counted {
            bus,
            accesses: 0,
            instruction: None,
            access_cycle: self.cycles,
        };
        match cpu.prepare_step(&mut counted) {
            Ok(step) => {
                let (cycles, muldiv_ready) =
                    self.instruction_cost(step.pc(), step.instruction(), &registers);
                Ok(PreparedInstruction {
                    step,
                    cycles,
                    muldiv_ready,
                    access_cycle: counted.access_cycle,
                })
            }
            Err(fault) => {
                if let Some(word) = counted.instruction {
                    (self.cycles, self.muldiv_ready) = self.instruction_cost(pc, word, &registers);
                    self.cycles += counted.accesses;
                }
                self.branch_boundary = false;
                Err(fault)
            }
        }
    }

    /// Perform the retained data access once, then charge and retire once.
    /// An actual bus fault still charges its attempted access.
    pub(crate) fn complete_instruction(
        &mut self,
        prepared: PreparedInstruction,
        cpu: &mut Cpu,
        bus: &mut Interconnect,
    ) -> Result<Step, Fault> {
        let mut counted = Counted {
            bus,
            accesses: 0,
            instruction: None,
            access_cycle: prepared.access_cycle,
        };
        let result = cpu.complete_step(prepared.step, &mut counted);
        self.cycles = prepared.cycles + counted.accesses;
        self.muldiv_ready = prepared.muldiv_ready;
        self.branch_boundary = result.as_ref().is_ok_and(|step| step.in_delay_slot);
        result
    }

    fn instruction_cost(&self, pc: u32, word: u32, registers: &[u32; 32]) -> (u64, u64) {
        let mut cycles =
            self.cycles + 2 + u64::from(pc.wrapping_add(4) & 0xffc0_0000 == 0xbfc0_0000) * 20;
        let mut muldiv_ready = self.muldiv_ready;
        if word >> 26 == 0 {
            match word & 63 {
                0x10 | 0x12 => cycles = cycles.max(muldiv_ready),
                0x11 | 0x13 => muldiv_ready = 0,
                0x18 | 0x19 => {
                    let mut rs = registers[((word >> 21) & 31) as usize];
                    if word & 63 == 0x18 && (rs as i32) < 0 {
                        rs = !rs;
                    }
                    let latency = if rs < 0x800 {
                        6
                    } else if rs < 0x10_0000 {
                        9
                    } else {
                        13
                    };
                    muldiv_ready = cycles + latency + 2;
                }
                0x1a | 0x1b => muldiv_ready = cycles + 38,
                _ => (),
            }
        }
        (cycles, muldiv_ready)
    }

    pub(crate) fn functional_service(&mut self) {
        self.cycles += 2;
        self.branch_boundary = false;
    }
}

struct Counted<'a> {
    bus: &'a mut Interconnect,
    accesses: u64,
    instruction: Option<u32>,
    access_cycle: u64,
}

impl Bus for Counted<'_> {
    fn set_cache_isolation(&mut self, isolated: bool) -> Result<(), &'static str> {
        self.bus.set_cache_isolation(isolated)
    }
    fn fetch(&mut self, address: u32) -> Result<u32, BusError> {
        let word = self.bus.fetch(address)?;
        self.instruction = Some(word);
        self.access_cycle +=
            2 + u64::from(address.wrapping_add(4) & 0xffc0_0000 == 0xbfc0_0000) * 20;
        Ok(word)
    }
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        self.access(|bus| bus.read(address, width))
    }
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        self.access(|bus| bus.write(address, width, value))
    }
    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
        self.access(|bus| bus.write_masked(address, value, lanes))
    }
}

impl Counted<'_> {
    fn access<T>(
        &mut self,
        operation: impl FnOnce(&mut Interconnect) -> Result<T, BusError>,
    ) -> Result<T, BusError> {
        self.accesses += 1;
        self.bus
            .set_reference_access(Some(self.access_cycle + self.accesses));
        let result = operation(self.bus);
        self.bus.set_reference_access(None);
        result
    }
}
