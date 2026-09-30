//! CD decoder host registers; caller schedules the separate drive MCU responses.
//! No disc-command semantics, sector delivery or clock timing are inferred here.
use super::{cd_audio::Volume, cd_data};
use crate::Result;

#[derive(Clone, Copy, Debug)]
pub enum Model {
    /// Pinned PCSX-Redux reset state, not a hardware power-on timing model.
    EmulatorReference,
}

#[derive(Debug, PartialEq, Eq)]
pub struct Command {
    pub opcode: u8,
    pub parameters: Vec<u8>,
}

pub struct Host {
    index: u8,
    parameters: Vec<u8>,
    command: Option<u8>,
    busy: bool,
    response: [u8; 16],
    response_cursor: usize,
    response_remaining: usize,
    awaiting_interrupt: bool,
    interrupt_enable: u8,
    interrupt_flags: u8,
    volume: Volume,
    data: Option<cd_data::Fifo>,
}

impl Host {
    /// Explicit reset register state. No media, response or completion is seeded.
    pub fn from_reset(model: Model) -> Result<Self> {
        match model {
            Model::EmulatorReference => {
                let mut host = Self::new([128, 0, 128, 0])?;
                host.interrupt_enable = 0x1f;
                Ok(host)
            }
        }
    }

    /// Explicit empty host-interface context, not an original BIOS/drive state.
    pub fn new(volume: [u8; 4]) -> Result<Self> {
        Ok(Self {
            index: 0,
            parameters: Vec::new(),
            command: None,
            busy: false,
            response: [0; 16],
            response_cursor: 0,
            response_remaining: 0,
            awaiting_interrupt: false,
            interrupt_enable: 0,
            interrupt_flags: 0,
            volume: Volume::new(volume)?,
            data: None,
        })
    }
    pub fn irq_line(&self) -> bool {
        self.interrupt_enable & self.interrupt_flags != 0
    }
    pub fn volume(&self) -> &Volume {
        &self.volume
    }
    pub fn busy(&self) -> bool {
        self.busy
    }
    pub fn configure_data(&mut self, model: cd_data::Model) -> Result<()> {
        if self.data.is_some() {
            return Err("CD data already configured".into());
        }
        self.data = Some(cd_data::Fifo::new(model));
        Ok(())
    }
    pub fn present_data(&mut self, bytes: &[u8]) -> Result<()> {
        self.data
            .as_mut()
            .ok_or("CD data model is not configured")?
            .present(bytes)
    }
    pub fn data_ready(&self) -> bool {
        self.data.as_ref().is_some_and(cd_data::Fifo::ready)
    }
    pub(crate) fn reset_data(&mut self) -> usize {
        self.data.as_mut().map_or(0, cd_data::Fifo::reset)
    }
    /// Atomically select data and publish INT1 at an explicit delivery boundary.
    /// Prior command/response/IRQ work must be fully drained. An unrequested
    /// old data block may be displaced; active reads remain protected.
    pub fn deliver_data(&mut self, status: u8, bytes: &[u8]) -> Result<usize> {
        if self.busy
            || self.command.is_some()
            || self.interrupt_flags != 0
            || self.response_remaining != 0
            || self.awaiting_interrupt
        {
            return Err("CD host: INT1 delivery requires idle command/response/IRQ state".into());
        }
        let displaced = self
            .data
            .as_mut()
            .ok_or("CD data model is not configured")?
            .select_for_interrupt(bytes)?;
        // All response preconditions were checked before selecting the block.
        self.respond(1, &[status])?;
        Ok(displaced)
    }
    pub fn read_data(&mut self, bytes: usize) -> Result<u32> {
        self.data
            .as_mut()
            .ok_or("CD data model is not configured")?
            .read(bytes)
    }

    /// Explicit MCU acceptance boundary. BUSY remains set until its response.
    pub fn take_command(&mut self) -> Option<Command> {
        self.command.take().map(|opcode| Command {
            opcode,
            parameters: std::mem::take(&mut self.parameters),
        })
    }

    /// Publish response bytes and clear BUSY independently of IRQ delivery.
    pub fn stage_response(&mut self, bytes: &[u8]) -> Result<()> {
        if bytes.is_empty() || bytes.len() > 16 {
            return Err("CD host: response requires 1..16 bytes".into());
        }
        if self.command.is_some() {
            return Err("CD host: drive must accept the pending command before responding".into());
        }
        if self.interrupt_flags != 0 || self.response_remaining != 0 || self.awaiting_interrupt {
            return Err("CD host: previous response must be acknowledged and drained; response queue scheduling is external".into());
        }
        self.response.fill(0);
        self.response[..bytes.len()].copy_from_slice(bytes);
        self.response_cursor = 0;
        self.response_remaining = bytes.len();
        self.awaiting_interrupt = true;
        self.busy = false;
        Ok(())
    }

    /// Signal the previously staged response, even if software already read it.
    pub fn raise_interrupt(&mut self, interrupt: u8) -> Result<()> {
        if !(1..=5).contains(&interrupt) || !self.awaiting_interrupt || self.interrupt_flags != 0 {
            return Err("CD host: IRQ 1..5 requires a staged response and no pending IRQ".into());
        }
        self.interrupt_flags = interrupt;
        self.awaiting_interrupt = false;
        Ok(())
    }

    /// Convenience for explicitly coincident response/IRQ boundaries, not latency.
    pub fn respond(&mut self, interrupt: u8, bytes: &[u8]) -> Result<()> {
        if !(1..=5).contains(&interrupt) {
            return Err("CD host: response requires IRQ 1..5".into());
        }
        self.stage_response(bytes)?;
        self.raise_interrupt(interrupt)
    }

    pub fn read(&mut self, offset: u8) -> Result<u8> {
        Ok(match offset {
            0 => {
                self.index
                    | (u8::from(self.parameters.is_empty()) << 3)
                    | (u8::from(self.parameters.len() < 16) << 4)
                    | (u8::from(self.response_remaining != 0) << 5)
                    | (u8::from(self.data_ready()) << 6)
                    | (u8::from(self.busy) << 7)
            }
            1 => {
                let value = self.response[self.response_cursor];
                self.response_cursor = (self.response_cursor + 1) & 15;
                self.response_remaining = self.response_remaining.saturating_sub(1);
                value
            }
            2 => self.read_data(1)? as u8,
            3 if self.index & 1 == 0 => 0xe0 | self.interrupt_enable,
            3 => 0xe0 | self.interrupt_flags,
            _ => return Err("CD host: register offset outside 0..3".into()),
        })
    }

    pub fn write(&mut self, offset: u8, value: u8) -> Result<()> {
        match (self.index, offset) {
            (_, 0) => self.index = value & 3,
            (0, 1) => {
                if self.busy
                    || self.interrupt_flags != 0
                    || self.response_remaining != 0
                    || self.awaiting_interrupt
                {
                    return Err(
                        "CD host: overlapping command or undrained response is unsupported".into(),
                    );
                }
                self.command = Some(value);
                self.busy = true;
            }
            (0, 2) => {
                if self.parameters.len() == 16 {
                    return Err("CD host: parameter FIFO overflow is unsupported".into());
                }
                self.parameters.push(value);
            }
            (0, 3) if value & !0x80 == 0 => {
                if let Some(data) = &mut self.data {
                    data.request(value != 0)?;
                } else if value != 0 {
                    return Err("CD data model is not configured".into());
                }
            }
            (0, 3) => return Err(
                "CD host: reserved request bits and manual XA sound-map controls are unimplemented"
                    .into(),
            ),
            (1, 2) => self.interrupt_enable = value & 0x1f,
            (1, 3) => {
                if value & 0xa0 != 0 {
                    return Err(
                        "CD host: decoder reset and XA sound-map clear are unimplemented".into(),
                    );
                }
                self.interrupt_flags &= !(value & 0x1f);
                if value & 0x40 != 0 {
                    self.parameters.clear();
                }
            }
            (2, 2 | 3) | (3, 1..=3) => self.volume.write(self.index, offset, value)?,
            (1 | 2, 1) => {
                return Err("CD host: manual XA data/coding ports are unimplemented".into())
            }
            _ => return Err("CD host: register offset outside 0..3".into()),
        }
        Ok(())
    }
}
