//! Explicit shared noise arithmetic and frame convention, selected by state.
//! Initial phase is caller supplied; no original BIOS/power-on state is inferred.
use crate::Result;
use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum State {
    Published { level: u16, timer: i32 },
    EmulatorReference { value: u32, count: u32 },
}

#[derive(Clone, Debug)]
pub struct Noise {
    state: State,
}

impl Noise {
    pub fn new(state: State) -> Result<Self> {
        if let State::Published { timer, .. } = state {
            if !(0..=0x20000).contains(&timer) {
                return Err(
                    "SPU noise timer must be within the published 0..=0x20000 range".into(),
                );
            }
        }
        Ok(Self { state })
    }
    pub fn level(&self) -> i16 {
        match self.state {
            State::Published { level, .. } => level as i16,
            State::EmulatorReference { value, .. } => value as i16,
        }
    }
    pub fn state(&self) -> State {
        self.state
    }

    /// One scheduled frame. Published voices consume the preceding level;
    /// the native reference clocks before consumption. Neither resets state.
    pub fn sample(&mut self, clock: u8) -> Result<i16> {
        let previous = self.level();
        let current = self.tick(clock)?;
        Ok(match self.state {
            State::Published { .. } => previous,
            State::EmulatorReference { .. } => current,
        })
    }

    /// Clock contains the six SPUCNT bits 8..13. Return the new noise level.
    pub fn tick(&mut self, clock: u8) -> Result<i16> {
        if clock > 63 {
            return Err("SPU noise clock exceeds six bits".into());
        }
        match &mut self.state {
            State::Published { level, timer } => {
                *timer -= 4 + i32::from(clock & 3);
                if *timer < 0 {
                    *level = next(u32::from(*level)) as u16;
                    let period = 0x20000 >> (clock >> 2);
                    *timer += period;
                    if *timer < 0 {
                        *timer += period;
                    }
                }
            }
            State::EmulatorReference { value, count } => {
                let fraction = [0, 84, 140, 180][usize::from(clock & 3)];
                *count = count.wrapping_add(0x10000 + fraction);
                if *count & 0xffff >= 210 {
                    *count = count.wrapping_add(0x10000 - fraction);
                }
                let period = (0x8000 >> (clock >> 2)) << 16;
                if *count >= period {
                    *count %= period;
                    *value = next(*value);
                }
            }
        }
        Ok(self.level())
    }
}

fn next(value: u32) -> u32 {
    let parity = (1 ^ (value >> 15) ^ (value >> 12) ^ (value >> 11) ^ (value >> 10)) & 1;
    value.wrapping_shl(1) | parity
}
