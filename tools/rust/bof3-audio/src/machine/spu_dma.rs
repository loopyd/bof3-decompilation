//! DMA4 request-mode register and block progress. Transfer/bus timing is separate.

use crate::Result;

#[derive(Clone, Debug, Default)]
pub struct Channel {
    address: u32,
    block: u32,
    control: u32,
    words_in_slice: u32,
    blocks_left: u32,
    current: u32,
}

impl Channel {
    pub fn active(&self) -> bool {
        self.control & (1 << 24) != 0
    }
    pub fn from_ram(&self) -> bool {
        self.control & 1 != 0
    }
    pub fn read(&self, offset: u32) -> Result<u32> {
        match offset {
            0 => Ok(self.address),
            4 => Ok(self.block),
            8 => Ok(self.control),
            _ => Err("DMA4: unsupported register offset".into()),
        }
    }
    pub fn write(&mut self, offset: u32, value: u32) -> Result<()> {
        match offset {
            0 | 4 if self.active() => {
                return Err(
                    "DMA4: changing address/count during active transfer is unsupported".into(),
                )
            }
            0 => self.address = value & 0x00ff_ffff,
            4 => self.block = value,
            8 => {
                if value & (1 << 24) != 0 {
                    if self.active() {
                        return Err("DMA4: restarting active transfer is unsupported".into());
                    }
                    if value & !0x1100_0203 != 0 || value & 0x600 != 0x200 {
                        return Err("DMA4: only request-mode transfers without chopping/pause/snooping are implemented".into());
                    }
                    if !(1..=16).contains(&(self.block & 0xffff)) {
                        return Err("DMA4: SPU block size must be 1..=16 words".into());
                    }
                    self.blocks_left = if self.block >> 16 == 0 {
                        65536
                    } else {
                        self.block >> 16
                    };
                    self.current = self.address;
                } else {
                    self.blocks_left = 0;
                }
                self.words_in_slice = 0;
                self.control = value;
            }
            _ => return Err("DMA4: unsupported register offset".into()),
        }
        Ok(())
    }

    /// The device request is sampled only at slice boundaries. A started slice
    /// continues after DREQ drops, subject to FIFO capacity at the interconnect.
    pub fn next_word(&mut self, requested: bool) -> Option<u32> {
        if !self.active() {
            return None;
        }
        if self.words_in_slice == 0 {
            if !requested && self.control & (1 << 28) == 0 {
                return None;
            }
            self.control &= !(1 << 28);
            self.words_in_slice = self.block & 0xffff;
        }
        Some(self.current & !3)
    }

    /// Returns whether a slice ended. Completion clears busy; BCR retains BS.
    pub fn finish_word(&mut self) -> Result<bool> {
        if self.words_in_slice == 0 {
            return Err("DMA4: no word pending".into());
        }
        self.current = if self.control & 2 == 0 {
            self.current.wrapping_add(4)
        } else {
            self.current.wrapping_sub(4)
        } & 0x00ff_ffff;
        self.words_in_slice -= 1;
        if self.words_in_slice != 0 {
            return Ok(false);
        }
        self.address = self.current;
        self.blocks_left -= 1;
        self.block = (self.block & 0xffff) | ((self.blocks_left & 0xffff) << 16);
        if self.blocks_left == 0 {
            self.control &= !(1 << 24);
        }
        Ok(true)
    }
}
