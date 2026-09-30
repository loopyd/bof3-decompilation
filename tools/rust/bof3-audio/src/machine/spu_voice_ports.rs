//! Voice MMIO latches; the execution owner explicitly schedules frame boundaries.
use super::{
    adsr,
    spu_noise::Noise,
    spu_sample,
    spu_voices::{Output, Voices, VOICES},
};
use crate::Result;
use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum DisableModel {
    /// Falling SPUCNT bit 15 forces active envelopes off. Neither hardware
    /// write latency nor disabled-frame execution is established by this model.
    EmulatorReference,
}

pub struct Ports {
    voices: Voices,
    on: u32,
    off: u32,
    key_latches: [u32; 2],
    end_readback: Option<u32>,
    modulation: u32,
    reverb: u32,
    noise_mask: u32,
    noise: Option<Noise>,
    disable_model: Option<DisableModel>,
}

#[derive(Debug)]
pub struct Frame {
    pub voices: [Output; VOICES],
    /// Per-voice sends are retained for a future reverb mixer, not discarded.
    pub reverb_mask: u32,
    /// Shared pre-envelope noise level used for this frame, when configured.
    pub noise_level: Option<i16>,
}

impl Ports {
    pub fn new(sample: spu_sample::Model, envelope: adsr::Model) -> Self {
        Self {
            voices: Voices::new(sample, envelope),
            on: 0,
            off: 0,
            key_latches: [0; 2],
            end_readback: None,
            modulation: 0,
            reverb: 0,
            noise_mask: 0,
            noise: None,
            disable_model: None,
        }
    }

    pub fn configure_disable(&mut self, model: DisableModel) -> Result<()> {
        if self.disable_model.is_some() {
            return Err("SPU disable model already configured".into());
        }
        self.disable_model = Some(model);
        Ok(())
    }

    pub(crate) fn require_disable_model(&self) -> Result<()> {
        self.disable_model
            .ok_or("SPU disable transition requires an explicit evidence model")?;
        Ok(())
    }

    /// Call only after control-write validation, so rejected transfer/IRQ
    /// changes cannot partially silence the device. Pending keys stay pending.
    pub(crate) fn disable(&mut self) {
        match self.disable_model.expect("validated SPU disable model") {
            DisableModel::EmulatorReference => self.voices.disable_active_envelopes(),
        }
    }
    pub fn configure_noise(&mut self, noise: Noise) -> Result<()> {
        if self.noise.is_some() {
            return Err("SPU noise state already configured".into());
        }
        self.noise = Some(noise);
        Ok(())
    }
    /// Relative to 0x1f801c00. Key reads expose the last register writes,
    /// not voice activity or pending keys consumed at a frame boundary.
    pub fn read(&self, offset: u32) -> Result<u16> {
        if (0x200..0x260).contains(&offset) && offset & 1 == 0 {
            return self
                .voices
                .read_volume(((offset - 0x200) / 4) as usize, ((offset & 2) / 2) as usize);
        }
        if offset < 0x180 {
            return self.voices.read((offset / 16) as usize, offset & 15);
        }
        let flags = match offset & !2 {
            0x188 => self.key_latches[0],
            0x18c => self.key_latches[1],
            0x190 => self.modulation,
            0x194 => self.noise_mask,
            0x198 => self.reverb,
            0x19c => self.end_readback.unwrap_or_else(|| self.voices.end_flags()),
            _ => {
                return Err(
                    format!("SPU voice flag read at offset {offset:#x} is unsupported").into(),
                )
            }
        };
        if offset & 1 != 0 {
            return Err("unaligned SPU voice flag read".into());
        }
        Ok((flags >> ((offset & 2) * 8)) as u16)
    }
    pub fn write(&mut self, offset: u32, value: u16) -> Result<()> {
        if (0x200..0x260).contains(&offset) && offset & 1 == 0 {
            return Err("SPU current voice-volume writes are not hardware-verified".into());
        }
        if offset < 0x180 {
            return self
                .voices
                .write((offset / 16) as usize, offset & 15, value);
        }
        if offset & 1 != 0 {
            return Err("unaligned SPU voice flag write".into());
        }
        let shift = (offset & 2) * 8;
        let merge =
            |old: u32| ((old & !(0xffff << shift)) | (u32::from(value) << shift)) & 0x00ff_ffff;
        match offset & !2 {
            0x188 | 0x18c => {
                let index = ((offset - 0x188) / 4) as usize;
                self.key_latches[index] =
                    (self.key_latches[index] & !(0xffff << shift)) | (u32::from(value) << shift);
                if index == 0 {
                    self.on = merge(self.on);
                } else {
                    self.off = merge(self.off);
                }
            }
            0x190 => {
                self.modulation = merge(self.modulation) & !1;
                self.voices.set_modulation(self.modulation);
            }
            0x194 => self.noise_mask = merge(self.noise_mask),
            0x198 => self.reverb = merge(self.reverb),
            // ENDX writes can briefly read back but must not change the
            // voice loop-end latches. Our frame boundary refreshes readback;
            // sub-frame hardware overwrite timing remains unverified.
            0x19c => {
                self.end_readback = Some(merge(
                    self.end_readback.unwrap_or_else(|| self.voices.end_flags()),
                ));
            }
            _ => {
                return Err(
                    format!("SPU voice flag write at offset {offset:#x} is unsupported").into(),
                )
            }
        }
        Ok(())
    }
    /// Apply pending keys once at the caller-selected boundary, then step voices.
    /// This does not establish hardware key latency or minimum retrigger spacing.
    pub fn tick(&mut self, ram: &[u8], control: u16) -> Result<Frame> {
        if self.noise_mask != 0 && self.noise.is_none() {
            return Err(
                "SPU noise requires an explicit published-model initial level and timer".into(),
            );
        }
        let noise_level = self.noise.as_ref().map(Noise::level);
        self.voices.apply_keys(self.on, self.off);
        self.on = 0;
        self.off = 0;
        let voices = self
            .voices
            .tick(ram, noise_level.map(|level| (self.noise_mask, level)))?;
        self.end_readback = None;
        if let Some(noise) = &mut self.noise {
            noise.tick(((control >> 8) & 63) as u8)?;
        }
        Ok(Frame {
            voices,
            reverb_mask: self.reverb,
            noise_level,
        })
    }
}
