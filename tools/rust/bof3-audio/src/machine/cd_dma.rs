//! DMA3 burst-to-RAM register state; caller owns bus service and clocks.
use crate::Result;

#[derive(Clone, Debug, Default)]
pub struct Channel {
    address: u32,
    block: u32,
    control: u32,
    current: u32,
    remaining: u32,
    started: bool,
}
impl Channel {
    pub fn active(&self) -> bool {
        self.control & (1 << 24) != 0
    }
    pub fn read(&self, offset: u32) -> Result<u32> {
        match offset {
            0 => Ok(self.address),
            4 => Ok(self.block),
            8 => Ok(self.control),
            _ => Err("DMA3: unsupported register offset".into()),
        }
    }
    pub fn write(&mut self, offset: u32, value: u32) -> Result<()> {
        match offset {
            0 | 4 if self.active() => {
                return Err("DMA3: changing address/count during transfer is unsupported".into())
            }
            0 => self.address = value & 0x00ff_ffff,
            4 => self.block = value,
            8 => {
                if value & (1 << 24) != 0 {
                    if self.active() {
                        return Err("DMA3: restarting active transfer is unsupported".into());
                    }
                    if value & !0x1100_0002 != 0 {
                        return Err("DMA3: only burst reads to RAM without chopping/pause/snooping are implemented".into());
                    }
                    self.remaining = match self.block & 0xffff {
                        0 => 65536,
                        n => n,
                    };
                    self.current = self.address;
                } else {
                    self.remaining = 0;
                }
                self.started = false;
                self.control = value;
            }
            _ => return Err("DMA3: unsupported register offset".into()),
        }
        Ok(())
    }
    pub fn next_word(&mut self, requested: bool) -> Option<u32> {
        if !self.active() {
            return None;
        }
        if !self.started {
            if !requested && self.control & (1 << 28) == 0 {
                return None;
            }
            self.started = true;
            self.control &= !(1 << 28);
        }
        Some(self.current & !3)
    }
    pub fn finish_word(&mut self) -> Result<bool> {
        if !self.active() || !self.started || self.remaining == 0 {
            return Err("DMA3: no word pending".into());
        }
        self.current = if self.control & 2 == 0 {
            self.current.wrapping_add(4)
        } else {
            self.current.wrapping_sub(4)
        } & 0x00ff_ffff;
        self.remaining -= 1;
        if self.remaining == 0 {
            self.control &= !(1 << 24);
        }
        // Non-chopped burst mode preserves both visible MADR and BCR.
        Ok(self.remaining == 0)
    }
}
