//! CD-to-SPU sample processing at caller-selected boundaries; no drive scheduler.
use super::xa_filter::ZIGZAG;
use crate::{
    xa::{Arithmetic, Decoder, Format, Histories, Stream},
    Result,
};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Model {
    /// Published 37.8 kHz indexing. No published 18.9 kHz algorithm is established.
    Published37800,
    /// Native ring alignment, with midpoint insertion for 18.9 kHz input.
    EmulatorReference,
}

#[derive(Clone)]
pub struct Resampler {
    format: Format,
    model: Model,
    ring: [[i16; 32]; 2],
    position: usize,
    phase: u8,
    previous: [i16; 2],
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
            previous: [0; 2],
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
        let mut output = Vec::new();
        for frame in pcm.chunks_exact(channels) {
            let stereo = [frame[0], frame[channels - 1]];
            if self.format.sample_rate() == 18900 {
                let midpoint = std::array::from_fn(|channel| {
                    ((i32::from(self.previous[channel]) + i32::from(stereo[channel])) >> 1) as i16
                });
                self.push(midpoint, &mut output);
            }
            self.push(stereo, &mut output);
            self.previous = stereo;
        }
        Ok(output)
    }

    fn push(&mut self, frame: [i16; 2], output: &mut Vec<[i16; 2]>) {
        for (ring, sample) in self.ring.iter_mut().zip(frame) {
            ring[self.position] = sample;
        }
        self.position = (self.position + 1) & 31;
        self.phase -= 1;
        if self.phase != 0 {
            return;
        }
        self.phase = 6;
        for weights in ZIGZAG {
            output.push(std::array::from_fn(|channel| {
                let sum: i32 = weights
                    .iter()
                    .enumerate()
                    .map(|(tap, &weight)| {
                        let sample = self.ring[channel][(self.position + 63 - tap) & 31];
                        (i32::from(sample) * i32::from(weight)) >> 15
                    })
                    .sum();
                clip(sum)
            }));
        }
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
    /// Native XA gain at sector admission, after interpolation. The queue
    /// owner handles mute before interpolation; queued samples retain this gain.
    pub fn apply_xa(&self, frame: [i16; 2]) -> [i16; 2] {
        self.apply_matrix(frame, 33617)
    }

    /// Native CDDA attenuation before its 44.1 kHz bypass. Command mute belongs
    /// to the queue owner; the XA-only mute bit does not suppress CDDA.
    pub fn apply_cdda(&self, frame: [i16; 2]) -> [i16; 2] {
        self.apply_matrix(frame, 31130)
    }

    fn apply_matrix(&self, frame: [i16; 2], coefficient: i64) -> [i16; 2] {
        let part = |sample, volume| i64::from(sample) * i64::from(volume);
        let scale = |sum: i64| {
            let clipped = ((sum * coefficient) >> 22).clamp(-32768, 32767);
            ((clipped * 31880) >> 15) as i16
        };
        [
            scale(part(frame[0], self.active[0]) + part(frame[1], self.active[3])),
            scale(part(frame[0], self.active[1]) + part(frame[1], self.active[2])),
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
