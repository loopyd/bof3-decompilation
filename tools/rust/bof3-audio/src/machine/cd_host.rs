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
    awaiting_response: bool,
    response: Response,
    pending_response: Option<Response>,
    awaiting_interrupt: bool,
    interrupt_enable: u8,
    volume: Volume,
    data: Option<cd_data::Fifo>,
}

#[derive(Default)]
struct Response {
    payload: [u8; 16],
    size: usize,
    cursor: usize,
    drained: bool,
    interrupt: u8,
}

impl Response {
    fn new(interrupt: u8, bytes: &[u8]) -> Result<Self> {
        if bytes.is_empty() || bytes.len() > 16 {
            return Err("CD host: response requires 1..16 bytes".into());
        }
        let mut response = Self {
            size: bytes.len(),
            interrupt,
            ..Self::default()
        };
        response.payload[..bytes.len()].copy_from_slice(bytes);
        Ok(response)
    }

    fn empty(&self) -> bool {
        self.interrupt == 0 && (self.size == 0 || self.drained)
    }

    fn read(&mut self) -> u8 {
        self.cursor &= 15;
        let value = self.payload[self.cursor];
        self.cursor += 1;
        self.drained |= self.cursor == self.size;
        value
    }
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
            awaiting_response: false,
            response: Response::default(),
            pending_response: None,
            awaiting_interrupt: false,
            interrupt_enable: 0,
            volume: Volume::new(volume)?,
            data: None,
        })
    }
    pub fn irq_line(&self) -> bool {
        self.interrupt_enable & self.response.interrupt != 0
    }
    pub fn volume(&self) -> &Volume {
        &self.volume
    }
    pub fn busy(&self) -> bool {
        self.command.is_some()
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
    /// Atomically select data and publish INT1 at an explicit delivery boundary.
    /// Prior command/response/IRQ work must be fully drained. An unrequested
    /// old data block may be displaced; active reads remain protected.
    pub fn deliver_data(&mut self, status: u8, bytes: &[u8]) -> Result<usize> {
        if self.command.is_some()
            || self.awaiting_response
            || !self.response.empty()
            || self.pending_response.is_some()
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

    /// Explicit native FIFO callback boundary, without an inferred delay.
    /// An acknowledged head can be replaced by the tail even if unread. A
    /// command starts only after both response causes have cleared.
    pub fn service_fifo(&mut self) -> Option<Command> {
        if self.awaiting_interrupt {
            return None;
        }
        if self.response.interrupt == 0 {
            if let Some(response) = self.pending_response.take() {
                self.response = response;
            }
        }
        if self.response.interrupt != 0 || self.pending_response.is_some() {
            return None;
        }
        let opcode = self.command.take()?;
        self.awaiting_response = true;
        Some(Command {
            opcode,
            parameters: std::mem::take(&mut self.parameters),
        })
    }

    /// Publish an explicit pre-IRQ observation with an otherwise empty response
    /// queue. BUSY clears at command acceptance, independently of this boundary.
    pub fn stage_response(&mut self, bytes: &[u8]) -> Result<()> {
        let response = Response::new(0, bytes)?;
        if self.command.is_some() {
            return Err("CD host: drive must accept the pending command before responding".into());
        }
        if !self.response.empty() || self.pending_response.is_some() || self.awaiting_interrupt {
            return Err("CD host: staged response requires an empty response queue".into());
        }
        self.response = response;
        self.awaiting_response = false;
        self.awaiting_interrupt = true;
        Ok(())
    }

    /// Signal the previously staged response, even if software already read it.
    pub fn raise_interrupt(&mut self, interrupt: u8) -> Result<()> {
        if !(1..=5).contains(&interrupt) || !self.awaiting_interrupt || self.response.interrupt != 0
        {
            return Err("CD host: IRQ 1..5 requires a staged response and no pending IRQ".into());
        }
        self.response.interrupt = interrupt;
        self.awaiting_interrupt = false;
        Ok(())
    }

    /// Insert a native response candidate at a supplied MCU boundary. Retain two
    /// responses; a full queue drops another candidate. An acknowledged head
    /// with a tail promotes that tail before appending the new candidate.
    pub fn respond(&mut self, interrupt: u8, bytes: &[u8]) -> Result<()> {
        if !(1..=5).contains(&interrupt) {
            return Err("CD host: response requires IRQ 1..5".into());
        }
        let response = Response::new(interrupt, bytes)?;
        if self.awaiting_interrupt {
            return Err("CD host: staged response must receive its interrupt first".into());
        }
        self.awaiting_response = false;
        if self.response.interrupt == 0 && self.pending_response.is_some() {
            self.response = self.pending_response.replace(response).unwrap();
        } else if self.response.empty() {
            self.response = response;
        } else if self.pending_response.is_none() {
            self.pending_response = Some(response);
        }
        Ok(())
    }

    pub fn read(&mut self, offset: u8) -> Result<u8> {
        Ok(match offset {
            0 => {
                self.index
                    | (u8::from(self.parameters.is_empty()) << 3)
                    | (u8::from(self.parameters.len() < 16) << 4)
                    | (u8::from(self.response.cursor != self.response.size) << 5)
                    | (u8::from(self.data_ready()) << 6)
                    | (u8::from(self.busy()) << 7)
            }
            1 => self.response.read(),
            2 => self.read_data(1)? as u8,
            3 if self.index & 1 == 0 => 0xe0 | self.interrupt_enable,
            3 => 0xe0 | self.response.interrupt,
            _ => return Err("CD host: register offset outside 0..3".into()),
        })
    }

    pub fn write(&mut self, offset: u8, value: u8) -> Result<()> {
        match (self.index, offset) {
            (_, 0) => self.index = value & 3,
            (0, 1) => {
                self.command = Some(value);
            }
            (0, 2) => {
                if self.parameters.len() < 16 {
                    self.parameters.push(value);
                }
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
                self.response.interrupt &= !(value & 0x1f);
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
