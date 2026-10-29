//! PCSX-Redux software GPU status/control for audio register polling.
//! GP0 processing, VRAM, GPU DMA and display output are outside this model.

use crate::Result;

const RESET: u32 = 0x1480_2000;

#[derive(Debug)]
pub struct Status {
    value: u32,
    vblank: bool,
}

impl Default for Status {
    fn default() -> Self {
        Self {
            value: RESET,
            vblank: false,
        }
    }
}

impl Status {
    /// The read FIFO is empty because this model rejects GP0 and GPU DMA.
    /// Redux derives DMA-request bit 25 when DMA direction is FIFO.
    pub fn read(&self) -> u32 {
        self.value
            | if self.value & 0x6000_0000 == 0x2000_0000 {
                0x0200_0000
            } else {
                0
            }
    }

    /// Execute supported GP1 commands. Errors preserve the complete status.
    /// PAL mode is refused because the attached audio clock is NTSC.
    pub fn control(&mut self, command: u32) -> Result<()> {
        match command >> 24 {
            0 => self.value = RESET,
            // No queued GP0 commands exist in this model.
            1 => (),
            2 => self.value &= !0x0100_0000,
            3 => self.value = (self.value & !0x0080_0000) | ((command & 1) << 23),
            4 => self.value = (self.value & !0x6000_0000) | ((command & 3) << 29),
            8 => {
                if command & 8 != 0 {
                    return Err("GPU status model requires NTSC display mode".into());
                }
                let width = ((command >> 6) & 1) | ((command & 3) << 1);
                self.value = (self.value & !0x007f_0000)
                    | (width << 16)
                    | ((command & 4) << 17)
                    | ((command & 0x10) << 17)
                    | ((command & 0x20) << 17);
            }
            _ => {
                return Err(
                    format!("unsupported GPU status control command {command:#010x}").into(),
                )
            }
        }
        Ok(())
    }

    /// Redux's software GPU toggles field bit 31 once per VBlank callback.
    /// The caller supplies frame boundaries; repeated levels do not toggle.
    pub fn set_vblank(&mut self, active: bool) {
        if active && !self.vblank {
            self.value ^= 0x8000_0000;
        }
        self.vblank = active;
    }
}
