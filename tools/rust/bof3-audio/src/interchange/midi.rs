//! Standard MIDI File framing and preservation. BOF3 event translation is separate.
//!
//! Supports format 0/1 with PPQN timing. Unchanged tracks retain running status
//! and VLQ encodings; changed tracks are serialized with explicit channel status.
//! Unknown chunks, header extensions, meta and SysEx payloads are never discarded.

use crate::Result;

const MAX_VLQ: u32 = 0x0fff_ffff;

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Message {
    Channel { status: u8, data: Vec<u8> },
    Meta { kind: u8, data: Vec<u8> },
    SysEx { status: u8, data: Vec<u8> },
}

impl Message {
    pub fn end() -> Self {
        Self::Meta {
            kind: 0x2f,
            data: Vec::new(),
        }
    }

    fn is_end(&self) -> bool {
        matches!(self, Self::Meta { kind: 0x2f, .. })
    }

    fn validate(&self) -> Result<()> {
        match self {
            Self::Channel { status, data } => {
                if data.len() != channel_bytes(*status)? || data.iter().any(|b| *b >= 128) {
                    return Err(format!(
                        "MIDI channel 0x{status:02x}: invalid data length or non-seven-bit data"
                    )
                    .into());
                }
            }
            Self::Meta { kind, data } => {
                if *kind >= 128 {
                    return Err("MIDI: meta type must be seven-bit".into());
                }
                if *kind == 0x2f && !data.is_empty() {
                    return Err("MIDI: end-of-track data must be empty".into());
                }
                length(data.len())?;
            }
            Self::SysEx { status, data } => {
                if !matches!(status, 0xf0 | 0xf7) {
                    return Err("MIDI: SysEx status must be F0 or F7".into());
                }
                length(data.len())?;
            }
        }
        Ok(())
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Event {
    pub tick: u64,
    pub message: Message,
}

#[derive(Clone, Debug)]
pub struct Track {
    pub events: Vec<Event>,
    original: Option<(Vec<Event>, Vec<u8>)>,
}

impl Track {
    pub fn new(events: Vec<Event>) -> Self {
        Self {
            events,
            original: None,
        }
    }

    fn validate(&self) -> Result<()> {
        let mut previous = 0;
        for (index, event) in self.events.iter().enumerate() {
            event.message.validate()?;
            let delta = event
                .tick
                .checked_sub(previous)
                .ok_or_else(|| format!("MIDI event {index}: ticks go backwards"))?;
            if delta > u64::from(MAX_VLQ) {
                return Err(
                    format!("MIDI event {index}: delta exceeds four-byte VLQ capacity").into(),
                );
            }
            if event.message.is_end() && index + 1 != self.events.len() {
                return Err(format!("MIDI event {index}: event follows end-of-track").into());
            }
            previous = event.tick;
        }
        if !self.events.last().is_some_and(|e| e.message.is_end()) {
            return Err("MIDI: missing end-of-track".into());
        }
        Ok(())
    }

    fn parse(bytes: &[u8]) -> Result<Self> {
        let mut input = Input::new(bytes);
        let mut tick = 0u64;
        let mut running = None;
        let mut events = Vec::new();
        while !input.done() {
            tick = tick
                .checked_add(u64::from(input.vlq()?))
                .ok_or("MIDI: tick overflow")?;
            let first = input.byte()?;
            let status = if first >= 128 {
                first
            } else {
                input.position -= 1;
                running.ok_or("MIDI: data byte without running channel status")?
            };
            let message = match status {
                0x80..=0xef => {
                    running = Some(status);
                    Message::Channel {
                        status,
                        data: input.take(channel_bytes(status)?)?.to_vec(),
                    }
                }
                0xff => {
                    running = None;
                    let kind = input.byte()?;
                    let len = input.vlq()? as usize;
                    Message::Meta {
                        kind,
                        data: input.take(len)?.to_vec(),
                    }
                }
                0xf0 | 0xf7 => {
                    running = None;
                    let len = input.vlq()? as usize;
                    Message::SysEx {
                        status,
                        data: input.take(len)?.to_vec(),
                    }
                }
                _ => {
                    return Err(
                        format!("MIDI: unsupported file-event status 0x{status:02x}").into(),
                    )
                }
            };
            message.validate()?;
            let end = message.is_end();
            events.push(Event { tick, message });
            if end && !input.done() {
                return Err("MIDI: bytes follow end-of-track inside track chunk".into());
            }
        }
        let track = Self {
            original: Some((events.clone(), bytes.to_vec())),
            events,
        };
        track.validate()?;
        Ok(track)
    }

    fn encode(&self) -> Result<Vec<u8>> {
        self.validate()?;
        if let Some((events, bytes)) = &self.original {
            if events == &self.events {
                return Ok(bytes.clone());
            }
        }
        let mut output = Vec::new();
        let mut previous = 0;
        for event in &self.events {
            vlq((event.tick - previous) as u32, &mut output);
            match &event.message {
                Message::Channel { status, data } => {
                    output.push(*status);
                    output.extend(data);
                }
                Message::Meta { kind, data } => {
                    output.extend([0xff, *kind]);
                    vlq(length(data.len())?, &mut output);
                    output.extend(data);
                }
                Message::SysEx { status, data } => {
                    output.push(*status);
                    vlq(length(data.len())?, &mut output);
                    output.extend(data);
                }
            }
            previous = event.tick;
        }
        Ok(output)
    }
}

#[derive(Clone, Debug)]
enum Chunk {
    Track(usize),
    Opaque([u8; 4], Vec<u8>),
}

#[derive(Clone, Debug)]
pub struct Midi {
    pub format: u16,
    pub ppqn: u16,
    pub tracks: Vec<Track>,
    header_extension: Vec<u8>,
    chunks: Vec<Chunk>,
}

impl Midi {
    pub fn new(format: u16, ppqn: u16, tracks: Vec<Track>) -> Result<Self> {
        let chunks = (0..tracks.len()).map(Chunk::Track).collect();
        let file = Self {
            format,
            ppqn,
            tracks,
            header_extension: Vec::new(),
            chunks,
        };
        file.validate()?;
        Ok(file)
    }

    pub fn from_bytes(bytes: &[u8]) -> Result<Self> {
        let mut input = Input::new(bytes);
        if input.take(4)? != b"MThd" {
            return Err("MIDI: missing MThd header".into());
        }
        let len = input.word()? as usize;
        let mut header = Input::new(input.take(len)?);
        let format = header.half()?;
        let count = header.half()? as usize;
        let ppqn = header.half()?;
        let header_extension = header.take(len - 6)?.to_vec();
        let mut tracks = Vec::new();
        let mut chunks = Vec::new();
        while !input.done() {
            let id: [u8; 4] = input.take(4)?.try_into().unwrap();
            let size = input.word()? as usize;
            let payload = input.take(size)?;
            match &id {
                b"MThd" => return Err("MIDI: duplicate header chunk".into()),
                b"MTrk" => {
                    let index = tracks.len();
                    tracks.push(
                        Track::parse(payload).map_err(|e| format!("MIDI track {index}: {e}"))?,
                    );
                    chunks.push(Chunk::Track(index));
                }
                _ => chunks.push(Chunk::Opaque(id, payload.to_vec())),
            }
        }
        if tracks.len() != count {
            return Err(format!(
                "MIDI: header declares {count} tracks, found {}",
                tracks.len()
            )
            .into());
        }
        let file = Self {
            format,
            ppqn,
            tracks,
            header_extension,
            chunks,
        };
        file.validate()?;
        Ok(file)
    }

    fn validate(&self) -> Result<()> {
        if self.format > 1 {
            return Err(format!(
                "MIDI: format {} is unsupported; independent sequences require separate files",
                self.format
            )
            .into());
        }
        if self.ppqn & 0x8000 != 0 {
            return Err("MIDI: SMPTE time division is unsupported".into());
        }
        if self.ppqn == 0 {
            return Err("MIDI: PPQN must be nonzero".into());
        }
        if self.tracks.is_empty()
            || self.tracks.len() > u16::MAX as usize
            || (self.format == 0 && self.tracks.len() != 1)
        {
            return Err("MIDI: invalid track count for format".into());
        }
        if self
            .chunks
            .iter()
            .filter(|c| matches!(c, Chunk::Track(_)))
            .count()
            != self.tracks.len()
        {
            return Err("MIDI: track layout changed; construct a new file explicitly".into());
        }
        for (index, track) in self.tracks.iter().enumerate() {
            track
                .validate()
                .map_err(|e| format!("MIDI track {index}: {e}"))?;
        }
        Ok(())
    }

    pub fn to_bytes(&self) -> Result<Vec<u8>> {
        self.validate()?;
        let mut header = Vec::new();
        header.extend(self.format.to_be_bytes());
        header.extend((self.tracks.len() as u16).to_be_bytes());
        header.extend(self.ppqn.to_be_bytes());
        header.extend(&self.header_extension);
        let mut output = Vec::new();
        chunk(b"MThd", &header, &mut output)?;
        for part in &self.chunks {
            match part {
                Chunk::Track(index) => chunk(b"MTrk", &self.tracks[*index].encode()?, &mut output)?,
                Chunk::Opaque(id, data) => chunk(id, data, &mut output)?,
            }
        }
        Ok(output)
    }
}

fn channel_bytes(status: u8) -> Result<usize> {
    match status {
        0x80..=0xbf | 0xe0..=0xef => Ok(2),
        0xc0..=0xdf => Ok(1),
        _ => Err(format!("MIDI: invalid channel status 0x{status:02x}").into()),
    }
}

fn length(value: usize) -> Result<u32> {
    let value = u32::try_from(value).map_err(|_| "MIDI: event length exceeds u32")?;
    if value > MAX_VLQ {
        return Err("MIDI: event length exceeds four-byte VLQ capacity".into());
    }
    Ok(value)
}

fn vlq(mut value: u32, output: &mut Vec<u8>) {
    let mut bytes = [0u8; 4];
    let mut index = 3;
    bytes[index] = (value & 0x7f) as u8;
    while {
        value >>= 7;
        value != 0
    } {
        index -= 1;
        bytes[index] = (value as u8 & 0x7f) | 0x80;
    }
    output.extend_from_slice(&bytes[index..]);
}

fn chunk(id: &[u8; 4], bytes: &[u8], output: &mut Vec<u8>) -> Result<()> {
    let len = u32::try_from(bytes.len()).map_err(|_| "MIDI: chunk exceeds u32 length")?;
    output.extend(id);
    output.extend(len.to_be_bytes());
    output.extend(bytes);
    Ok(())
}

struct Input<'a> {
    bytes: &'a [u8],
    position: usize,
}
impl<'a> Input<'a> {
    fn new(bytes: &'a [u8]) -> Self {
        Self { bytes, position: 0 }
    }
    fn done(&self) -> bool {
        self.position == self.bytes.len()
    }
    fn take(&mut self, len: usize) -> Result<&'a [u8]> {
        if len > self.bytes.len() - self.position {
            return Err(format!(
                "MIDI: truncated data at byte {} (need {len})",
                self.position
            )
            .into());
        }
        let start = self.position;
        self.position += len;
        Ok(&self.bytes[start..self.position])
    }
    fn byte(&mut self) -> Result<u8> {
        Ok(self.take(1)?[0])
    }
    fn half(&mut self) -> Result<u16> {
        Ok(u16::from_be_bytes(self.take(2)?.try_into().unwrap()))
    }
    fn word(&mut self) -> Result<u32> {
        Ok(u32::from_be_bytes(self.take(4)?.try_into().unwrap()))
    }
    fn vlq(&mut self) -> Result<u32> {
        let mut value = 0;
        for _ in 0..4 {
            let byte = self.byte()?;
            value = (value << 7) | u32::from(byte & 0x7f);
            if byte < 128 {
                return Ok(value);
            }
        }
        Err("MIDI: VLQ exceeds four bytes".into())
    }
}
