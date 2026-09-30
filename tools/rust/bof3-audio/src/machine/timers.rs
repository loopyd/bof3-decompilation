//! Root-counter state driven by explicit CPU, dotclock and blanking inputs.
//! CPU/GPU clock integration and sub-clock pulse widths remain separate gates.
//! https://psx-spx.consoledev.net/timers/

use super::interrupts::{Interrupts, Source};
use crate::Result;

#[derive(Clone, Debug)]
struct Counter {
    count: u16,
    target: u16,
    mode: u16,
    fired: bool,
    blank: bool,
    free_after_blank: bool,
    zero_hold: bool,
}

impl Default for Counter {
    fn default() -> Self {
        Self {
            count: 0,
            target: 0,
            mode: 0x400,
            fired: false,
            blank: false,
            free_after_blank: false,
            zero_hold: false,
        }
    }
}

#[derive(Clone, Debug, Default)]
pub struct Timers {
    counters: [Counter; 3],
    divide_eight: u8,
}

impl Timers {
    pub(crate) fn require_scanline_clocks(&self) -> Result<()> {
        if self.counters[0].mode & 0x101 != 0 {
            return Err(format!("audio clock does not implement root-counter HBlank or dot-clock sources (timer 0 mode {:#06x}, timer 1 mode {:#06x})", self.counters[0].mode, self.counters[1].mode).into());
        }
        Ok(())
    }

    pub fn read(&mut self, timer: usize, register: u32) -> Result<u32> {
        let counter = self.counters.get_mut(timer).ok_or("invalid root counter")?;
        Ok(match register {
            0 => u32::from(counter.count),
            4 => {
                let mode = counter.mode;
                counter.mode &= !0x1800;
                u32::from(mode)
            }
            8 => u32::from(counter.target),
            _ => return Err("invalid root counter register".into()),
        })
    }
    pub fn write(
        &mut self,
        timer: usize,
        register: u32,
        value: u32,
        irq: &mut Interrupts,
    ) -> Result<()> {
        let counter = self.counters.get_mut(timer).ok_or("invalid root counter")?;
        match register {
            0 => counter.count = value as u16,
            4 => {
                counter.mode = (value as u16 & 0x3ff) | 0x400;
                counter.count = 0;
                counter.fired = false;
                counter.free_after_blank = false;
                counter.zero_hold = true;
                irq.set_line(source(timer), false);
            }
            8 => counter.target = value as u16,
            _ => return Err("invalid root counter register".into()),
        }
        Ok(())
    }
    pub fn advance_cpu(&mut self, clocks: u32, irq: &mut Interrupts) -> Result<()> {
        for _ in 0..clocks {
            for index in 0..2 {
                if self.counters[index].mode & 0x100 == 0 {
                    self.clock(index, irq)?;
                }
            }
            self.divide_eight = (self.divide_eight + 1) & 7;
            if self.counters[2].mode & 0x200 == 0 || self.divide_eight == 0 {
                self.clock(2, irq)?;
            }
        }
        Ok(())
    }
    pub fn advance_dotclocks(&mut self, clocks: u32, irq: &mut Interrupts) -> Result<()> {
        if self.counters[0].mode & 0x100 != 0 {
            for _ in 0..clocks {
                self.clock(0, irq)?;
            }
        }
        Ok(())
    }
    pub fn set_hblank(&mut self, active: bool, irq: &mut Interrupts) -> Result<()> {
        let rising = active && !self.counters[0].blank;
        self.blank(0, active);
        if rising && self.counters[1].mode & 0x100 != 0 {
            self.clock(1, irq)?;
        }
        Ok(())
    }
    pub fn set_vblank(&mut self, active: bool) {
        self.blank(1, active);
    }

    fn blank(&mut self, index: usize, active: bool) {
        let counter = &mut self.counters[index];
        if active && !counter.blank && counter.mode & 1 != 0 {
            match (counter.mode >> 1) & 3 {
                1 | 2 => {
                    counter.count = 0;
                    counter.zero_hold = true;
                }
                3 => counter.free_after_blank = true,
                _ => {}
            }
        }
        counter.blank = active;
    }
    fn clock(&mut self, index: usize, irq: &mut Interrupts) -> Result<()> {
        let counter = &mut self.counters[index];
        if counter.mode & 1 != 0 {
            let sync = (counter.mode >> 1) & 3;
            let paused = if index == 2 {
                sync == 0 || sync == 3
            } else {
                match sync {
                    0 => counter.blank,
                    1 => false,
                    2 => !counter.blank,
                    _ => !counter.free_after_blank,
                }
            };
            if paused {
                return Ok(());
            }
        }
        if counter.mode & 8 != 0 && counter.target == 0 {
            return Err("root-counter target-zero reset timing requires hardware evidence".into());
        }
        if counter.zero_hold {
            counter.zero_hold = false;
            return Ok(());
        }
        if counter.mode & 8 != 0 && counter.count == counter.target {
            counter.count = 0;
            counter.zero_hold = true;
            return Ok(());
        }
        counter.count = counter.count.wrapping_add(1);
        let target = counter.count == counter.target;
        let overflow = counter.count == 0xffff;
        if target {
            counter.mode |= 1 << 11;
        }
        if overflow {
            counter.mode |= 1 << 12;
        }
        let request = (target && counter.mode & 16 != 0) || (overflow && counter.mode & 32 != 0);
        if request && (!counter.fired || counter.mode & 64 != 0) {
            counter.fired = true;
            if counter.mode & 128 != 0 {
                counter.mode ^= 1 << 10;
                irq.set_line(source(index), counter.mode & (1 << 10) == 0);
            } else {
                // Event-boundary pulse. Its electrical width is not yet cycle-validated.
                irq.set_line(source(index), true);
                irq.set_line(source(index), false);
            }
        }
        Ok(())
    }
}

fn source(index: usize) -> Source {
    [Source::Timer0, Source::Timer1, Source::Timer2][index]
}
