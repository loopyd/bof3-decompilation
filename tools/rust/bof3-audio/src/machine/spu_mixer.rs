//! Integer SPU mixer routing, before/after the separately owned reverb engine.
//! Ordering follows the inspected emulator; hardware acceptance remains open.
use super::{adsr::Model, spu_voice_ports::Frame, spu_volume::Volume};
use crate::Result;

#[derive(Clone, Copy, Debug, Default)]
pub struct Inputs {
    /// Already at 44.1 kHz after drive-side volume/resampling, before SPU gain.
    pub cd: [i16; 2],
    pub external: [i16; 2],
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Sends {
    pub dry: [i32; 2],
    pub reverb: [i16; 2],
}

#[derive(Clone, Debug, Default)]
pub struct Mixer {
    main: [Volume; 2],
    reverb: [i16; 2],
    cd: [i16; 2],
    external: [i16; 2],
}

impl Mixer {
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
            0x180 => self.main[channel].register(),
            0x184 => self.reverb[channel] as u16,
            0x1b0 => self.cd[channel] as u16,
            0x1b4 => self.external[channel] as u16,
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
                self.main[channel].write(value)?;
            }
            0x184 => self.reverb[channel] = value as i16,
            0x1b0 => self.cd[channel] = value as i16,
            0x1b4 => self.external[channel] = value as i16,
            0x1b8 => self.main[channel].write_level(value),
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

    pub fn prepare(&self, voices: &Frame, control: u16, inputs: Inputs) -> Sends {
        let mut dry = [0i32; 2];
        let mut wet = [0i32; 2];
        if control & 0xc000 == 0xc000 {
            for (index, voice) in voices.voices.iter().enumerate() {
                for channel in 0..2 {
                    dry[channel] += i32::from(voice.stereo[channel]);
                    if voices.reverb_mask & (1 << index) != 0 {
                        wet[channel] += i32::from(voice.stereo[channel]);
                    }
                }
            }
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
        Sends {
            dry,
            reverb: wet.map(clamp),
        }
    }

    /// Reverb return is the unscaled stereo output of that engine. Clamp the
    /// accumulated mix before main gain. The sole +32768 full-negative-gain rail
    /// is unverified on hardware and fails explicitly rather than inventing wrap.
    pub fn finish(&mut self, sends: Sends, reverb_return: [i16; 2]) -> Result<[i16; 2]> {
        let mut result = [0; 2];
        for channel in 0..2 {
            let sum = i64::from(sends.dry[channel])
                + i64::from(scale(reverb_return[channel], self.reverb[channel]));
            let value = scale(sum.clamp(-32768, 32767) as i16, self.main[channel].level());
            result[channel] = i16::try_from(value).map_err(|_| {
                "SPU final +32768 gain rail has unverified wrap/saturation behavior"
            })?;
        }
        for volume in &mut self.main {
            volume.tick();
        }
        Ok(result)
    }
}

fn scale(sample: i16, gain: i16) -> i32 {
    (i32::from(sample) * i32::from(gain)) >> 15
}
fn clamp(value: i32) -> i16 {
    value.clamp(-32768, 32767) as i16
}
