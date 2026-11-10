//! SPU stream routing, integer gain and explicit native float output.
//! Ordering follows the inspected emulator; hardware acceptance remains open.
use crate::machine::{
    spu::sample::Model as SampleModel, spu::voice::envelope::Model, spu::voice::ports::Frame,
    spu::volume::Volume,
};
use crate::Result;
use serde::{Deserialize, Serialize};

/// Fixed host settings for an explicitly selected native-reference context.
#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct NativeSettings {
    pub volume: u8,
    pub mono: bool,
    pub muted: bool,
}

impl NativeSettings {
    fn divisor(self) -> Result<i32> {
        if self.volume > 3 {
            return Err("native SPU host volume must be in 0..=3".into());
        }
        Ok(4 - i32::from(self.volume))
    }
}

#[derive(Clone, Copy, Debug, Default)]
pub struct Inputs {
    /// Already at 44.1 kHz after drive-side volume/resampling, before SPU gain.
    pub cd: [i16; 2],
    pub external: [i16; 2],
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Sends {
    pub dry: [i32; 2],
    pub reverb: [i32; 2],
    /// Native CD stream before host division; published CD is already in dry.
    pub cd: [i16; 2],
}

#[derive(Clone, Debug, Default)]
pub struct Mixer {
    main: [Volume; 2],
    native_main_registers: [u16; 2],
    native_main_levels: [i16; 2],
    native: Option<NativeSettings>,
    reverb: [i16; 2],
    cd: [i16; 2],
    external: [i16; 2],
}

impl Mixer {
    pub fn configure_native(&mut self, settings: NativeSettings) -> Result<()> {
        settings.divisor()?;
        if self.native.is_some() {
            return Err("native SPU output settings already configured".into());
        }
        self.native = Some(settings);
        Ok(())
    }

    pub fn native_settings(&self) -> Option<NativeSettings> {
        self.native
    }

    pub fn require_model(&self, sample: SampleModel) -> Result<()> {
        if (sample == SampleModel::EmulatorReference) != self.native.is_some() {
            return Err("SPU output policy must match the configured sample model; native output requires explicit host settings".into());
        }
        Ok(())
    }

    pub fn configure_sweeps(&mut self, model: Model) -> Result<()> {
        for volume in &mut self.main {
            volume.configure(model)?;
        }
        Ok(())
    }
    /// Offsets relative to 0x1f801c00.
    pub fn read(&self, offset: u32) -> Result<u16> {
        let channel = ((offset & 2) / 2) as usize;
        if offset & 1 != 0 {
            return Err("unaligned SPU mixer register".into());
        }
        Ok(match offset & !2 {
            0x180 if self.native.is_some() => self.native_main_registers[channel],
            0x180 => self.main[channel].register(),
            0x184 => self.reverb[channel] as u16,
            0x1b0 => self.cd[channel] as u16,
            0x1b4 => self.external[channel] as u16,
            0x1b8 if self.native.is_some() => self.native_main_levels[channel] as u16,
            0x1b8 => self.main[channel].level() as u16,
            _ => return Err(format!("unsupported SPU mixer register {offset:#x}").into()),
        })
    }

    /// Apply an already scheduled register write.
    pub fn write(&mut self, offset: u32, value: u16) -> Result<()> {
        let channel = ((offset & 2) / 2) as usize;
        if offset & 1 != 0 {
            return Err("unaligned SPU mixer register".into());
        }
        match offset & !2 {
            0x180 => {
                if self.native.is_some() {
                    self.native_main_registers[channel] = value;
                    if value & 0x8000 == 0 {
                        self.native_main_levels[channel] = (value << 1) as i16;
                    }
                } else {
                    self.main[channel].write(value)?;
                }
            }
            0x184 => self.reverb[channel] = value as i16,
            0x1b0 => self.cd[channel] = value as i16,
            0x1b4 => self.external[channel] = value as i16,
            0x1b8 => {
                if self.native.is_some() {
                    self.native_main_levels[channel] = value as i16;
                } else {
                    self.main[channel].write_level(value);
                }
            }
            _ => return Err(format!("unsupported SPU mixer register {offset:#x}").into()),
        }
        Ok(())
    }

    /// Disabled reverb can still emit its stored tail. Dry rendering is valid
    /// only with both wet gains zero and reverb RAM writes disabled.
    pub fn require_dry(&self, control: u16) -> Result<()> {
        if control & 0x80 != 0 || self.reverb != [0, 0] {
            return Err("SPU reverb engine is not configured; enabled reverb or audible tail cannot be discarded".into());
        }
        Ok(())
    }

    pub fn prepare(&self, voices: &Frame, control: u16, inputs: Inputs) -> Result<Sends> {
        self.require_model(voices.sample_model)?;
        if self.native.is_some() && inputs.external != [0; 2] {
            return Err("native SPU output has no external input source".into());
        }
        let mut dry = [0i32; 2];
        let mut wet = [0i32; 2];
        if voices.sample_model == SampleModel::EmulatorReference || control & 0x4000 != 0 {
            for (index, voice) in voices.voices.iter().enumerate() {
                for channel in 0..2 {
                    dry[channel] = dry[channel]
                        .checked_add(voice.stereo[channel])
                        .ok_or("SPU dry voice sum exceeds signed 32-bit domain")?;
                    if voices.reverb_mask & (1 << index) != 0 {
                        wet[channel] = wet[channel]
                            .checked_add(voice.stereo[channel])
                            .ok_or("SPU reverb voice sum exceeds signed 32-bit domain")?;
                    }
                }
            }
        }
        if self.native.is_some() {
            return Ok(Sends {
                dry,
                reverb: wet,
                cd: inputs.cd,
            });
        }
        for (enable, reverb, samples, gain) in [
            (1, 4, inputs.cd, self.cd),
            (2, 8, inputs.external, self.external),
        ] {
            if control & enable != 0 {
                for channel in 0..2 {
                    let sample = scale(samples[channel], gain[channel]);
                    dry[channel] += sample;
                    if control & reverb != 0 {
                        wet[channel] += sample;
                    }
                }
            }
        }
        Ok(Sends {
            dry,
            reverb: wet,
            cd: [0; 2],
        })
    }

    pub fn reverb_gain(&self) -> [i16; 2] {
        self.reverb
    }

    /// Complete the producer's signed16 voice stream. Callback mute, mono and
    /// the independently queued CD stream are applied only by its consumer.
    pub(crate) fn finish_native_voice(
        &self,
        sends: Sends,
        reverb_return: [i32; 2],
    ) -> Result<[i16; 2]> {
        let divisor = self
            .native
            .ok_or("native SPU producer requires explicit mixer settings")?
            .divisor()?;
        let mut result = [0; 2];
        for channel in 0..2 {
            let sum = sends.dry[channel]
                .checked_add(reverb_return[channel])
                .ok_or("native SPU final sum exceeds signed 32-bit domain")?;
            result[channel] = (sum / divisor).clamp(-32767, 32767) as i16;
        }
        Ok(result)
    }

    /// Reverb already includes wet gain. Published output clips before Q15 main
    /// gain; native output divides and symmetrically clips the SPU stream, then
    /// adds the separate CD stream as float samples without a final clamp.
    pub fn finish(&mut self, sends: Sends, reverb_return: [i32; 2]) -> Result<[f32; 2]> {
        if self.native.is_some() {
            let voice = self.finish_native_voice(sends, reverb_return)?;
            return self.finish_native_callback(voice, sends.cd);
        }
        let mut result = [0; 2];
        for channel in 0..2 {
            let sum = i64::from(sends.dry[channel]) + i64::from(reverb_return[channel]);
            let value = scale(sum.clamp(-32768, 32767) as i16, self.main[channel].level());
            result[channel] = i16::try_from(value).map_err(|_| {
                "SPU final +32768 gain rail has unverified wrap/saturation behavior"
            })?;
        }
        for volume in &mut self.main {
            volume.tick();
        }
        Ok(result.map(|sample| f32::from(sample) / 32767.0))
    }

    pub(crate) fn finish_native_callback(&self, voice: [i16; 2], cd: [i16; 2]) -> Result<[f32; 2]> {
        let settings = self
            .native
            .ok_or("native callback requires explicit mixer settings")?;
        let divisor = settings.divisor()?;
        let mut result = [0.0; 2];
        if !settings.muted {
            for channel in 0..2 {
                let cd = i32::from(cd[channel]) / divisor;
                let sum = f32::from(voice[channel]) / 32767.0 + cd as f32 / 32767.0;
                result[channel] = sum * (f32::from(self.native_main_levels[channel]) / 32768.0);
            }
        }
        if settings.mono {
            result = [(result[0] + result[1]) * 0.5; 2];
        }
        Ok(result)
    }
}

fn scale(sample: i16, gain: i16) -> i32 {
    (i32::from(sample) * i32::from(gain)) >> 15
}
