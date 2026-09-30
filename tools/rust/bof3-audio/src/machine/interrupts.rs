//! Edge-latched PSX interrupt controller (I_STAT/I_MASK).
//! https://psx-spx.consoledev.net/interrupts/

#[derive(Clone, Copy, Debug)]
#[repr(u8)]
pub enum Source {
    Vblank,
    Gpu,
    Cdrom,
    Dma,
    Timer0,
    Timer1,
    Timer2,
    Controller,
    Serial,
    Spu,
    Pio,
}

#[derive(Clone, Debug, Default)]
pub struct Interrupts {
    status: u16,
    mask: u16,
    levels: u16,
}

impl Interrupts {
    pub fn status(&self) -> u16 {
        self.status
    }
    pub fn mask(&self) -> u16 {
        self.mask
    }
    pub fn pending(&self) -> bool {
        self.status & self.mask != 0
    }
    pub fn set_mask(&mut self, value: u32) {
        self.mask = value as u16 & 0x7ff;
    }
    pub fn acknowledge(&mut self, value: u32) {
        self.status &= value as u16;
    }
    pub fn set_line(&mut self, source: Source, asserted: bool) {
        let bit = 1 << source as u8;
        if asserted {
            if self.levels & bit == 0 {
                self.status |= bit;
            }
            self.levels |= bit;
        } else {
            self.levels &= !bit;
        }
    }
}
