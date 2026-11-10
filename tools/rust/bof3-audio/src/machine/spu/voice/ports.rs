//! Voice MMIO latches; the execution owner explicitly schedules frame boundaries.
use crate::machine::{
    spu::noise::{Noise, State as NoiseState},
    spu::sample,
    spu::voice::envelope,
    spu::voice::{NativeCapture, NativeChannel, NativeInput, Output, Voices, VOICES},
};
use crate::Result;
use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum DisableModel {
    /// Native bit 15 writes preserve voice state and scheduled frames continue.
    /// This does not establish hardware disable behavior or write/sample latency.
    EmulatorReference,
}

pub struct Ports {
    voices: Voices,
    envelope_model: envelope::Model,
    on: u32,
    off: u32,
    key_latches: [u32; 2],
    end_readback: Option<u32>,
    modulation: u32,
    reverb: u32,
    active_reverb: u32,
    noise_mask: u32,
    noise: Option<Noise>,
    disable_model: Option<DisableModel>,
    current_volume_mirrors: [u16; VOICES * 2],
}

#[derive(Debug)]
pub struct Frame {
    /// Native bit-14 mute is already applied at ordinary sample stores.
    pub sample_model: sample::Model,
    pub voices: [Output; VOICES],
    /// Native sends latch at key-on; published sends use the live register.
    /// Modulation sources have zero stereo sends.
    pub reverb_mask: u32,
    /// Shared pre-envelope noise level used for this frame, when configured.
    pub noise_level: Option<i16>,
}

pub(crate) struct NativeSample {
    pub output: Output,
    /// Unclipped post-ADSR output for the shared modulation buffer.
    pub modulation: i32,
    pub modulation_consumed: bool,
    pub capture: NativeCapture,
    pub finished: bool,
    pub reverb_active: bool,
}

impl Ports {
    pub fn new(sample: sample::Model, envelope: envelope::Model) -> Self {
        Self {
            voices: Voices::new(sample, envelope),
            envelope_model: envelope,
            on: 0,
            off: 0,
            key_latches: [0; 2],
            end_readback: None,
            modulation: 0,
            reverb: 0,
            active_reverb: 0,
            noise_mask: 0,
            noise: None,
            disable_model: None,
            current_volume_mirrors: [0; VOICES * 2],
        }
    }

    pub(crate) fn sample_model(&self) -> sample::Model {
        self.voices.sample_model()
    }

    pub(crate) fn envelope_model(&self) -> envelope::Model {
        self.envelope_model
    }

    pub fn configure_key_on_delay(&mut self, frames: i32) -> Result<()> {
        self.voices.configure_key_on_delay(frames)
    }

    pub fn key_on_delay(&self) -> Option<i32> {
        self.voices.key_on_delay()
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
            .ok_or("SPU disable behavior requires an explicit evidence model")?;
        Ok(())
    }
    pub fn configure_noise(&mut self, noise: Noise) -> Result<()> {
        if self.noise.is_some() {
            return Err("SPU noise state already configured".into());
        }
        self.noise = Some(noise);
        Ok(())
    }

    pub(crate) fn require_native_context(&self) -> Result<()> {
        if self.voices.sample_model() != sample::Model::EmulatorReference
            || self.envelope_model != envelope::Model::EmulatorReference
        {
            return Err(
                "native channel processing requires native-reference samples and ADSR".into(),
            );
        }
        if !matches!(
            self.noise.as_ref().map(Noise::state),
            Some(NoiseState::EmulatorReference { .. })
        ) {
            return Err(
                "native channel processing requires explicit native-reference noise state".into(),
            );
        }
        Ok(())
    }

    /// Clock one prelude sample, including when no voice currently selects noise.
    pub(crate) fn native_noise_sample(&mut self, control: u16) -> Result<i16> {
        self.require_native_context()?;
        self.noise
            .as_mut()
            .ok_or("native channel processing requires explicit native-reference noise state")?
            .sample(((control >> 8) & 63) as u8)
    }

    /// Admit one channel's pending start and ordinary pitch before its sample loop.
    pub(crate) fn begin_native_channel(
        &mut self,
        index: usize,
        control: u16,
    ) -> Result<NativeChannel> {
        self.require_native_context()?;
        if index >= VOICES {
            return Err("SPU voice index outside 0..24".into());
        }
        let bit = 1 << index;
        let start = self.on & bit != 0;
        let channel = self
            .voices
            .begin_native_channel(index, self.noise_mask & bit != 0, start)?;
        if start {
            let armed = if control & 0x80 != 0 { self.reverb } else { 0 };
            self.active_reverb = (self.active_reverb & !bit) | (armed & bit);
            self.on &= !bit;
            self.off &= !bit;
        }
        self.end_readback = None;
        Ok(channel)
    }

    /// Consume one sample from the selected channel. Pending starts for all
    /// channels remain pending until their next channel-entry admission.
    pub(crate) fn native_channel_sample(
        &mut self,
        channel: &mut NativeChannel,
        ram: &[u8],
        control: u16,
        noise_level: i16,
        modulation: Option<i32>,
    ) -> Result<NativeSample> {
        self.require_native_context()?;
        let bit = 1 << channel.voice();
        let off = &self.off;
        let value = self.voices.native_channel_sample(
            channel,
            ram,
            NativeInput {
                control,
                noise_level,
                modulation,
            },
            || *off & bit != 0,
        )?;
        Ok(NativeSample {
            output: value.output,
            modulation: value.modulation,
            modulation_consumed: value.modulation_consumed,
            capture: value.capture,
            finished: value.finished,
            reverb_active: self.active_reverb & bit != 0,
        })
    }
    /// Relative to 0x1f801c00. Key reads expose the last register writes,
    /// not voice activity or pending keys consumed at a frame boundary.
    pub fn read(&self, offset: u32) -> Result<u16> {
        if (0x200..0x260).contains(&offset) && offset & 1 == 0 {
            if self.voices.sample_model() == sample::Model::EmulatorReference {
                return Ok(self.current_volume_mirrors[((offset - 0x200) / 2) as usize]);
            }
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
            if self.voices.sample_model() == sample::Model::EmulatorReference {
                self.current_volume_mirrors[((offset - 0x200) / 2) as usize] = value;
                return Ok(());
            }
            return Err("SPU current voice-volume writes are not hardware-verified".into());
        }
        if offset < 0x180 {
            if offset & 15 == 12 && self.voices.sample_model() == sample::Model::EmulatorReference {
                return Ok(());
            }
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
                if self.voices.sample_model() == sample::Model::EmulatorReference {
                    let mask = (u32::from(value) << shift) & 0x00ff_ffff;
                    if index == 0 {
                        self.on |= self.voices.accept_native_key_on(mask);
                    } else {
                        self.off |= mask;
                    }
                } else if index == 0 {
                    self.on = merge(self.on);
                } else {
                    self.off = merge(self.off);
                }
            }
            0x190 => {
                self.modulation = merge(self.modulation) & !1;
                self.voices.write_modulation(offset & 2 != 0, value);
            }
            0x194 => self.noise_mask = merge(self.noise_mask),
            0x198 => self.reverb = merge(self.reverb),
            // Native ENDX reads live flags and ignores writes. Published writes
            // retain their temporary readback without changing loop-end latches.
            0x19c => {
                if self.voices.sample_model() == sample::Model::Published {
                    self.end_readback = Some(merge(
                        self.end_readback.unwrap_or_else(|| self.voices.end_flags()),
                    ));
                }
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
        self.voices.require_key_on_delay()?;
        if self.noise_mask != 0 && self.noise.is_none() {
            return Err("SPU noise requires explicit model-qualified initial state".into());
        }
        let noise_level = self
            .noise
            .as_mut()
            .map(|noise| noise.sample(((control >> 8) & 63) as u8))
            .transpose()?;
        if self.voices.sample_model() == sample::Model::EmulatorReference {
            let armed = if control & 0x80 != 0 { self.reverb } else { 0 };
            self.active_reverb = (self.active_reverb & !self.on) | (armed & self.on);
        }
        match self.voices.sample_model() {
            sample::Model::Published => self.voices.apply_keys(self.on, self.off)?,
            sample::Model::EmulatorReference => self.voices.start_keys(self.on, self.off)?,
        }
        self.on = 0;
        self.off = 0;
        let voices = self.voices.tick(
            ram,
            noise_level.map(|level| (self.noise_mask, level)),
            control & 0x4000 != 0,
        )?;
        self.end_readback = None;
        Ok(Frame {
            sample_model: self.voices.sample_model(),
            voices,
            reverb_mask: match self.voices.sample_model() {
                sample::Model::Published => self.reverb,
                sample::Model::EmulatorReference => self.active_reverb,
            },
            noise_level,
        })
    }
}
