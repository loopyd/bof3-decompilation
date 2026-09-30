//! PCM16 RIFF/WAVE interchange. Rates are explicit, and loop endpoints are
//! exclusive internally but inclusive in RIFF smpl records. Unknown chunks,
//! chunk order and odd-byte padding survive parsing and serialization.

use crate::Result;
use serde::Serialize;

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct SampleLoop {
    pub identifier: u32,
    pub loop_type: u32,
    pub start_frame: u32,
    pub end_frame_exclusive: u32,
    pub fraction: u32,
    pub play_count: u32,
}

impl SampleLoop {
    pub fn forward(start_frame: u32, end_frame_exclusive: u32) -> Result<Self> {
        if start_frame >= end_frame_exclusive {
            return Err("WAV loop must have a nonempty frame range".into());
        }
        Ok(Self {
            identifier: 0,
            loop_type: 0,
            start_frame,
            end_frame_exclusive,
            fraction: 0,
            play_count: 0,
        })
    }
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct Sampler {
    pub manufacturer: u32,
    pub product: u32,
    pub sample_period_ns: u32,
    pub midi_unity_note: u32,
    pub midi_pitch_fraction: u32,
    pub smpte_format: u32,
    pub smpte_offset: u32,
    pub loops: Vec<SampleLoop>,
    pub sampler_data: Vec<u8>,
}

impl Sampler {
    pub fn new(sample_rate: u32, midi_unity_note: u32) -> Result<Self> {
        if sample_rate == 0 || midi_unity_note > 127 {
            return Err("WAV sampler needs a nonzero rate and MIDI unity note 0..127".into());
        }
        Ok(Self {
            manufacturer: 0,
            product: 0,
            sample_period_ns: 1_000_000_000 / sample_rate,
            midi_unity_note,
            midi_pitch_fraction: 0,
            smpte_format: 0,
            smpte_offset: 0,
            loops: Vec::new(),
            sampler_data: Vec::new(),
        })
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct OpaqueChunk {
    pub id: [u8; 4],
    pub data: Vec<u8>,
    pub padding: Option<u8>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
enum Slot {
    Format,
    Data,
    Sampler(u8),
    Fact(Vec<u8>, u8),
    Opaque(OpaqueChunk),
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Wave {
    pub channels: u16,
    pub sample_rate: u32,
    pub pcm: Vec<i16>,
    pub sampler: Option<Sampler>,
    format_suffix: Vec<u8>,
    layout: Vec<Slot>,
}

impl Wave {
    pub fn new(channels: u16, sample_rate: u32, pcm: Vec<i16>) -> Result<Self> {
        let wave = Self {
            channels,
            sample_rate,
            pcm,
            sampler: None,
            format_suffix: Vec::new(),
            layout: vec![Slot::Format, Slot::Data],
        };
        wave.validate()?;
        Ok(wave)
    }

    pub fn frames(&self) -> usize {
        self.pcm
            .len()
            .checked_div(usize::from(self.channels))
            .unwrap_or(0)
    }

    pub fn opaque_chunks(&self) -> impl Iterator<Item = &OpaqueChunk> {
        self.layout.iter().filter_map(|slot| {
            if let Slot::Opaque(chunk) = slot {
                Some(chunk)
            } else {
                None
            }
        })
    }

    /// Extra `fact` bytes have no defined PCM sample semantics.
    pub fn has_extended_fact(&self) -> bool {
        self.layout
            .iter()
            .any(|slot| matches!(slot, Slot::Fact(extra, _) if !extra.is_empty()))
    }

    /// Restrict parsed loop semantics to those representable by a normal SPU
    /// sample loop. Parsing retains other legal sampler metadata for diagnosis.
    pub fn single_forward_loop(&self) -> Result<Option<&SampleLoop>> {
        self.validate()?;
        let Some(sampler) = &self.sampler else {
            return Ok(None);
        };
        match sampler.loops.as_slice() {
            [] => Ok(None),
            [sample_loop]
                if sample_loop.loop_type == 0
                    && sample_loop.fraction == 0
                    && sample_loop.play_count == 0 =>
            {
                Ok(Some(sample_loop))
            }
            [_] => Err(
                "WAV sample loop cannot map to SPU: requires forward, whole-frame, infinite repeat"
                    .into(),
            ),
            _ => Err("WAV has multiple sample loops; a normal SPU sample supports one".into()),
        }
    }

    fn validate(&self) -> Result<()> {
        if !matches!(self.channels, 1 | 2) {
            return Err(format!(
                "unsupported WAV channel count {}; expected mono or stereo PCM16",
                self.channels
            )
            .into());
        }
        if self.sample_rate == 0
            || self
                .sample_rate
                .checked_mul(u32::from(self.channels) * 2)
                .is_none()
        {
            return Err("WAV sample rate is zero or byte rate overflows RIFF fields".into());
        }
        if !self.pcm.len().is_multiple_of(usize::from(self.channels)) {
            return Err("WAV PCM data contains an incomplete interleaved frame".into());
        }
        if let Some(sampler) = &self.sampler {
            if sampler.midi_unity_note > 127 {
                return Err("WAV smpl MIDI unity note is outside 0..127".into());
            }
            for (index, sample_loop) in sampler.loops.iter().enumerate() {
                if sample_loop.start_frame >= sample_loop.end_frame_exclusive
                    || u64::from(sample_loop.end_frame_exclusive) > self.frames() as u64
                {
                    return Err(format!(
                        "WAV smpl loop {index} range {}..{} is outside {} PCM frames",
                        sample_loop.start_frame,
                        sample_loop.end_frame_exclusive,
                        self.frames()
                    )
                    .into());
                }
            }
        }
        Ok(())
    }

    pub fn from_bytes(bytes: &[u8]) -> Result<Self> {
        if bytes.len() < 12 || &bytes[..4] != b"RIFF" || &bytes[8..12] != b"WAVE" {
            return Err(
                "expected little-endian RIFF/WAVE; RF64/RIFX and other containers are unsupported"
                    .into(),
            );
        }
        if u64::from(u32le(bytes, 4)) + 8 != bytes.len() as u64 {
            return Err("WAV RIFF size does not equal the input length".into());
        }
        let (mut format, mut pcm, mut sampler, mut fact) = (None, None, None, None);
        let mut layout = Vec::new();
        let mut position = 12usize;
        while position < bytes.len() {
            if bytes.len() - position < 8 {
                return Err(format!("truncated WAV chunk header at byte {position}").into());
            }
            let id: [u8; 4] = bytes[position..position + 4].try_into()?;
            let size = u32le(bytes, position + 4) as usize;
            let start = position + 8;
            let end = start.checked_add(size).ok_or("WAV chunk size overflow")?;
            let next = end
                .checked_add(size & 1)
                .ok_or("WAV chunk padding overflow")?;
            if next > bytes.len() {
                return Err(format!(
                    "truncated WAV chunk {:?} at byte {position}",
                    String::from_utf8_lossy(&id)
                )
                .into());
            }
            let data = &bytes[start..end];
            let padding = if size & 1 != 0 {
                Some(bytes[end])
            } else {
                None
            };
            match &id {
                b"fmt " => {
                    if format.is_some() {
                        return Err("duplicate WAV fmt chunk".into());
                    }
                    if !matches!(data.len(), 16 | 18) || (data.len() == 18 && u16le(data, 16) != 0)
                    {
                        return Err("unsupported WAV fmt extension; expected PCM fmt size 16 or 18 with cbSize zero".into());
                    }
                    if u16le(data, 0) != 1 || u16le(data, 14) != 16 {
                        return Err(
                            "unsupported WAV encoding; only signed PCM16 is accepted".into()
                        );
                    }
                    let channels = u16le(data, 2);
                    let rate = u32le(data, 4);
                    let align = u32::from(channels) * 2;
                    if u32::from(u16le(data, 12)) != align
                        || rate.checked_mul(align) != Some(u32le(data, 8))
                    {
                        return Err(
                            "WAV fmt block alignment or byte rate disagrees with PCM16 format"
                                .into(),
                        );
                    }
                    format = Some((channels, rate, data[16..].to_vec()));
                    layout.push(Slot::Format);
                }
                b"data" => {
                    if pcm.is_some() {
                        return Err("duplicate WAV data chunk".into());
                    }
                    if !data.len().is_multiple_of(2) {
                        return Err("WAV PCM16 data has an odd byte count".into());
                    }
                    pcm = Some(
                        data.as_chunks::<2>()
                            .0
                            .iter()
                            .map(|v| i16::from_le_bytes(*v))
                            .collect(),
                    );
                    layout.push(Slot::Data);
                }
                b"smpl" => {
                    if sampler.is_some() {
                        return Err("duplicate WAV smpl chunk".into());
                    }
                    sampler = Some(parse_sampler(data)?);
                    layout.push(Slot::Sampler(padding.unwrap_or(0)));
                }
                b"fact" => {
                    if fact.is_some() || data.len() < 4 {
                        return Err("duplicate or truncated WAV fact chunk".into());
                    }
                    fact = Some(u32le(data, 0));
                    layout.push(Slot::Fact(data[4..].to_vec(), padding.unwrap_or(0)));
                }
                b"LIST" if data.starts_with(b"wavl") => {
                    return Err(
                        "segmented WAV wavl data is unsupported; expected one PCM data chunk"
                            .into(),
                    );
                }
                _ => layout.push(Slot::Opaque(OpaqueChunk {
                    id,
                    data: data.to_vec(),
                    padding,
                })),
            }
            position = next;
        }
        let (channels, sample_rate, format_suffix) = format.ok_or("missing WAV fmt chunk")?;
        let wave = Self {
            channels,
            sample_rate,
            pcm: pcm.ok_or("missing WAV data chunk")?,
            sampler,
            format_suffix,
            layout,
        };
        wave.validate()?;
        if fact.is_some_and(|frames| u64::from(frames) != wave.frames() as u64) {
            return Err("WAV fact frame count disagrees with PCM data".into());
        }
        Ok(wave)
    }

    pub fn to_bytes(&self) -> Result<Vec<u8>> {
        self.validate()?;
        let capacity = self.encoded_len()?;
        let data_size = self
            .pcm
            .len()
            .checked_mul(2)
            .and_then(|v| u32::try_from(v).ok())
            .ok_or("WAV PCM data exceeds RIFF capacity")?;
        let mut out = Vec::new();
        out.try_reserve_exact(capacity)?;
        out.extend(b"RIFF\0\0\0\0WAVE");
        let mut wrote_sampler = false;
        for slot in &self.layout {
            match slot {
                Slot::Format => {
                    let mut fmt = Vec::new();
                    fmt.extend(1u16.to_le_bytes());
                    fmt.extend(self.channels.to_le_bytes());
                    fmt.extend(self.sample_rate.to_le_bytes());
                    fmt.extend((self.sample_rate * u32::from(self.channels) * 2).to_le_bytes());
                    fmt.extend((self.channels * 2).to_le_bytes());
                    fmt.extend(16u16.to_le_bytes());
                    fmt.extend(&self.format_suffix);
                    chunk(&mut out, b"fmt ", &fmt, 0)?;
                }
                Slot::Data => {
                    out.extend(b"data");
                    out.extend(data_size.to_le_bytes());
                    for sample in &self.pcm {
                        out.extend(sample.to_le_bytes())
                    }
                }
                Slot::Sampler(padding) => {
                    if let Some(sampler) = &self.sampler {
                        chunk(&mut out, b"smpl", &encode_sampler(sampler)?, *padding)?;
                        wrote_sampler = true;
                    }
                }
                Slot::Fact(extra, padding) => {
                    let mut fact = u32::try_from(self.frames())?.to_le_bytes().to_vec();
                    fact.extend(extra);
                    chunk(&mut out, b"fact", &fact, *padding)?;
                }
                Slot::Opaque(opaque) => chunk(
                    &mut out,
                    &opaque.id,
                    &opaque.data,
                    opaque.padding.unwrap_or(0),
                )?,
            }
        }
        if !wrote_sampler {
            if let Some(sampler) = &self.sampler {
                chunk(&mut out, b"smpl", &encode_sampler(sampler)?, 0)?
            }
        }
        let riff_size =
            u32::try_from(out.len() - 8).map_err(|_| "WAV output exceeds RIFF capacity")?;
        out[4..8].copy_from_slice(&riff_size.to_le_bytes());
        Ok(out)
    }

    fn encoded_len(&self) -> Result<usize> {
        let sampler_size = self
            .sampler
            .as_ref()
            .map(|sampler| {
                (sampler.loops.len() as u64)
                    .checked_mul(24)
                    .and_then(|size| size.checked_add(36))
                    .and_then(|size| size.checked_add(sampler.sampler_data.len() as u64))
                    .ok_or("WAV smpl size overflow")
            })
            .transpose()?;
        let pcm_size = (self.pcm.len() as u64)
            .checked_mul(2)
            .ok_or("WAV PCM size overflow")?;
        let mut total = 12u64;
        let mut add = |size: u64| -> Result<()> {
            if size > u64::from(u32::MAX) {
                return Err("WAV chunk exceeds RIFF capacity".into());
            }
            total = total
                .checked_add(8 + size + (size & 1))
                .ok_or("WAV size overflow")?;
            if total - 8 > u64::from(u32::MAX) {
                return Err("WAV output exceeds RIFF capacity".into());
            }
            Ok(())
        };
        let mut sampler_slot = false;
        for slot in &self.layout {
            match slot {
                Slot::Format => add(16 + self.format_suffix.len() as u64)?,
                Slot::Data => add(pcm_size)?,
                Slot::Sampler(_) => {
                    if let Some(size) = sampler_size {
                        add(size)?
                    }
                    sampler_slot = true;
                }
                Slot::Fact(extra, _) => add(4 + extra.len() as u64)?,
                Slot::Opaque(opaque) => add(opaque.data.len() as u64)?,
            }
        }
        if !sampler_slot {
            if let Some(size) = sampler_size {
                add(size)?
            }
        }
        Ok(usize::try_from(total)?)
    }
}

fn parse_sampler(data: &[u8]) -> Result<Sampler> {
    if data.len() < 36 {
        return Err("truncated WAV smpl header".into());
    }
    let count = u32le(data, 28) as usize;
    let tail = u32le(data, 32) as usize;
    let loops_end = count
        .checked_mul(24)
        .and_then(|v| v.checked_add(36))
        .ok_or("WAV smpl loop count overflow")?;
    if loops_end.checked_add(tail) != Some(data.len()) {
        return Err("WAV smpl loop count or sampler-data size disagrees with chunk size".into());
    }
    let mut loops = Vec::new();
    for index in 0..count {
        let offset = 36 + index * 24;
        loops.push(SampleLoop {
            identifier: u32le(data, offset),
            loop_type: u32le(data, offset + 4),
            start_frame: u32le(data, offset + 8),
            end_frame_exclusive: u32le(data, offset + 12)
                .checked_add(1)
                .ok_or("WAV inclusive loop endpoint overflows")?,
            fraction: u32le(data, offset + 16),
            play_count: u32le(data, offset + 20),
        });
    }
    Ok(Sampler {
        manufacturer: u32le(data, 0),
        product: u32le(data, 4),
        sample_period_ns: u32le(data, 8),
        midi_unity_note: u32le(data, 12),
        midi_pitch_fraction: u32le(data, 16),
        smpte_format: u32le(data, 20),
        smpte_offset: u32le(data, 24),
        loops,
        sampler_data: data[loops_end..].to_vec(),
    })
}

fn encode_sampler(sampler: &Sampler) -> Result<Vec<u8>> {
    let mut data = Vec::new();
    for word in [
        sampler.manufacturer,
        sampler.product,
        sampler.sample_period_ns,
        sampler.midi_unity_note,
        sampler.midi_pitch_fraction,
        sampler.smpte_format,
        sampler.smpte_offset,
        u32::try_from(sampler.loops.len())?,
        u32::try_from(sampler.sampler_data.len())?,
    ] {
        data.extend(word.to_le_bytes())
    }
    for sample_loop in &sampler.loops {
        for word in [
            sample_loop.identifier,
            sample_loop.loop_type,
            sample_loop.start_frame,
            sample_loop.end_frame_exclusive - 1,
            sample_loop.fraction,
            sample_loop.play_count,
        ] {
            data.extend(word.to_le_bytes())
        }
    }
    data.extend(&sampler.sampler_data);
    Ok(data)
}

fn chunk(out: &mut Vec<u8>, id: &[u8; 4], data: &[u8], padding: u8) -> Result<()> {
    let size = u32::try_from(data.len()).map_err(|_| "WAV chunk exceeds RIFF capacity")?;
    out.extend(id);
    out.extend(size.to_le_bytes());
    out.extend(data);
    if data.len() & 1 != 0 {
        out.push(padding)
    }
    Ok(())
}

fn u16le(bytes: &[u8], offset: usize) -> u16 {
    u16::from_le_bytes(
        bytes[offset..offset + 2]
            .try_into()
            .expect("validated WAV field bounds"),
    )
}
fn u32le(bytes: &[u8], offset: usize) -> u32 {
    u32::from_le_bytes(
        bytes[offset..offset + 4]
            .try_into()
            .expect("validated WAV field bounds"),
    )
}
