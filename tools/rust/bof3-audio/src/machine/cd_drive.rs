//! Functional Mode-2 drive transactions with caller-supplied mechanical boundaries.
//! No BIOS defaults, authentication, media clock or response latency is inferred.
use super::{cd_host::Command, cd_position::CdPosition};
use crate::{codec::edc, Result};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Response {
    pub interrupt: u8,
    pub bytes: Vec<u8>,
}

#[derive(Debug)]
pub struct AppliedCommand {
    pub response: Response,
    pub audio_reset: bool,
    pub audio_selection_released: bool,
    pub discarded_audio_frames: usize,
    pub discarded_data_bytes: usize,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Activity {
    Paused,
    Resetting,
    Seeking { target: i32, read: bool },
    Reading,
    Pausing { was_reading: bool },
}

#[derive(Debug, PartialEq, Eq)]
pub enum Delivery {
    /// Caller publishes the block and INT1 at its selected service boundary.
    Data {
        bytes: Vec<u8>,
        response: Response,
    },
    /// Extracted 2336-byte sector for the existing XA decoder; no data IRQ.
    Xa(Vec<u8>),
    Filtered,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Drive {
    activity: Activity,
    mode: u8,
    filter: [u8; 2],
    next_lba: i32,
    target: Option<i32>,
    last_header: Option<[u8; 8]>,
    muted: bool,
    xa_selection: Option<[u8; 2]>,
}

impl Drive {
    /// Explicit already-authenticated, closed, spinning, unmuted Mode-2 context.
    /// This constructor is not a substitute for Init, boot or authentication.
    pub fn ready(position: CdPosition, mode: u8, filter: [u8; 2]) -> Result<Self> {
        check_mode(mode)?;
        if position.lba() < 0 {
            return Err("CD drive: lead-in media is not implemented".into());
        }
        Ok(Self {
            activity: Activity::Paused,
            mode,
            filter,
            next_lba: position.lba(),
            target: None,
            last_header: None,
            muted: false,
            xa_selection: None,
        })
    }

    pub fn activity(&self) -> Activity {
        self.activity
    }
    pub fn next_lba(&self) -> i32 {
        self.next_lba
    }
    pub fn muted(&self) -> bool {
        self.muted
    }
    pub fn status(&self) -> u8 {
        2 | match self.activity {
            Activity::Reading | Activity::Pausing { was_reading: true } => 0x20,
            Activity::Seeking { .. } => 0x40,
            Activity::Paused | Activity::Resetting | Activity::Pausing { was_reading: false } => 0,
        }
    }
    fn response(&self, interrupt: u8) -> Response {
        Response {
            interrupt,
            bytes: vec![self.status()],
        }
    }
    fn parameter_error(&self, code: u8) -> Response {
        Response {
            interrupt: 5,
            bytes: vec![self.status() | 1, code],
        }
    }

    /// Execute one accepted command. The caller owns host acceptance/IRQ timing.
    /// Unsupported operations return Err without changing drive state; known bad
    /// parameters produce the documented INT5 response instead of success.
    pub fn command(&mut self, command: &Command) -> Result<Response> {
        let count = match command.opcode {
            0x01 | 0x06 | 0x09 | 0x0a | 0x0b | 0x0c | 0x0f | 0x10 | 0x15 | 0x16 | 0x1b => 0,
            0x02 => 3,
            0x0d => 2,
            0x0e => 1,
            value => {
                return Err(format!("CD drive: command {value:#04x} is not implemented").into())
            }
        };
        if command.parameters.len() != count {
            return Ok(self.parameter_error(0x20));
        }
        if matches!(
            self.activity,
            Activity::Seeking { .. } | Activity::Pausing { .. } | Activity::Resetting
        ) && !matches!(command.opcode, 0x01 | 0x0b | 0x0c | 0x0f | 0x10)
        {
            return Err("CD drive: command during mechanical transition is not implemented".into());
        }
        let response = self.response(3);
        match command.opcode {
            0x01 => {}
            0x02 => {
                let Ok(position) = CdPosition::from_bcd(command.parameters[..].try_into()?) else {
                    return Ok(self.parameter_error(0x10));
                };
                if position.lba() < 0 {
                    return Err("CD drive: seeking into lead-in is not implemented".into());
                }
                self.target = Some(position.lba());
            }
            0x06 | 0x1b | 0x15 | 0x16 => {
                let read = matches!(command.opcode, 0x06 | 0x1b);
                if read
                    && self.target.is_none_or(|target| target == self.next_lba)
                    && self.activity == Activity::Reading
                {
                    self.target = None;
                    return Ok(response);
                }
                // Read without Setloc resumes the most recently received sector.
                let target = self.target.take().unwrap_or_else(|| {
                    self.last_header.map_or(self.next_lba, |h| {
                        CdPosition::from_bcd(h[..3].try_into().unwrap())
                            .unwrap()
                            .lba()
                    })
                });
                self.activity = Activity::Seeking { target, read };
                self.xa_selection = None;
                self.last_header = None;
            }
            0x09 => {
                self.xa_selection = None;
                self.activity = Activity::Pausing {
                    was_reading: self.activity == Activity::Reading,
                }
            }
            0x0a => {
                // PCSX-Redux's CdlReset (SDK CdlInit, opcode 0x0a) stops
                // reading, unmutes and selects 2340-byte mode. It preserves
                // filter, Setloc and transfer-header state. The caller supplies
                // the separate completion boundary; no reset latency inferred.
                self.activity = Activity::Resetting;
                self.mode = 0x20;
                self.muted = false;
                self.xa_selection = None;
                return Ok(self.response(3));
            }
            0x0d => {
                self.filter.copy_from_slice(&command.parameters);
                self.xa_selection = None;
            }
            0x0b => self.muted = true,
            0x0c => self.muted = false,
            0x0e => {
                check_mode(command.parameters[0])?;
                self.mode = command.parameters[0];
            }
            0x0f => {
                return Ok(Response {
                    interrupt: 3,
                    bytes: vec![self.status(), self.mode, 0, self.filter[0], self.filter[1]],
                })
            }
            0x10 => {
                return Ok(match self.last_header {
                    Some(header) => Response {
                        interrupt: 3,
                        bytes: header.to_vec(),
                    },
                    None => self.parameter_error(0x80),
                })
            }
            _ => unreachable!(),
        }
        Ok(response)
    }

    /// Complete a supplied seek/pause/reset boundary. No duration is assigned here.
    /// An implicit read seek has no INT2; explicit SeekL/SeekP and Pause do.
    pub fn complete(&mut self) -> Result<Option<Response>> {
        match self.activity {
            Activity::Seeking { target, read } => {
                self.next_lba = target;
                self.activity = if read {
                    Activity::Reading
                } else {
                    Activity::Paused
                };
                Ok((!read).then(|| self.response(2)))
            }
            Activity::Pausing { .. } | Activity::Resetting => {
                self.activity = Activity::Paused;
                Ok(Some(self.response(2)))
            }
            _ => Err("CD drive: no mechanical transition to complete".into()),
        }
    }

    /// Deliver a clean raw Mode-2 sector at the next expected position.
    /// No overrun, deferred-data retry or disc-end behaviour is fabricated.
    /// All checks precede state changes, including EDC and routing checks.
    pub fn sector(&mut self, raw: &[u8]) -> Result<Delivery> {
        if self.activity != Activity::Reading {
            return Err("CD drive: sector arrival requires completed read seek".into());
        }
        if raw.len() != 2352
            || raw[..12] != [0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0]
            || raw[15] != 2
            || raw[16..20] != raw[20..24]
        {
            return Err("CD drive: expected raw Mode-2 sector with matching subheaders".into());
        }
        let position = CdPosition::from_bcd(raw[12..15].try_into()?)?;
        if position.lba() != self.next_lba {
            return Err(format!(
                "CD drive: sector LBA {} differs from expected {}",
                position.lba(),
                self.next_lba
            )
            .into());
        }
        let next_lba = self
            .next_lba
            .checked_add(1)
            .ok_or("CD drive LBA overflow")?;
        CdPosition::from_lba(next_lba)?;
        if raw[18] & 0x20 != 0 {
            edc::check_form2(&raw[16..])?;
        } else if edc::checksum(&raw[16..2072]) != u32::from_le_bytes(raw[2072..2076].try_into()?) {
            return Err("CD drive: Form-1 EDC mismatch; ECC repair is not implemented".into());
        }
        let audio = raw[18] & 0x44 == 0x44;
        let filtered = self.mode & 8 != 0;
        let delivery = if audio && filtered && raw[16..18] != self.filter {
            Delivery::Filtered
        } else if audio && self.mode & 0x40 != 0 {
            let stream = [raw[16], raw[17]];
            // Emulator-reference selection precedes decoder admission. A
            // filtered-out EOF cannot release another stream's selection.
            if self.xa_selection.is_some_and(|selected| selected != stream)
                || (self.xa_selection.is_none() && !filtered && stream[1] == 255)
            {
                Delivery::Filtered
            } else {
                self.xa_selection = (raw[18] & 0x80 == 0).then_some(stream);
                Delivery::Xa(raw[16..].to_vec())
            }
        } else if audio && filtered {
            // Published routing rejects realtime audio from data delivery even
            // if XA decoding is disabled. Deferred retry behaviour is excluded.
            Delivery::Filtered
        } else {
            let bytes = if self.mode & 0x20 != 0 {
                &raw[12..]
            } else {
                &raw[24..2072]
            };
            Delivery::Data {
                bytes: bytes.to_vec(),
                response: self.response(1),
            }
        };
        self.last_header = Some(raw[12..20].try_into()?);
        self.next_lba = next_lba;
        Ok(delivery)
    }
}

fn check_mode(mode: u8) -> Result<()> {
    if mode & !0xe8 != 0 {
        return Err(format!(
            "CD drive: CDDA/report/autopause/ignore mode bits {:#04x} unimplemented",
            mode & !0xe8
        )
        .into());
    }
    Ok(())
}
