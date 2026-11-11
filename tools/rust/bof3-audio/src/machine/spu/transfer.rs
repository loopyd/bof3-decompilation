//! Native immediate SPU RAM ports and DMA payload access.
//! Explicit frame/fetch IRQ comparisons do not establish bus or CPU latency.

use crate::Result;
use serde::{Deserialize, Serialize};

pub const RAM_BYTES: usize = 512 * 1024;

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum IrqModel {
    /// Native comparisons and latch semantics. The execution owner schedules
    /// producer batches and CPU delivery; explicit frame calls use caller time.
    EmulatorReference,
}

#[derive(Clone, Copy, Debug, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum RamControlModel {
    /// Native register storage preserves the latch but does not gate 512 KiB
    /// RAM access. This does not model hardware RAM-control modes.
    EmulatorReference,
}

pub struct Transfer {
    ram: Vec<u8>,
    current_address: usize,
    irq_address: u16,
    control: u16,
    status_latch: u16,
    ram_control: u16,
    ram_control_model: Option<RamControlModel>,
    capture_position: u16,
    capture_irq_position: u16,
    cd_capture_position: u16,
    irq_model: Option<IrqModel>,
    irq_flag: bool,
    irq_request: bool,
}

impl Default for Transfer {
    fn default() -> Self {
        Self {
            ram: vec![0; RAM_BYTES],
            current_address: 0,
            irq_address: 0,
            control: 0,
            status_latch: 0,
            ram_control: 0,
            ram_control_model: None,
            capture_position: 0,
            capture_irq_position: 0,
            cd_capture_position: 0,
            irq_model: None,
            irq_flag: false,
            irq_request: false,
        }
    }
}

impl Transfer {
    pub fn configure_ram_control(&mut self, model: RamControlModel) -> Result<()> {
        if self.ram_control_model.is_some() {
            return Err("SPU RAM-control model already configured".into());
        }
        self.ram_control_model = Some(model);
        Ok(())
    }

    pub fn ram_control_model(&self) -> Option<RamControlModel> {
        self.ram_control_model
    }

    pub(crate) fn require_ram_control(&self) -> Result<()> {
        if self.ram_control != 4 && self.ram_control_model.is_none() {
            return Err(format!("SPU: RAM transfer control {:#06x} unsupported; normal 512 KiB mode requires 0x0004", self.ram_control).into());
        }
        Ok(())
    }

    pub fn configure_irq(&mut self, model: IrqModel) -> Result<()> {
        if self.irq_model.is_some() {
            return Err("SPU IRQ model already configured".into());
        }
        self.irq_model = Some(model);
        Ok(())
    }

    /// Consume a device request separately from the SPUSTAT flag. The CPU
    /// interrupt controller owns its own latch and acknowledgement.
    pub(crate) fn take_irq_request(&mut self) -> bool {
        std::mem::take(&mut self.irq_request)
    }

    pub(crate) fn observe_voice_fetch(&mut self, address: u32, loop_address: Option<u32>) {
        let irq = u32::from(self.irq_address) * 8;
        let block_match = irq > address && irq <= address + 16;
        let loop_match = loop_address.is_some_and(|address| irq <= address && address - irq < 16);
        if block_match || loop_match {
            self.trigger_irq();
        }
    }

    fn trigger_irq(&mut self) {
        if let Some(IrqModel::EmulatorReference) = self.irq_model {
            if self.control & 0x40 != 0 {
                self.control &= !0x40;
                self.irq_flag = true;
                self.irq_request = true;
            }
        }
    }

    pub fn ram(&self) -> &[u8] {
        &self.ram
    }
    pub(crate) fn ram_mut(&mut self) -> &mut [u8] {
        &mut self.ram
    }
    pub fn capture_position(&self) -> u16 {
        self.capture_position
    }
    pub fn cd_capture_position(&self) -> u16 {
        self.cd_capture_position
    }

    /// One scheduled output-frame capture, after voice reads. CD is captured
    /// before SPU input gain, voices 1/3 after ADSR and before panning/mute.
    /// None holds CD RAM and its cursor; voice/IRQ position still advances.
    /// Both positions start at zero here, not a verified boot state.
    pub fn capture_frame(&mut self, cd: Option<[i16; 2]>, voices: [i16; 2]) -> Result<()> {
        self.require_ram_control()
            .map_err(|error| format!("SPU capture: {error}"))?;
        if let Some(cd) = cd {
            for (channel, sample) in cd.into_iter().enumerate() {
                let offset = channel * 1024 + usize::from(self.cd_capture_position);
                self.ram[offset..offset + 2].copy_from_slice(&sample.to_le_bytes());
            }
            self.cd_capture_position = (self.cd_capture_position + 2) & 1023;
        }
        for (channel, sample) in voices.into_iter().enumerate() {
            let offset = (channel + 2) * 1024 + usize::from(self.capture_position);
            self.ram[offset..offset + 2].copy_from_slice(&sample.to_le_bytes());
        }
        for channel in 0..4 {
            let offset = channel * 1024 + usize::from(self.capture_position);
            if usize::from(self.irq_address) * 8 == offset {
                self.trigger_irq();
            }
        }
        self.capture_position = (self.capture_position + 2) & 1023;
        self.capture_irq_position = self.capture_position;
        Ok(())
    }
    pub(crate) fn native_capture_voice(
        &mut self,
        voice: usize,
        start: u16,
        sample: usize,
        value: i16,
    ) -> Result<()> {
        self.require_ram_control()?;
        let region = match voice {
            1 => 2,
            3 => 3,
            _ => return Err("native voice capture requires voice 1 or 3".into()),
        };
        let offset = region * 1024 + (usize::from(start) + sample * 2) % 1024;
        self.ram[offset..offset + 2].copy_from_slice(&value.to_le_bytes());
        Ok(())
    }
    pub(crate) fn native_capture_cd(&mut self, cd: [i16; 2]) -> Result<()> {
        self.require_ram_control()?;
        for (channel, value) in cd.into_iter().enumerate() {
            let offset = channel * 1024 + usize::from(self.cd_capture_position);
            self.ram[offset..offset + 2].copy_from_slice(&value.to_le_bytes());
        }
        self.cd_capture_position = (self.cd_capture_position + 2) & 1023;
        Ok(())
    }
    pub(crate) fn native_capture_advance(&mut self, frames: u16) {
        self.capture_position = (self.capture_position + frames * 2) & 1023;
    }
    pub(crate) fn native_capture_irq(&mut self) {
        for channel in 0..4 {
            let offset = channel * 1024 + usize::from(self.capture_irq_position);
            if usize::from(self.irq_address) * 8 == offset {
                self.trigger_irq();
            }
        }
        self.capture_irq_position = (self.capture_irq_position + 2) & 1023;
    }
    pub fn current_address(&self) -> usize {
        self.current_address
    }
    pub fn status(&self) -> u16 {
        (self.status_latch & !0x083f)
            | (self.control & 0x3f)
            | (u16::from(self.irq_flag) << 6)
            | ((self.capture_position & 512) << 2)
    }

    /// Offsets are relative to 1F801DA0. Unsupported ports never become RAM.
    pub fn read(&mut self, offset: u32) -> Result<u16> {
        match offset {
            4 => Ok(self.irq_address),
            6 => Ok((self.current_address >> 3) as u16),
            10 => Ok(self.control),
            12 => Ok(self.ram_control),
            14 => Ok(self.status()),
            8 => self.read_halfword(),
            _ => Err(format!("SPU: unsupported transfer register offset {offset:#x}").into()),
        }
    }

    pub fn write(&mut self, offset: u32, value: u16) -> Result<()> {
        match offset {
            4 => self.irq_address = value,
            6 => {
                self.current_address = usize::from(value) * 8;
            }
            8 => self.write_halfword(value)?,
            10 => {
                if value & 0x40 != 0 && self.irq_model.is_none() {
                    return Err("SPU IRQ9 requires an explicit address-comparison model".into());
                }
                self.control = value;
                if value & 0x40 == 0 {
                    self.irq_flag = false;
                }
            }
            12 => self.ram_control = value,
            14 => {
                self.status_latch = value & 0xf800;
                self.irq_flag = false;
            }
            _ => {
                return Err(format!("SPU: unsupported transfer register offset {offset:#x}").into())
            }
        }
        Ok(())
    }

    pub(crate) fn read_halfword(&mut self) -> Result<u16> {
        self.require_ram_control()?;
        let value = u16::from_le_bytes(
            self.ram[self.current_address..self.current_address + 2].try_into()?,
        );
        self.advance_address();
        Ok(value)
    }

    pub(crate) fn write_halfword(&mut self, value: u16) -> Result<()> {
        self.require_ram_control()?;
        self.ram[self.current_address..self.current_address + 2]
            .copy_from_slice(&value.to_le_bytes());
        self.advance_address();
        Ok(())
    }

    fn advance_address(&mut self) {
        self.current_address = (self.current_address + 2) & (RAM_BYTES - 1);
    }
}
