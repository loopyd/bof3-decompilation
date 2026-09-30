//! DMA global control and interrupt state. Channel transfers are added with their
//! actual devices; an unimplemented transfer register is never treated as RAM.
//! https://psx-spx.consoledev.net/dmachannels/

#[derive(Clone, Debug)]
pub struct Dma {
    priority: u32,
    interrupt_control: u32,
    interrupt_flags: u8,
    pub(crate) spu: super::spu_dma::Channel,
    pub(crate) cd: super::cd_dma::Channel,
}

impl Default for Dma {
    fn default() -> Self {
        Self {
            priority: 0x0765_4321,
            interrupt_control: 0,
            interrupt_flags: 0,
            spu: super::spu_dma::Channel::default(),
            cd: super::cd_dma::Channel::default(),
        }
    }
}

impl Dma {
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
        self.interrupt_control & (1 << 15) != 0
            || (self.interrupt_control & (1 << 23) != 0 && self.interrupt_flags != 0)
    }
    pub fn set_interrupt(&mut self, value: u32) {
        self.interrupt_flags &= !((value >> 24) as u8 & 0x7f);
        self.interrupt_control = value & 0x00ff_807f;
    }
    pub fn complete(&mut self, channel: u8) -> crate::Result<()> {
        if channel >= 7 {
            return Err(format!("invalid DMA channel {channel}").into());
        }
        // Channel/master enables gate flag creation, not an already latched flag.
        let enable = (1 << 23) | (1 << (16 + channel));
        if self.interrupt_control & enable == enable {
            self.interrupt_flags |= 1 << channel;
        }
        Ok(())
    }
}
