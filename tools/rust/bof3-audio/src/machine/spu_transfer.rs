//! Normal 512 KiB SPU RAM transfers. Service calls are transactions, not CPU clocks.
//! Voice/reverb fetch timing and IRQ9 comparison are separate work.

use crate::Result;
use std::collections::VecDeque;

pub const RAM_BYTES: usize = 512 * 1024;
pub(crate) const FIFO_HALFWORDS: usize = 32;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Mode {
    Stopped,
    ManualWrite,
    DmaWrite,
    DmaRead,
}

pub struct Transfer {
    ram: Vec<u8>,
    fifo: VecDeque<u16>,
    address_register: u16,
    current_address: usize,
    irq_address: u16,
    control: u16,
    applied_control: u16,
    ram_control: u16,
    capture_position: u16,
}

impl Default for Transfer {
    fn default() -> Self {
        Self {
            ram: vec![0; RAM_BYTES],
            fifo: VecDeque::new(),
            address_register: 0,
            current_address: 0,
            irq_address: 0,
            control: 0,
            applied_control: 0,
            ram_control: 0,
            capture_position: 0,
        }
    }
}

impl Transfer {
    pub fn ram(&self) -> &[u8] {
        &self.ram
    }
    pub(crate) fn ram_mut(&mut self) -> &mut [u8] {
        &mut self.ram
    }
    pub fn capture_position(&self) -> u16 {
        self.capture_position
    }

    /// One scheduled output-frame capture, after voice reads. CD is captured
    /// before SPU input gain, voices 1/3 after ADSR and before panning/mute.
    /// Position starts at zero in this component context, not a verified boot state.
    pub fn capture_frame(&mut self, cd: [i16; 2], voices: [i16; 2]) -> Result<()> {
        if self.ram_control != 4 {
            return Err("SPU capture requires normal 512 KiB RAM control 0x0004".into());
        }
        for (channel, sample) in [cd[0], cd[1], voices[0], voices[1]].into_iter().enumerate() {
            let offset = channel * 1024 + usize::from(self.capture_position);
            self.ram[offset..offset + 2].copy_from_slice(&sample.to_le_bytes());
        }
        self.capture_position = (self.capture_position + 2) & 1023;
        Ok(())
    }
    pub fn current_address(&self) -> usize {
        self.current_address
    }
    pub fn fifo_halfwords(&self) -> usize {
        self.fifo.len()
    }
    pub fn mode(&self) -> Mode {
        mode(self.applied_control)
    }

    pub fn dma_request(&self) -> bool {
        match self.mode() {
            Mode::DmaWrite => self.fifo.is_empty(),
            Mode::DmaRead => self.fifo.len() == FIFO_HALFWORDS,
            _ => false,
        }
    }

    pub fn status(&self) -> u16 {
        let request = self.dma_request();
        let busy = match self.mode() {
            Mode::Stopped => false,
            Mode::ManualWrite | Mode::DmaWrite => !self.fifo.is_empty(),
            Mode::DmaRead => self.fifo.len() < FIFO_HALFWORDS,
        };
        (self.applied_control & 0x3f)
            | ((self.capture_position & 512) << 2)
            | (u16::from(busy) << 10)
            | (u16::from(request) << 7)
            | (u16::from(request && self.mode() == Mode::DmaWrite) << 8)
            | (u16::from(request && self.mode() == Mode::DmaRead) << 9)
    }

    /// Offsets are relative to 1F801DA0. Unsupported ports never become RAM.
    pub fn read(&self, offset: u32) -> Result<u16> {
        match offset {
            4 => Ok(self.irq_address),
            6 => Ok(self.address_register),
            10 => Ok(self.control),
            12 => Ok(self.ram_control),
            14 => Ok(self.status()),
            8 => Err("SPU: manual FIFO reads are unverified; use DMA read".into()),
            _ => Err(format!("SPU: unsupported transfer register offset {offset:#x}").into()),
        }
    }

    pub fn write(&mut self, offset: u32, value: u16) -> Result<()> {
        match offset {
            4 => self.irq_address = value,
            6 => {
                self.address_register = value;
                self.current_address = usize::from(value) * 8;
            }
            8 => {
                if self.fifo.len() == FIFO_HALFWORDS {
                    return Err("SPU: transfer FIFO overflow".into());
                }
                if self.mode() == Mode::DmaRead || mode(self.control) == Mode::DmaRead {
                    return Err("SPU: manual write during DMA read is unsupported".into());
                }
                self.fifo.push_back(value);
            }
            10 => {
                if value & 0x40 != 0 {
                    return Err("SPU: IRQ9 address-comparison timing is not implemented".into());
                }
                if !self.fifo.is_empty()
                    && mode(value) == Mode::Stopped
                    && self.mode() != Mode::Stopped
                {
                    return Err("SPU: stopping with buffered transfer data has unverified drain/discard behavior".into());
                }
                if !self.fifo.is_empty()
                    && (mode(value) == Mode::DmaRead) != (self.mode() == Mode::DmaRead)
                {
                    return Err(
                        "SPU: changing FIFO direction with buffered data is unsupported".into(),
                    );
                }
                self.control = value;
            }
            12 => self.ram_control = value,
            14 => return Err("SPU: status-register writes are unverified".into()),
            _ => {
                return Err(format!("SPU: unsupported transfer register offset {offset:#x}").into())
            }
        }
        Ok(())
    }

    /// Apply a scheduled device-control boundary. No wall-clock delay is implied.
    pub fn apply_control(&mut self) -> Result<()> {
        if mode(self.control) != Mode::Stopped && self.ram_control != 4 {
            return Err(format!("SPU: RAM transfer control {:#06x} unsupported; normal 512 KiB mode requires 0x0004", self.ram_control).into());
        }
        self.applied_control = self.control;
        Ok(())
    }

    pub fn can_dma_word(&self, from_ram: bool) -> bool {
        if from_ram {
            self.mode() == Mode::DmaWrite && self.fifo.len() <= FIFO_HALFWORDS - 2
        } else {
            self.mode() == Mode::DmaRead && self.fifo.len() >= 2
        }
    }

    pub fn dma_write(&mut self, word: u32) -> Result<()> {
        if !self.can_dma_word(true) {
            return Err("SPU: DMA write without FIFO space or write mode".into());
        }
        self.fifo.push_back(word as u16);
        self.fifo.push_back((word >> 16) as u16);
        Ok(())
    }

    pub fn dma_read(&mut self) -> Result<u32> {
        if !self.can_dma_word(false) {
            return Err("SPU: DMA read without FIFO data or read mode".into());
        }
        let low = self.fifo.pop_front().unwrap();
        let high = self.fifo.pop_front().unwrap();
        Ok(u32::from(low) | (u32::from(high) << 16))
    }

    /// Move at most one halfword between FIFO and sound RAM. Address wraps at
    /// 512 KiB; the externally visible TSA register retains its programmed value.
    pub fn service_halfword(&mut self) -> Result<bool> {
        if self.mode() == Mode::Stopped {
            return Ok(false);
        }
        if self.ram_control != 4 {
            return Err("SPU: unsupported RAM transfer control".into());
        }
        match self.mode() {
            Mode::DmaRead if self.fifo.len() < FIFO_HALFWORDS => {
                let value = u16::from_le_bytes(
                    self.ram[self.current_address..self.current_address + 2].try_into()?,
                );
                self.fifo.push_back(value);
            }
            Mode::ManualWrite | Mode::DmaWrite => {
                let Some(value) = self.fifo.pop_front() else {
                    return Ok(false);
                };
                self.ram[self.current_address..self.current_address + 2]
                    .copy_from_slice(&value.to_le_bytes());
            }
            _ => return Ok(false),
        }
        self.current_address = (self.current_address + 2) % RAM_BYTES;
        Ok(true)
    }
}

fn mode(control: u16) -> Mode {
    match (control >> 4) & 3 {
        0 => Mode::Stopped,
        1 => Mode::ManualWrite,
        2 => Mode::DmaWrite,
        _ => Mode::DmaRead,
    }
}
