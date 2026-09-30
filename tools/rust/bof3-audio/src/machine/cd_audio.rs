//! XA-to-SPU sample processing at caller-selected boundaries; no drive scheduler.
use super::xa_filter::{HALF_RATE, ZIGZAG};
use crate::{
    xa::{Arithmetic, Decoder, Format, Histories, Stream},
    Result,
};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Model {
    /// Published 37.8 kHz indexing. No published 18.9 kHz algorithm is established.
    Published37800,
    /// Inspected emulator's alignment and separate half-rate filter.
    EmulatorReference,
}

#[derive(Clone)]
pub struct Resampler {
    format: Format,
    model: Model,
    ring: [[i16; 32]; 2],
    position: usize,
    phase: u8,
}

impl Resampler {
    /// Zero history, position zero and phase six are an explicit reset context,
    /// not evidence that an original seek/channel transition resets the device.
    pub fn new(format: Format, model: Model) -> Result<Self> {
        if format.emphasis() {
            return Err("XA emphasis filtering is not implemented".into());
        }
        if format.sample_rate() == 18900 && model == Model::Published37800 {
            return Err(
                "published XA resampling model has no established 18.9 kHz algorithm".into(),
            );
        }
        Ok(Self {
            format,
            model,
            ring: [[0; 32]; 2],
            position: 0,
            phase: 6,
        })
    }
    pub fn pending_phase(&self) -> u8 {
        self.phase
    }

    fn change_format(&mut self, format: Format) -> Result<()> {
        if format == self.format {
            return Ok(());
        }
        if self.model != Model::EmulatorReference {
            return Err("XA coding transitions require the emulator-reference resampler".into());
        }
        if format.emphasis() {
            return Err("XA emphasis filtering not implemented".into());
        }
        // The reference device shares its ring, cursor and phase between
        // coding modes; changing the sector format is not a decoder reset.
        self.format = format;
        Ok(())
    }

    /// Preserve phase/history across chunks. No automatic end padding or tail
    /// flushing: the caller owns stream timing and subsequent input samples.
    pub fn process(&mut self, pcm: &[i16]) -> Result<Vec<[i16; 2]>> {
        let channels = usize::from(self.format.channels());
        if !pcm.len().is_multiple_of(channels) {
            return Err("XA resampling input ends within a stereo frame".into());
        }
        let frames = pcm.len() / channels;
        let mut output = Vec::new();
        if self.format.sample_rate() == 37800 {
            for frame in pcm.chunks_exact(channels) {
                self.ring[0][self.position] = frame[0];
                if channels == 2 {
                    self.ring[1][self.position] = frame[1];
                }
                self.position = (self.position + 1) & 31;
                self.phase -= 1;
                if self.phase != 0 {
                    continue;
                }
                self.phase = 6;
                for weights in ZIGZAG {
                    output.push(std::array::from_fn(|output_channel| {
                        let channel = if channels == 1 { 0 } else { output_channel };
                        let displacement = usize::from(self.model == Model::Published37800);
                        let sum: i32 = weights
                            .iter()
                            .enumerate()
                            .map(|(tap, &weight)| {
                                let sample = self.ring[channel]
                                    [(self.position + 64 - tap - displacement) & 31];
                                (i32::from(sample) * i32::from(weight)) >> 15
                            })
                            .sum();
                        clip(sum)
                    }));
                }
            }
        } else {
            let mut consumed = 0;
            while consumed < frames {
                if self.phase >= 7 {
                    self.phase -= 7;
                    self.position = (self.position + 1) & 31;
                    self.ring[0][self.position] = pcm[consumed * channels];
                    if channels == 2 {
                        self.ring[1][self.position] = pcm[consumed * channels + 1];
                    }
                    consumed += 1;
                }
                let weights = &HALF_RATE[usize::from(self.phase)];
                output.push(std::array::from_fn(|output_channel| {
                    let channel = if channels == 1 { 0 } else { output_channel };
                    let sum: i64 = weights
                        .iter()
                        .enumerate()
                        .map(|(tap, &weight)| {
                            i64::from(self.ring[channel][(self.position + 32 - 25 + tap) & 31])
                                * i64::from(weight)
                        })
                        .sum();
                    clip((sum >> 15) as i32)
                }));
                self.phase += 3;
            }
        }
        Ok(output)
    }
}

#[derive(Clone)]
pub struct XaAudio {
    decoder: Decoder,
    resampler: Resampler,
}
impl XaAudio {
    /// Runtime sectors may change coding without releasing file/channel.
    /// Rebinding identity requires an explicit selection release. Histories
    /// remain device-wide; mono leaves the right-channel history untouched.
    pub(crate) fn select_stream(&mut self, stream: Stream, released: bool) -> Result<()> {
        let old = self.decoder.stream();
        if !released && [stream.file, stream.channel] != [old.file, old.channel] {
            return Err("XA file/channel change requires selection release".into());
        }
        let decoder = Decoder::new(stream, self.decoder.arithmetic(), self.decoder.histories())?;
        self.resampler.change_format(decoder.format())?;
        self.decoder = decoder;
        Ok(())
    }

    pub fn new(
        stream: Stream,
        arithmetic: Arithmetic,
        histories: Histories,
        model: Model,
    ) -> Result<Self> {
        let decoder = Decoder::new(stream, arithmetic, histories)?;
        let resampler = Resampler::new(decoder.format(), model)?;
        Ok(Self { decoder, resampler })
    }
    /// Only already-selected sectors enter here. Decoder errors preserve both
    /// predictor and filter state; file/channel/coding switches require a new context.
    pub fn sector(&mut self, bytes: &[u8]) -> Result<Vec<[i16; 2]>> {
        let pcm = self.decoder.decode_sector(bytes)?;
        self.resampler.process(&pcm)
    }
    /// Emulator-reference mute boundary: decode predictor history, but leave
    /// interpolation state and queued output untouched.
    pub(crate) fn muted_sector(&mut self, bytes: &[u8]) -> Result<()> {
        self.decoder.decode_sector(bytes)?;
        Ok(())
    }
}

/// Drive attenuation, distinct from SPU CD gain. Ports use ATV0/1/2/3 order:
/// left->left, left->right, right->right, right->left.
pub struct Volume {
    pending: [u8; 4],
    active: [u8; 4],
    xa_muted: bool,
}
impl Volume {
    pub fn new(initial: [u8; 4]) -> Result<Self> {
        validate_gain(initial)?;
        Ok(Self {
            pending: initial,
            active: initial,
            xa_muted: false,
        })
    }
    pub fn active(&self) -> [u8; 4] {
        self.active
    }
    pub fn xa_muted(&self) -> bool {
        self.xa_muted
    }
    /// Bank/offset are already decoded by the CD host register owner.
    /// Only audio attenuation/control writes are implemented here.
    pub fn write(&mut self, bank: u8, offset: u8, value: u8) -> Result<()> {
        match (bank, offset) {
            (2, 2) => self.pending[0] = value,
            (2, 3) => self.pending[1] = value,
            (3, 1) => self.pending[2] = value,
            (3, 2) => self.pending[3] = value,
            (3, 3) => {
                if value & !0x21 != 0 {
                    return Err("CD audio control reserved bits are unsupported".into());
                }
                if value & 0x20 != 0 {
                    validate_gain(self.pending)?;
                    self.active = self.pending;
                }
                self.xa_muted = value & 1 != 0;
            }
            _ => return Err("unsupported CD audio register write".into()),
        }
        Ok(())
    }
    /// A supplied sample at the caller's output boundary. This does not choose
    /// whether mute changes decoder/FIFO state; the drive execution owner must.
    pub fn apply(&self, frame: [i16; 2], xa: bool) -> [i16; 2] {
        if xa && self.xa_muted {
            return [0; 2];
        }
        let part = |sample, volume| (i32::from(sample) * i32::from(volume)) >> 7;
        [
            clip(part(frame[0], self.active[0]) + part(frame[1], self.active[3])),
            clip(part(frame[0], self.active[1]) + part(frame[1], self.active[2])),
        ]
    }
}
fn validate_gain(values: [u8; 4]) -> Result<()> {
    if u16::from(values[0]) + u16::from(values[3]) > 256
        || u16::from(values[1]) + u16::from(values[2]) > 256
    {
        return Err("CD matrix gain above double volume has unverified hardware saturation".into());
    }
    Ok(())
}
fn clip(value: i32) -> i16 {
    value.clamp(-32768, 32767) as i16
}
