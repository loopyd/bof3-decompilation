//! DMA global control and interrupt state. Channel transfers are added with their
//! actual devices; an unimplemented transfer register is never treated as RAM.
//! https://psx-spx.consoledev.net/dmachannels/

#[derive(Clone, Debug)]
pub struct Dma {
    priority: u32,
    reference: bool,
    reference_master: bool,
    interrupt_control: u32,
    interrupt_flags: u8,
    pub(crate) spu: super::spu_dma::Channel,
    pub(crate) cd: super::cd_dma::Channel,
}

impl Default for Dma {
    fn default() -> Self {
        Self {
            priority: 0x0765_4321,
            reference: false,
            reference_master: false,
            interrupt_control: 0,
            interrupt_flags: 0,
            spu: super::spu_dma::Channel::default(),
            cd: super::cd_dma::Channel::default(),
        }
    }
}

impl Dma {
    pub(crate) fn configure_reference(&mut self) -> crate::Result<()> {
        if self.cd.active() || self.spu.active() {
            return Err("native DMA requires idle channels at controller attachment".into());
        }
        self.reference_master = self.irq_line();
        self.reference = true;
        Ok(())
    }
    pub(crate) fn reference(&self) -> bool {
        self.reference
    }
    pub fn priority(&self) -> u32 {
        self.priority
    }
    pub fn set_priority(&mut self, value: u32) {
        self.priority = value;
    }
    pub fn bus_error(&mut self) {
        self.interrupt_control |= 1 << 15;
    }
    pub fn interrupt(&self) -> u32 {
        self.interrupt_control
            | (u32::from(self.interrupt_flags) << 24)
            | (u32::from(self.irq_line()) << 31)
    }
    pub fn irq_line(&self) -> bool {
        if self.reference {
            return self.reference_master;
        }
        self.interrupt_control & (1 << 15) != 0
            || (self.interrupt_control & (1 << 23) != 0 && self.interrupt_flags != 0)
    }
    pub fn set_interrupt(&mut self, value: u32) {
        self.interrupt_flags &= !((value >> 24) as u8 & 0x7f);
        if self.reference {
            self.interrupt_control = value & 0x00ff_803f;
            self.reference_master =
                (self.interrupt_control | (u32::from(self.interrupt_flags) << 24)) & 0x7f00_8000
                    != 0
                    && value & 0x0080_8000 != 0;
        } else {
            self.interrupt_control = value & 0x00ff_807f;
        }
    }
    pub fn complete(&mut self, channel: u8) -> crate::Result<()> {
        if channel >= 7 {
            return Err(format!("invalid DMA channel {channel}").into());
        }
        // Channel/master enables gate flag creation, not an already latched flag.
        let enable = (1 << 23) | (1 << (16 + channel));
        if self.interrupt_control & enable == enable {
            self.interrupt_flags |= 1 << channel;
            self.reference_master |= self.reference;
        }
        Ok(())
    }
}
