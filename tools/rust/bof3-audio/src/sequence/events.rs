//! SEP event framing supported by the linked US dispatcher, not MIDI translation.
//!
//! Source offsets retain running-status encodings and the opaque suffix after EOT.
//! Controller/loop semantics and scheduler quantization require separate validation.

use crate::Result;
use serde::Serialize;

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
#[serde(tag = "kind", rename_all = "snake_case")]
pub enum Kind {
    Note {
        key: u8,
        velocity: u8,
    },
    Controller {
        controller: u8,
        value: u8,
    },
    Program {
        program: u8,
    },
    /// The linked pitch handler discards the low byte. Keep it for reconstruction.
    PitchBend {
        low: u8,
        high: u8,
    },
    Tempo {
        microseconds_per_quarter: u32,
    },
    End,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct Event {
    /// Includes the preceding delta and any explicit status byte.
    pub offset: usize,
    pub encoded_bytes: usize,
    pub delta: u32,
    pub tick: u64,
    pub status: u8,
    pub explicit_status: bool,
    pub kind: Kind,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct Events {
    pub events: Vec<Event>,
    pub trailing_offset: usize,
    pub trailing_bytes: usize,
    pub semantic_validation: &'static str,
}

impl Events {
    pub fn parse(bytes: &[u8]) -> Result<Self> {
        let mut input = Input { bytes, pos: 0 };
        let mut events = Vec::new();
        let mut running = None;
        let mut tick = 0u64;
        loop {
            let offset = input.pos;
            let delta = input.delta()?;
            tick = tick
                .checked_add(u64::from(delta))
                .ok_or("SEP: tick overflow")?;
            let message = read_message(bytes, input.pos, running)?;
            input.pos = message.end;
            let Decoded {
                status,
                explicit_status,
                kind,
                ..
            } = message;
            running = Some(status);
            let end = kind == Kind::End;
            events.push(Event {
                offset,
                encoded_bytes: input.pos - offset,
                delta,
                tick,
                status,
                explicit_status,
                kind,
            });
            if end {
                return Ok(Self {
                    events,
                    trailing_offset: input.pos,
                    trailing_bytes: bytes.len() - input.pos,
                    semantic_validation: "framing_only",
                });
            }
        }
    }
}

/// A message starts after its delta. Loop jumps retain the dispatcher's running
/// status and may therefore decode a different channel than physical-order scans.
pub(crate) struct Decoded {
    pub end: usize,
    pub status: u8,
    pub explicit_status: bool,
    pub kind: Kind,
}

pub(crate) fn read_delta(bytes: &[u8], offset: usize) -> Result<(u32, usize)> {
    let mut input = Input { bytes, pos: offset };
    let delta = input.delta()?;
    Ok((delta, input.pos))
}

pub(crate) fn read_message(bytes: &[u8], offset: usize, running: Option<u8>) -> Result<Decoded> {
    let mut input = Input { bytes, pos: offset };
    let first = input.byte()?;
    let explicit_status = first & 0x80 != 0;
    let status = if explicit_status {
        first
    } else {
        input.pos -= 1;
        running.ok_or_else(|| format!("SEP event at byte {offset}: no running status"))?
    };
    let kind = match status {
        0x90..=0x9f => Kind::Note {
            key: input.data()?,
            velocity: input.data()?,
        },
        0xb0..=0xbf => Kind::Controller {
            controller: input.data()?,
            value: input.data()?,
        },
        0xc0..=0xcf => Kind::Program {
            program: input.data()?,
        },
        0xe0..=0xef => Kind::PitchBend {
            low: input.data()?,
            high: input.data()?,
        },
        0xff => {
            match input.byte()? {
                0x2f => Kind::End,
                0x51 => {
                    let tempo = (u32::from(input.byte()?) << 16)
                        | (u32::from(input.byte()?) << 8)
                        | u32::from(input.byte()?);
                    if tempo == 0 {
                        return Err(format!("SEP tempo at byte {offset}: zero divides the runtime tempo calculation").into());
                    }
                    Kind::Tempo {
                        microseconds_per_quarter: tempo,
                    }
                }
                meta => {
                    return Err(format!(
                        "SEP event at byte {offset}: unsupported meta type 0x{meta:02x}"
                    )
                    .into())
                }
            }
        }
        _ => {
            return Err(
                format!("SEP event at byte {offset}: unsupported US status 0x{status:02x}").into(),
            )
        }
    };
    Ok(Decoded {
        end: input.pos,
        status,
        explicit_status,
        kind,
    })
}

struct Input<'a> {
    bytes: &'a [u8],
    pos: usize,
}

impl Input<'_> {
    fn byte(&mut self) -> Result<u8> {
        let value = *self.bytes.get(self.pos).ok_or_else(|| {
            format!(
                "SEP: truncated event or missing end marker at byte {}",
                self.pos
            )
        })?;
        self.pos += 1;
        Ok(value)
    }

    fn data(&mut self) -> Result<u8> {
        let at = self.pos;
        let value = self.byte()?;
        if value & 0x80 != 0 {
            return Err(format!("SEP: non-seven-bit event data 0x{value:02x} at byte {at}").into());
        }
        Ok(value)
    }

    fn delta(&mut self) -> Result<u32> {
        let at = self.pos;
        let mut value = 0u32;
        for _ in 0..4 {
            let byte = self.byte()?;
            value = (value << 7) | u32::from(byte & 0x7f);
            if byte & 0x80 == 0 {
                return Ok(value);
            }
        }
        Err(format!("SEP delta at byte {at}: more than four VLQ bytes are unsupported").into())
    }
}
