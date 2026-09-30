//! Scheduled fixed/swept SPU gain. Register bus latency remains caller-owned.
use super::adsr::{Model, Rate};
use crate::Result;

#[derive(Clone, Copy, Debug, Default)]
pub struct Volume {
    register: u16,
    level: i16,
    counter: u32,
    active: bool,
    model: Option<Model>,
}

impl Volume {
    pub fn new(model: Model) -> Self {
        Self {
            model: Some(model),
            ..Self::default()
        }
    }
    pub fn configure(&mut self, model: Model) -> Result<()> {
        if self.model.is_some() {
            return Err("SPU volume arithmetic already configured".into());
        }
        self.model = Some(model);
        Ok(())
    }
    pub fn register(&self) -> u16 {
        self.register
    }
    pub fn level(&self) -> i16 {
        self.level
    }
    pub fn counter(&self) -> u32 {
        self.counter
    }
    pub fn active(&self) -> bool {
        self.active
    }

    /// This is the effective register write, after any device-side delay.
    /// Sweep writes retain current gain and restart the rate counter.
    pub fn write(&mut self, raw: u16) -> Result<()> {
        if raw & 0x8000 != 0 && self.model.is_none() {
            return Err("SPU volume sweeps require an explicit envelope arithmetic model".into());
        }
        self.register = raw;
        self.counter = 0;
        self.active = raw & 0x8000 != 0 && raw & 127 != 127;
        if raw & 0x8000 == 0 {
            self.level = raw.wrapping_shl(1) as i16;
        }
        Ok(())
    }
    /// Direct current-volume writes do not rearm a stopped sweep.
    pub fn write_level(&mut self, raw: u16) {
        self.level = raw as i16;
    }

    /// One envelope clock, after the caller has used the current gain.
    pub fn tick(&mut self) {
        if !self.active {
            return;
        }
        let decreasing = self.register & 0x2000 != 0;
        let negative_phase = self.register & 0x1000 != 0;
        let rate = Rate {
            shift: ((self.register >> 2) & 31) as u8,
            step: (self.register & 3) as u8,
            decreasing,
            exponential: self.register & 0x4000 != 0,
            frozen: false,
            negative_phase,
        };
        let (delta, increment) = rate.delta_and_increment(self.level, self.model.unwrap());
        self.counter += increment;
        if self.counter & 0x8000 == 0 {
            return;
        }
        self.counter = 0;
        let value = i32::from(self.level) + delta;
        if decreasing {
            self.level = if negative_phase {
                value.clamp(-32768, 0)
            } else {
                value.max(0)
            } as i16;
            self.active = self.level != 0;
        } else {
            self.level = value.clamp(-32768, 32767) as i16;
            self.active = self.level != if delta < 0 { -32768 } else { 32767 };
        }
    }
}
