//! PCSX-Redux's absolute-cycle NTSC root-counter scheduler.
//! This reproduces reference-emulator arithmetic, not hardware timer timing.
use super::super::{
    gpu_status::Status,
    interrupts::{Interrupts, Source},
};
use crate::Result;

const LINE: u64 = 33_868_800 / (60 * 263);

#[derive(Clone, Copy)]
struct Counter {
    mode: u16,
    target: u16,
    rate: u64,
    origin: u64,
    period: u64,
    to_target: bool,
    irq_fired: bool,
    gate_started: bool,
}

impl Default for Counter {
    fn default() -> Self {
        Self {
            mode: 0,
            target: 0,
            rate: 1,
            origin: 0,
            period: 0xffff,
            to_target: false,
            irq_fired: false,
            gate_started: false,
        }
    }
}

pub(crate) struct Counters {
    counters: [Counter; 4],
    cycle: u64,
    next: u64,
    line: u16,
}

impl Default for Counters {
    fn default() -> Self {
        let mut counters = [Counter::default(); 4];
        counters[3].mode = 8;
        counters[3].target = LINE as u16;
        counters[3].period = LINE;
        counters[3].to_target = true;
        Self {
            counters,
            cycle: 0,
            next: LINE,
            line: 0,
        }
    }
}

impl Counters {
    pub(crate) fn set_cycle(&mut self, cycle: u64) {
        self.cycle = cycle;
    }

    pub(crate) fn due(&self) -> bool {
        self.cycle >= self.next
    }

    fn count(&self, index: usize) -> u64 {
        self.cycle.wrapping_sub(self.counters[index].origin) / self.counters[index].rate
    }

    fn schedule(&mut self) {
        let mut remaining = i64::MAX as u64;
        for counter in &self.counters {
            let delta = counter
                .period
                .wrapping_sub(self.cycle.wrapping_sub(counter.origin));
            if (delta as i64) < 0 {
                remaining = 0;
                break;
            }
            remaining = remaining.min(delta);
        }
        self.next = self.cycle.wrapping_add(remaining);
    }

    fn set_count(&mut self, index: usize, value: u64) {
        let counter = &mut self.counters[index];
        let value = value & 0xffff;
        counter.origin = self
            .cycle
            .wrapping_sub(u64::from((value as u32).wrapping_mul(counter.rate as u32)));
        counter.to_target = value < u64::from(counter.target) && counter.target != 0xffff;
        let ticks = if counter.to_target {
            u32::from(counter.target)
        } else {
            0xffff
        };
        counter.period = u64::from(ticks.wrapping_mul(counter.rate as u32));
    }

    fn rate(&mut self, index: usize, gpu: &Status) {
        let mode = self.counters[index].mode;
        self.counters[index].rate = match index {
            0 if mode & 0x100 != 0 => {
                let width = (gpu.read() >> 17) & 3;
                let divider = match (gpu.read() & 0x10000 != 0, width) {
                    (true, 0 | 1) => 7,
                    (_, 0) => 10,
                    (_, 1) => 8,
                    (_, 2) => 5,
                    _ => 4,
                };
                (LINE / (3413 / divider)).max(1)
            }
            1 if mode & 0x100 != 0 => LINE,
            2 if mode & 1 != 0 && matches!((mode >> 1) & 3, 0 | 3) => 0xffff_ffff,
            2 if mode & 0x200 != 0 => 8,
            _ => 1,
        };
    }

    fn pulse(irq: &mut Interrupts, source: Source) {
        irq.set_line(source, false);
        irq.set_line(source, true);
        irq.set_line(source, false);
    }

    fn event(&mut self, index: usize, irq: &mut Interrupts) {
        let old = self.counters[index];
        let count = self.count(index);
        let (value, flag, request) = if old.to_target {
            (
                if old.mode & 8 != 0 {
                    count.wrapping_sub(u64::from(old.target))
                } else {
                    count & 0xffff
                },
                0x800,
                old.mode & 0x10 != 0,
            )
        } else {
            (count.wrapping_sub(0xffff), 0x1000, old.mode & 0x20 != 0)
        };
        self.set_count(index, value);
        if request && (old.mode & 0x40 != 0 || !old.irq_fired) {
            if index < 3 {
                Self::pulse(irq, [Source::Timer0, Source::Timer1, Source::Timer2][index]);
            }
            self.counters[index].irq_fired = true;
        }
        self.counters[index].mode |= flag;
        if old.mode & 0x80 != 0 {
            self.counters[index].mode ^= 0x400;
        } else {
            self.counters[index].mode |= 0x400;
        }
        self.schedule();
    }

    fn gate(&mut self, index: usize, gpu: &Status) {
        let counter = self.counters[index];
        if counter.mode & 1 == 0 {
            return;
        }
        match (counter.mode >> 1) & 3 {
            1 | 2 => self.set_count(index, 0),
            3 if !counter.gate_started => {
                self.counters[index].gate_started = true;
                self.rate(index, gpu);
                self.set_count(index, 0);
                self.schedule();
            }
            _ => (),
        }
    }

    /// Native update services each counter once, then applies blanking gates.
    /// Reads and writes call it even before the next branch-test deadline.
    pub(crate) fn update(&mut self, irq: &mut Interrupts, gpu: &mut Status) {
        for index in 0..3 {
            let counter = self.counters[index];
            if self.cycle.wrapping_sub(counter.origin) >= counter.period {
                self.event(index, irq);
            }
        }
        let base = self.counters[3];
        if self.cycle.wrapping_sub(base.origin) >= base.period {
            self.event(3, irq);
            self.line += 1;
            self.gate(0, gpu);
            if self.line == 243 {
                Self::pulse(irq, Source::Vblank);
                gpu.set_vblank(true);
                self.gate(1, gpu);
            }
            if self.line >= 263 {
                self.line = 0;
                gpu.set_vblank(false);
            }
        }
    }

    fn validate(index: usize, register: u32) -> Result<()> {
        if index >= 3 || !matches!(register, 0 | 4 | 8) {
            return Err("invalid reference root-counter register".into());
        }
        Ok(())
    }

    pub(crate) fn read(
        &mut self,
        index: usize,
        register: u32,
        irq: &mut Interrupts,
        gpu: &mut Status,
    ) -> Result<u32> {
        Self::validate(index, register)?;
        if register == 8 {
            return Ok(u32::from(self.counters[index].target));
        }
        self.update(irq, gpu);
        if register == 0 {
            let mut count = self.count(index) as u32 & 0xffff;
            if index == 2
                && self.counters[index].to_target
                && self.counters[index].mode & 0x2ff == 0x248
            {
                count /= 2;
            }
            return Ok(count);
        }
        let mode = self.counters[index].mode;
        self.counters[index].mode &= !0x1800;
        Ok(u32::from(mode))
    }

    pub(crate) fn write(
        &mut self,
        index: usize,
        register: u32,
        value: u32,
        irq: &mut Interrupts,
        gpu: &mut Status,
    ) -> Result<()> {
        Self::validate(index, register)?;
        self.update(irq, gpu);
        match register {
            0 => self.set_count(index, u64::from(value)),
            4 => {
                self.counters[index].mode = value as u16 | 0x400;
                self.counters[index].irq_fired = false;
                self.counters[index].gate_started = false;
                self.rate(index, gpu);
                if index < 2 && value & 1 != 0 && matches!((value >> 1) & 3, 2 | 3) {
                    self.counters[index].rate = 0xffff_ffff;
                }
                self.set_count(index, 0);
            }
            8 => {
                self.counters[index].target = value as u16;
                self.set_count(index, self.count(index) & 0xffff);
            }
            _ => unreachable!(),
        }
        self.schedule();
        Ok(())
    }
}
