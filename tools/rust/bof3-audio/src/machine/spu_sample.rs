//! Frame-driven SPU sample reader. RAM fetch timing and IRQ comparison are separate.
use super::{gaussian::GAUSS, spu_transfer::RAM_BYTES};
use crate::{codec::adpcm::History, Result};
use serde::{Deserialize, Serialize};

/// Sources disagree on rounding and the pitch ceiling. Selection is mandatory;
/// neither model has independently recorded hardware acceptance in this project.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Model {
    /// Round each interpolation product separately; pitch ceiling 0x4000.
    Published,
    /// Round the sum once; pitch ceiling 0x3fff (inspected DuckStation behavior).
    EmulatorReference,
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
        Model::EmulatorReference => products.sum::<i64>() >> 15,
    };
    value.clamp(i64::from(i16::MIN), i64::from(i16::MAX)) as i16
}

/// Modulation takes the preceding voice's signed post-ADSR, pre-pan output.
/// Voice zero's owner must pass None, regardless of PMON bit zero.
pub fn pitch_step(pitch: u16, preceding_voice: Option<i16>, model: Model) -> u16 {
    let step = preceding_voice.map_or(pitch, |level| {
        let factor = i64::from(level) + 0x8000;
        ((i64::from(pitch as i16) * factor) >> 15) as u16
    });
    step.min(match model {
        Model::Published => 0x4000,
        Model::EmulatorReference => 0x3fff,
    })
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Frame {
    pub sample: i16,
    /// Set after this frame consumes the end of a block carrying bit zero.
    pub loop_end: bool,
    /// End without repeat: the voice owner must zero its envelope after output.
    pub mute: bool,
    /// Byte address fetched at this frame boundary, for the future IRQ/timing owner.
    pub fetched_address: Option<u32>,
}

#[derive(Clone, Debug)]
pub struct Player {
    model: Model,
    address: u16,
    repeat: u16,
    counter: u32,
    history: History,
    samples: [i16; 31],
    flags: u8,
    loaded: bool,
}

impl Player {
    pub fn new(model: Model) -> Self {
        Self {
            model,
            address: 0,
            repeat: 0,
            counter: 0,
            history: History::default(),
            samples: [0; 31],
            flags: 0,
            loaded: false,
        }
    }
    /// Apply an already scheduled key-on. Low address bit is ignored for blocks.
    /// Key-on preserves the repeat register but clears both predictor histories.
    pub fn key_on(&mut self, start_address: u16) {
        self.address = start_address & !1;
        self.counter = 0;
        self.history = History::default();
        self.samples = [0; 31];
        self.flags = 0;
        self.loaded = false;
    }
    pub fn repeat_address(&self) -> u16 {
        self.repeat
    }
    pub fn write_repeat_address(&mut self, value: u16) {
        self.repeat = value;
    }
    pub fn current_address(&self) -> u16 {
        self.address
    }
    pub fn counter(&self) -> u32 {
        self.counter
    }
    pub fn history(&self) -> History {
        self.history
    }

    /// Render one 44.1 kHz frame. Fetches snapshot whole ADPCM blocks; concurrent
    /// RAM writes, bus arbitration, IRQs and key-on latency are not modeled here.
    /// Invalid blocks fail before player state changes; the caller must stop.
    pub fn tick(&mut self, ram: &[u8], pitch: u16, preceding_voice: Option<i16>) -> Result<Frame> {
        if ram.len() != RAM_BYTES {
            return Err("SPU sample reader requires exactly 512 KiB RAM".into());
        }
        let fetched_address = if !self.loaded {
            let address = usize::from(self.address) * 8;
            let block: &[u8; 16] = ram[address..address + 16].try_into()?;
            let decoded = self
                .history
                .decode_block(block)
                .map_err(|error| format!("SPU sample block at byte 0x{address:05X}: {error}"))?;
            self.samples.copy_within(28..31, 0);
            self.samples[3..].copy_from_slice(&decoded);
            self.flags = block[1];
            if self.flags & 4 != 0 {
                self.repeat = self.address;
            }
            self.loaded = true;
            Some(address as u32)
        } else {
            None
        };
        let index = (self.counter >> 12) as usize;
        let sample = interpolate(
            self.samples[index..index + 4].try_into()?,
            (self.counter >> 4) as u8,
            self.model,
        );
        self.counter += u32::from(pitch_step(pitch, preceding_voice, self.model));
        let loop_end = self.counter >= 28 * 0x1000 && self.flags & 1 != 0;
        if self.counter >= 28 * 0x1000 {
            self.counter -= 28 * 0x1000;
            self.address = if loop_end {
                self.repeat & !1
            } else {
                self.address.wrapping_add(2)
            };
            self.loaded = false;
        }
        Ok(Frame {
            sample,
            loop_end,
            mute: loop_end && self.flags & 2 == 0,
            fetched_address,
        })
    }
}
