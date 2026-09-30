//! Published psx-spx shared noise timer/LFSR, advanced once per output frame.
//! Initial phase is caller supplied; no original BIOS/power-on state is inferred.
use crate::Result;

#[derive(Clone, Debug)]
pub struct Noise {
    level: u16,
    timer: i32,
}

impl Noise {
    pub fn from_published_state(level: u16, timer: i32) -> Result<Self> {
        if !(0..=0x20000).contains(&timer) {
            return Err("SPU noise timer must be within the published 0..=0x20000 range".into());
        }
        Ok(Self { level, timer })
    }
    pub fn level(&self) -> i16 {
        self.level as i16
    }
    pub fn timer(&self) -> i32 {
        self.timer
    }

    /// Clock contains the six SPUCNT bits 8..13. Return the new noise level;
    /// the voice owner uses the old level for this frame, then clocks once.
    pub fn tick(&mut self, clock: u8) -> Result<i16> {
        if clock > 63 {
            return Err("SPU noise clock exceeds six bits".into());
        }
        self.timer -= 4 + i32::from(clock & 3);
        if self.timer < 0 {
            let parity = (1
                ^ (self.level >> 15)
                ^ (self.level >> 12)
                ^ (self.level >> 11)
                ^ (self.level >> 10))
                & 1;
            self.level = self.level.wrapping_shl(1) | parity;
            let period = 0x20000 >> (clock >> 2);
            self.timer += period;
            if self.timer < 0 {
                self.timer += period;
            }
        }
        Ok(self.level())
    }
}
