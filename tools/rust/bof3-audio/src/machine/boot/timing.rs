//! PCSX-Redux interpreter costs and NTSC VBlank phase for original-ROM evidence.
//! Root counters share the interpreter cycle and native update frontier.
//! GPU command processing and bus arbitration remain separate.
use super::super::{
    bus::{Bus, BusError, Width},
    cpu::{Cpu, Fault, Step},
    interconnect::Interconnect,
};

#[derive(Default)]
pub(crate) struct Clock {
    cycles: u64,
    muldiv_ready: u64,
    branch_boundary: bool,
}

impl Clock {
    pub(crate) fn cycles(&self) -> u64 {
        self.cycles
    }

    pub(crate) fn admit_interrupt(&self, cpu: &mut Cpu, bus: &Interconnect) -> bool {
        self.branch_boundary && cpu.take_interrupt(bus.interrupts().pending())
    }

    pub(crate) fn step(&mut self, cpu: &mut Cpu, bus: &mut Interconnect) -> Result<Step, Fault> {
        let pc = cpu.pc();
        let registers: [u32; 32] = std::array::from_fn(|i| cpu.register(i));
        let mut counted = Counted {
            bus,
            accesses: 0,
            instruction: None,
            access_cycle: self.cycles,
        };
        let result = cpu.step(&mut counted);
        if let Some(word) = counted.instruction {
            self.cycles += 2 + u64::from(pc.wrapping_add(4) & 0xffc0_0000 == 0xbfc0_0000) * 20;
            if word >> 26 == 0 {
                match word & 63 {
                    0x10 | 0x12 => self.cycles = self.cycles.max(self.muldiv_ready),
                    0x11 | 0x13 => self.muldiv_ready = 0,
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
                        self.muldiv_ready = self.cycles + latency + 2;
                    }
                    0x1a | 0x1b => self.muldiv_ready = self.cycles + 38,
                    _ => (),
                }
            }
            self.cycles += counted.accesses;
        }
        self.branch_boundary = result.as_ref().is_ok_and(|step| step.in_delay_slot);
        counted
            .bus
            .advance_reference_time(self.cycles, self.branch_boundary);
        result
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
