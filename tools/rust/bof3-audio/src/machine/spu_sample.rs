//! Frame-driven SPU sample reader. The interconnect owns fetch IRQ comparisons.
use super::{gaussian::GAUSS, spu_pitch::Pitch, spu_transfer::RAM_BYTES};
use crate::{
    codec::adpcm::{History, NativeHistory},
    Result,
};
use serde::{Deserialize, Serialize};

/// Select reader arithmetic, voice gain, modulation routing and mute policy. Compare with
/// PCSX-Redux Agentic; neither model establishes hardware fidelity.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Model {
    /// Round each interpolation product separately; pitch ceiling 0x4000.
    Published,
    /// Current native Gaussian guard-bit rounding; raw pitch ceiling 0x3fff.
    EmulatorReference,
}

/// Predictor state retains the selected model's width without conversion.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum DecoderState {
    Published(History),
    EmulatorReference(NativeHistory),
}

impl DecoderState {
    fn new(model: Model) -> Self {
        match model {
            Model::Published => Self::Published(History::default()),
            Model::EmulatorReference => Self::EmulatorReference(NativeHistory::default()),
        }
    }

    fn decode_block(&mut self, block: &[u8; 16]) -> Result<[i32; 28]> {
        match self {
            Self::Published(history) => Ok(history.decode_block(block)?.map(i32::from)),
            Self::EmulatorReference(history) => history.decode_block(block),
        }
    }
}

pub fn interpolate(samples: [i16; 4], phase: u8, model: Model) -> i16 {
    let i = usize::from(phase);
    let weights = [GAUSS[255 - i], GAUSS[511 - i], GAUSS[256 + i], GAUSS[i]];
    let products = samples
        .map(i64::from)
        .into_iter()
        .zip(weights)
        .map(|(sample, weight)| sample * i64::from(weight));
    let value = match model {
        Model::Published => products.map(|product| product >> 15).sum::<i64>(),
        Model::EmulatorReference => products.map(|product| product >> 14).sum::<i64>() >> 1,
    };
    value.clamp(i64::from(i16::MIN), i64::from(i16::MAX)) as i16
}

/// Scheduled native input routing. Published readers accept ordinary input only
/// and retain their existing interpolation policy regardless of bit-14 mute.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Signal {
    Ordinary { unmuted: bool },
    ModulationSource,
}

#[derive(Clone, Debug, Default)]
struct NativeWindow {
    samples: [i16; 4],
    ring: usize,
}

impl NativeWindow {
    fn store(&mut self, raw: i32, signal: Signal) {
        match signal {
            Signal::ModulationSource => {
                // The native raw source slot aliases the first two Gaussian
                // entries. Source stores preserve the ordinary ring index.
                self.samples[0] = raw as i16;
                self.samples[1] = (raw >> 16) as i16;
            }
            Signal::Ordinary { unmuted } => {
                self.samples[self.ring] = if unmuted {
                    raw.clamp(i32::from(i16::MIN), i32::from(i16::MAX)) as i16
                } else {
                    0
                };
                self.ring = (self.ring + 1) & 3;
            }
        }
    }

    fn sample(&self, phase: u8, signal: Signal) -> i32 {
        if signal == Signal::ModulationSource {
            return (u32::from(self.samples[0] as u16) | (u32::from(self.samples[1] as u16) << 16))
                as i32;
        }
        let window = std::array::from_fn(|i| self.samples[(self.ring + i) & 3]);
        i32::from(interpolate(window, phase, Model::EmulatorReference))
    }
}

#[derive(Clone, Debug)]
struct NativeReader {
    window: NativeWindow,
    position: u32,
    consumed: usize,
    ignore_loop: bool,
    ended: bool,
}

impl Default for NativeReader {
    fn default() -> Self {
        Self {
            window: NativeWindow::default(),
            position: 3 * 0x10000,
            consumed: 28,
            ignore_loop: false,
            ended: false,
        }
    }
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Frame {
    /// Ordinary interpolation is signed 16-bit; native sources retain raw i32.
    pub sample: i32,
    /// End-flag event: published block exit, native-reference block fetch.
    pub loop_end: bool,
    /// Published end without repeat: zero the envelope after ADPCM output.
    pub mute: bool,
    /// Native reader parked before output. Noise must also be suppressed.
    pub stopped: bool,
    /// Byte address fetched at this frame boundary.
    pub fetched_address: Option<u32>,
    /// Previous repeat byte address when the fetched block carries the end bit.
    /// Captured before a loop-start flag changes the repeat register.
    pub fetched_loop_address: Option<u32>,
}

#[derive(Clone, Debug)]
pub struct Player {
    model: Model,
    address: Option<u16>,
    repeat: u16,
    counter: u32,
    pitch: Pitch,
    history: DecoderState,
    samples: [i32; 31],
    native: Option<NativeReader>,
    flags: u8,
    loaded: bool,
}

impl Player {
    /// Native-reference addresses begin at RAM offset zero, as after native
    /// stream opening. This constructor does not import a saved native voice.
    pub fn new(model: Model) -> Self {
        Self {
            model,
            address: Some(0),
            repeat: 0,
            counter: 0,
            pitch: Pitch::new(model),
            history: DecoderState::new(model),
            samples: [0; 31],
            native: (model == Model::EmulatorReference).then(NativeReader::default),
            flags: 0,
            loaded: false,
        }
    }
    /// Apply an atomic component key-on. Both models ignore address bit 0,
    /// matching the native start-register handler's 16-byte alignment.
    /// Key-on preserves repeat and pitch state but clears predictor history.
    pub fn key_on(&mut self, start_address: u16) {
        self.accept_key_on();
        self.start(start_address);
    }

    pub(crate) fn accept_key_on(&mut self) {
        if let Some(native) = &mut self.native {
            native.ignore_loop = false;
        }
    }

    /// Start a previously accepted key without losing a later manual-loop write.
    pub(crate) fn start(&mut self, start_address: u16) {
        self.address = Some(start_address & !1);
        self.counter = match self.model {
            Model::Published => 0,
            Model::EmulatorReference => 2 * 0x10000,
        };
        self.history = DecoderState::new(self.model);
        self.samples = [0; 31];
        if let Some(native) = &mut self.native {
            let ignore_loop = native.ignore_loop;
            *native = NativeReader::default();
            native.ignore_loop = ignore_loop;
        }
        self.flags = 0;
        self.loaded = false;
    }
    pub fn repeat_address(&self) -> u16 {
        self.repeat
    }
    pub fn write_repeat_address(&mut self, value: u16) {
        self.repeat = value;
        if let Some(native) = &mut self.native {
            self.repeat &= !1;
            native.ignore_loop = true;
        }
    }
    /// Next native decode cursor, or the loaded published block address.
    /// None is the native parked cursor, even while its end block is buffered.
    pub fn current_address(&self) -> Option<u16> {
        self.address
    }
    /// Block-relative output position in 16.16 source-sample units.
    pub fn counter(&self) -> u32 {
        self.counter
    }
    pub fn history(&self) -> DecoderState {
        self.history
    }

    /// Apply an already scheduled pitch write before the selected frame.
    pub fn write_pitch(&mut self, value: u16) {
        self.pitch.write(value);
    }

    pub fn pitch(&self) -> Pitch {
        self.pitch
    }

    pub(crate) fn install_pending_frequency(&mut self) {
        self.pitch.install_pending_frequency();
    }

    /// Render one 44.1 kHz frame. Fetches snapshot whole ADPCM blocks; concurrent
    /// RAM writes, bus arbitration, IRQs and key-on latency are not modeled here.
    /// Invalid blocks fail before player state changes; the caller must stop.
    pub fn tick(
        &mut self,
        ram: &[u8],
        preceding_voice: Option<i32>,
        signal: Signal,
    ) -> Result<Frame> {
        if ram.len() != RAM_BYTES {
            return Err("SPU sample reader requires exactly 512 KiB RAM".into());
        }
        if self.model == Model::Published && signal == Signal::ModulationSource {
            return Err("raw modulation-source input requires the native-reference model".into());
        }
        if self.native.is_some() {
            return self.tick_native(ram, preceding_voice, signal);
        }
        let mut pitch = self.pitch;
        let step = pitch.tick(preceding_voice)?;
        let mut fetched_loop_address = None;
        let fetched_address = if !self.loaded {
            let current = self.address.ok_or("published sample cursor is parked")?;
            let address = usize::from(current) * 8;
            let block: &[u8; 16] = ram[address..address + 16].try_into()?;
            let decoded = self
                .history
                .decode_block(block)
                .map_err(|error| format!("SPU sample block at byte 0x{address:05X}: {error}"))?;
            self.samples.copy_within(28..31, 0);
            self.samples[3..].copy_from_slice(&decoded);
            self.flags = block[1];
            if self.flags & 1 != 0 {
                fetched_loop_address = Some(u32::from(self.repeat & !1) * 8);
            }
            if self.flags & 4 != 0 {
                self.repeat = current;
            }
            self.loaded = true;
            Some(address as u32)
        } else {
            None
        };
        let index = (self.counter >> 16) as usize;
        let phase = (self.counter >> 8) as u8;
        let sample = i32::from(interpolate(
            std::array::from_fn(|i| self.samples[index + i] as i16),
            phase,
            Model::Published,
        ));
        self.pitch = pitch;
        self.counter += step;
        let loop_end = self.counter >= 28 * 0x10000 && self.flags & 1 != 0;
        if self.counter >= 28 * 0x10000 {
            self.counter -= 28 * 0x10000;
            self.address = Some(if loop_end {
                self.repeat & !1
            } else {
                self.address
                    .ok_or("published sample cursor is parked")?
                    .wrapping_add(2)
            });
            self.loaded = false;
        }
        Ok(Frame {
            sample,
            loop_end,
            mute: loop_end && self.flags & 2 == 0,
            stopped: false,
            fetched_address,
            fetched_loop_address,
        })
    }

    fn tick_native(
        &mut self,
        ram: &[u8],
        preceding_voice: Option<i32>,
        signal: Signal,
    ) -> Result<Frame> {
        self.advance_native(ram, preceding_voice, Some(signal))
    }

    /// Released native voices discard samples while retaining installed pitch.
    pub(crate) fn tick_silent(&mut self, ram: &[u8]) -> Result<Frame> {
        self.advance_native(ram, None, None)
    }

    fn advance_native(
        &mut self,
        ram: &[u8],
        preceding_voice: Option<i32>,
        signal: Option<Signal>,
    ) -> Result<Frame> {
        let native = self.native.as_mut().unwrap();
        if native.ended {
            return Ok(Frame {
                stopped: true,
                ..Frame::default()
            });
        }
        let mut pitch = self.pitch;
        let step = match signal {
            Some(_) => pitch.tick(preceding_voice)?,
            None => pitch.step(),
        };

        // At most four samples are owed, so at most one new block is needed.
        // Validate it before consuming the previous tail to preserve failure
        // atomicity for invalid headers, straddling blocks and predictor overflow.
        let mut prepared = None;
        if (native.position >> 16) as usize > 28 - native.consumed {
            if let Some(current) = self.address {
                let address = usize::from(current) * 8;
                let block: &[u8; 16] = ram
                    .get(address..address + 16)
                    .ok_or_else(|| {
                        format!("native sample block at byte 0x{address:05X} straddles SPU RAM")
                    })?
                    .try_into()?;
                let mut history = self.history;
                let decoded = history.decode_block(block).map_err(|error| {
                    format!("SPU sample block at byte 0x{address:05X}: {error}")
                })?;
                prepared = Some((current, history, decoded, block[1]));
            }
        }

        self.pitch = pitch;
        let mut frame = Frame::default();
        while native.position >= 0x10000 {
            if native.consumed == 28 {
                let Some((current, history, decoded, flags)) = prepared.take() else {
                    native.ended = true;
                    frame.stopped = true;
                    return Ok(frame);
                };
                self.history = history;
                self.samples[3..].copy_from_slice(&decoded);
                native.consumed = 0;
                frame.fetched_address = Some(u32::from(current) * 8);
                frame.loop_end = flags & 1 != 0;
                if frame.loop_end {
                    frame.fetched_loop_address = Some(u32::from(self.repeat) * 8);
                }
                if flags & 4 != 0 && !native.ignore_loop {
                    self.repeat = current;
                }
                self.address = if frame.loop_end {
                    (flags == 3).then_some(self.repeat)
                } else {
                    Some(current.wrapping_add(2))
                };
            }
            if let Some(signal) = signal {
                native
                    .window
                    .store(self.samples[3 + native.consumed], signal);
            }
            native.consumed += 1;
            native.position -= 0x10000;
        }
        if let Some(signal) = signal {
            frame.sample = native.window.sample((native.position >> 8) as u8, signal);
        }
        native.position += step;
        self.counter = (self.counter + step) % (28 * 0x10000);
        Ok(frame)
    }
}
