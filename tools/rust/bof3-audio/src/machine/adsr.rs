//! Per-output-frame SPU ADSR state. MMIO/key-on latency and mixing are separate.
//!
//! Callers choose published Q15 arithmetic or the current native denominator
//! arithmetic. Neither model establishes hardware fidelity or bus timing.

use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Model {
    /// Strict >0x6000 threshold; rate floor after exponential adjustment.
    Published,
    /// Current native ADSR periods and phase rules. Volume sweeps separately
    /// retain their inclusive threshold and pre-adjustment rate floor.
    EmulatorReference,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Phase {
    Off,
    Attack,
    Decay,
    Sustain,
    Release,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct Registers {
    pub adsr1: u16,
    pub adsr2: u16,
}

impl Registers {
    pub fn sustain_level(self) -> i16 {
        (u32::from((self.adsr1 & 15) + 1) * 2048).min(32767) as i16
    }

    fn rate(self, phase: Phase) -> Rate {
        let (bits, decreasing, exponential, frozen) = match phase {
            Phase::Attack => {
                let bits = (self.adsr1 >> 8) & 127;
                (bits, false, self.adsr1 & 0x8000 != 0, bits == 127)
            }
            Phase::Decay => ((self.adsr1 >> 2) & 60, true, true, false),
            Phase::Sustain => {
                let bits = (self.adsr2 >> 6) & 127;
                (
                    bits,
                    self.adsr2 & 0x4000 != 0,
                    self.adsr2 & 0x8000 != 0,
                    bits == 127,
                )
            }
            Phase::Release => {
                let shift = self.adsr2 & 31;
                (shift * 4, true, self.adsr2 & 32 != 0, shift == 31)
            }
            Phase::Off => (127, false, false, true),
        };
        Rate {
            shift: (bits / 4) as u8,
            step: (bits % 4) as u8,
            decreasing,
            exponential,
            frozen,
            negative_phase: false,
        }
    }
}

pub(crate) struct Rate {
    pub shift: u8,
    pub step: u8,
    pub decreasing: bool,
    pub exponential: bool,
    pub frozen: bool,
    pub negative_phase: bool,
}

impl Rate {
    pub(crate) fn delta_and_increment(&self, level: i16, model: Model) -> (i32, u32) {
        let negative =
            (self.decreasing ^ self.negative_phase) || (self.decreasing && self.exponential);
        let magnitude = if negative {
            i32::from(self.step) - 8
        } else {
            7 - i32::from(self.step)
        };
        let mut delta = magnitude * (1 << 11u8.saturating_sub(self.shift));
        let mut increment = 0x8000u32 >> self.shift.saturating_sub(11);
        if model == Model::EmulatorReference && !self.frozen {
            increment = increment.max(1);
        }
        if self.exponential {
            if self.decreasing {
                // Arithmetic shift is essential: tiny negative changes round down.
                delta = (delta * i32::from(level)) >> 15;
            } else {
                let slow = match model {
                    Model::Published => level > 0x6000,
                    Model::EmulatorReference => level >= 0x6000,
                };
                if slow {
                    match self.shift {
                        0..=9 => delta >>= 2,
                        10 => {
                            delta >>= 1;
                            increment >>= 1;
                        }
                        _ => increment >>= 2,
                    }
                }
            }
        }
        if model == Model::Published && !self.frozen {
            increment = increment.max(1);
        }
        (delta, increment)
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct Adsr {
    registers: Registers,
    model: Model,
    phase: Phase,
    level: i16,
    counter: u32,
}

impl Adsr {
    pub fn new(registers: Registers, model: Model) -> Self {
        Self {
            registers,
            model,
            phase: Phase::Off,
            level: 0,
            counter: 0,
        }
    }

    pub fn registers(&self) -> Registers {
        self.registers
    }
    pub fn model(&self) -> Model {
        self.model
    }
    pub fn phase(&self) -> Phase {
        self.phase
    }
    pub fn level(&self) -> i16 {
        self.level
    }
    /// Published Q15 accumulator, or native elapsed frames since the last step.
    pub fn counter(&self) -> u32 {
        self.counter
    }

    /// Apply an already scheduled ADSR register write. The native model retains
    /// its elapsed-frame counter. Published arithmetic resets its accumulator.
    pub fn write_registers(&mut self, registers: Registers) {
        self.registers = registers;
        if self.model == Model::Published {
            self.counter = 0;
        }
    }

    /// Inject signed component state. This does not model a native guest ENVX
    /// write, which the current native register handler ignores.
    pub fn write_level(&mut self, raw: u16) {
        self.level = raw as i16;
    }

    pub fn key_on(&mut self) {
        self.level = 0;
        self.counter = 0;
        self.enter(Phase::Attack);
    }

    pub fn key_off(&mut self) {
        if !matches!(self.phase, Phase::Off | Phase::Release) {
            self.enter(Phase::Release);
        }
    }

    pub fn force_off(&mut self) {
        self.level = 0;
        self.counter = 0;
        self.enter(Phase::Off);
    }

    fn enter(&mut self, phase: Phase) {
        self.phase = phase;
        if self.model == Model::Published {
            self.counter = 0;
        }
    }

    /// Consume one frame's gain. Published output uses the preceding level;
    /// native-reference output uses the freshly stepped envelope.
    pub fn sample(&mut self) -> i16 {
        let preceding = self.level;
        let updated = self.tick();
        match self.model {
            Model::Published => preceding,
            Model::EmulatorReference => updated,
        }
    }

    /// Advance one 44.1 kHz envelope clock and return its output level.
    pub fn tick(&mut self) -> i16 {
        if self.model == Model::EmulatorReference {
            return self.tick_native();
        }
        if self.phase == Phase::Off {
            return self.level;
        }
        let rate = self.registers.rate(self.phase);
        let (delta, increment) = rate.delta_and_increment(self.level, self.model);
        self.counter += increment;
        if self.counter & 0x8000 != 0 {
            self.counter = 0;
            let value = i32::from(self.level) + delta;
            self.level = if rate.decreasing {
                value.max(0) as i16
            } else {
                value.clamp(-32768, 32767) as i16
            };
        }
        // Target checks also apply on non-step frames (e.g. a manual ENVX write).
        let next = match self.phase {
            Phase::Attack if self.level == 32767 => Some(Phase::Decay),
            Phase::Decay if self.level <= self.registers.sustain_level() => Some(Phase::Sustain),
            Phase::Release if self.level <= 0 => Some(Phase::Off),
            _ => None,
        };
        if let Some(phase) = next {
            self.enter(phase);
        }
        self.level
    }

    fn tick_native(&mut self) -> i16 {
        if self.phase == Phase::Off {
            return 0;
        }
        let rate = self.registers.rate(self.phase);
        let mut bits = u32::from(rate.shift) * 4 + u32::from(rate.step);
        if rate.exponential && !rate.decreasing && self.level >= 0x6000 {
            bits = (bits + 8).min(127);
        }
        let shift = bits / 4;
        let period = 1u32 << shift.saturating_sub(11);
        self.counter += 1;
        let mut level = i32::from(self.level);
        if self.counter >= period {
            self.counter = 0;
            let magnitude = if rate.decreasing {
                (bits % 4) as i32 - 8
            } else {
                7 - (bits % 4) as i32
            };
            let mut delta = magnitude * (1 << 11u32.saturating_sub(shift));
            if rate.exponential && rate.decreasing {
                delta = (delta * level) >> 15;
            }
            level += delta;
        }
        match self.phase {
            Phase::Attack if level >= 32767 => {
                level = 32767;
                self.enter(Phase::Decay);
            }
            Phase::Decay => {
                level = level.max(0);
                if (level >> 11) & 15 <= i32::from(self.registers.adsr1 & 15) {
                    self.enter(Phase::Sustain);
                }
            }
            Phase::Sustain if rate.decreasing => level = level.max(0),
            Phase::Sustain => level = level.min(32767),
            Phase::Release if level < 0 => {
                level = 0;
                self.enter(Phase::Off);
            }
            _ => (),
        }
        self.level = level as i16;
        self.level
    }
}
