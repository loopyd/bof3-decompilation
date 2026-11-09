//! Native DMA4 registers. Payload copying and scheduled completion have separate owners.
use crate::Result;

#[derive(Clone, Debug, Default)]
pub struct Channel {
    address: u32,
    block: u32,
    control: u32,
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
            0 => self.address = value,
            4 => self.block = value,
            8 => {
                if value & (1 << 24) != 0 && !matches!(value, 0x0100_0200 | 0x0100_0201) {
                    return Err(
                        "DMA4: only native request controls 0x01000200/0x01000201 are implemented"
                            .into(),
                    );
                }
                self.control = value;
            }
            _ => return Err("DMA4: unsupported register offset".into()),
        }
        Ok(())
    }

    pub(crate) fn transfer(&self) -> (u32, u32) {
        (
            self.address & 0x007f_fffc,
            (self.block >> 16) * (self.block & 0xffff),
        )
    }

    pub(crate) fn advance_registers(&mut self) {
        let blocks = if self.block >> 16 == 0 {
            65536
        } else {
            self.block >> 16
        };
        self.address = (self.address & 0x007f_fffc)
            .wrapping_add(blocks.wrapping_mul(self.block & 0xffff).wrapping_mul(4));
        self.block &= 0xffff;
    }

    pub(crate) fn complete(&mut self) {
        self.control &= !(1 << 24);
    }
}
