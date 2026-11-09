//! Scheduled pitch writes and per-frame steps. Native batch timing is separate.
use super::spu_sample::Model;
use crate::Result;

#[derive(Clone, Copy, Debug)]
pub struct Pitch {
    model: Model,
    register: u16,
    step: u32,
    frequency: u32,
    used_frequency: u32,
}

impl Pitch {
    pub fn new(model: Model) -> Self {
        Self {
            model,
            register: 0,
            step: u32::from(model == Model::EmulatorReference),
            frequency: 1,
            used_frequency: 0,
        }
    }

    /// Apply an already scheduled register write, including a repeated value.
    /// Native key-on does not reset this state.
    pub fn write(&mut self, value: u16) {
        self.register = match self.model {
            Model::Published => value,
            Model::EmulatorReference => value.min(0x3fff),
        };
        self.frequency = (44100 * u32::from(self.register) / 4096).max(1);
    }

    pub fn register(&self) -> u16 {
        self.register
    }

    /// Last installed step in 16.16 source-sample units.
    pub fn step(&self) -> u32 {
        self.step
    }

    /// Native programmed frequency, which a nonzero source can replace.
    pub fn frequency(&self) -> Option<u32> {
        (self.model == Model::EmulatorReference).then_some(self.frequency)
    }

    pub(crate) fn install_pending_frequency(&mut self) {
        if self.model == Model::EmulatorReference && self.frequency != self.used_frequency {
            self.used_frequency = self.frequency;
            self.step = (u32::from(self.register) << 4).max(1);
        }
    }

    /// Advance pitch state using a supplied post-envelope modulation source.
    /// Published input must fit its signed 16-bit signal representation. Native
    /// zero or absent input preserves the installed step after pending writes.
    pub fn tick(&mut self, source: Option<i32>) -> Result<u32> {
        match self.model {
            Model::Published => {
                let source = source.map(i16::try_from).transpose()?;
                let pitch = source.map_or(self.register, |level| {
                    let factor = i64::from(level) + 0x8000;
                    ((i64::from(self.register as i16) * factor) >> 15) as u16
                });
                self.step = u32::from(pitch.min(0x4000)) << 4;
            }
            Model::EmulatorReference => {
                self.install_pending_frequency();
                if let Some(source) = source.filter(|&level| level != 0) {
                    let scaled = (32768 + i64::from(source)) * i64::from(self.register) / 32768;
                    let pitch = scaled.clamp(1, 0x3fff) as u32;
                    self.frequency = 44100 * pitch / 4096;
                    self.used_frequency = self.frequency;
                    self.step = (((self.frequency / 10) << 16) / 4410).max(1);
                }
            }
        }
        Ok(self.step)
    }
}
