//! PCSX-Redux interpreter costs and NTSC VBlank phase for original-ROM evidence.
//! This does not model programmable root counters, GPU commands or bus arbitration.
use super::super::{
    bus::{Bus, BusError, Width},
    cpu::{Cpu, Fault, Step},
    interconnect::Interconnect,
    interrupts::Source,
};

const LINE_CYCLES: u64 = 33_868_800 / (60 * 263);

#[derive(Default)]
pub(crate) struct Clock {
    cycles: u64,
    muldiv_ready: u64,
    line: u64,
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
        if self.branch_boundary && (self.line + 1) * LINE_CYCLES <= self.cycles {
            self.line += 1;
            if self.line % 263 == 243 {
                counted.bus.interrupts_mut().set_line(Source::Vblank, true);
                counted.bus.interrupts_mut().set_line(Source::Vblank, false);
            }
        }
        result
    }
}

struct Counted<'a> {
    bus: &'a mut Interconnect,
    accesses: u64,
    instruction: Option<u32>,
}

impl Bus for Counted<'_> {
    fn set_cache_isolation(&mut self, isolated: bool) -> Result<(), &'static str> {
        self.bus.set_cache_isolation(isolated)
    }
    fn fetch(&mut self, address: u32) -> Result<u32, BusError> {
        let word = self.bus.fetch(address)?;
        self.instruction = Some(word);
        Ok(word)
    }
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        self.accesses += 1;
        self.bus.read(address, width)
    }
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        self.accesses += 1;
        self.bus.write(address, width, value)
    }
    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
        self.accesses += 1;
        self.bus.write_masked(address, value, lanes)
    }
}
