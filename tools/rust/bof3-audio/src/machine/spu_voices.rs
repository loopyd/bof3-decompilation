//! Scheduled voice register operations and 24 per-frame outputs, before final mix.
//! This component does not schedule MMIO, keys, capture, IRQs or reverb.
use super::{
    adsr::{Adsr, Model as EnvelopeModel, Registers},
    spu_sample::{Model as SampleModel, Player},
    spu_transfer::RAM_BYTES,
    spu_volume::Volume,
};
use crate::Result;

pub const VOICES: usize = 24;

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Output {
    /// Post-envelope signal, used to modulate the next voice before panning.
    pub mono: i16,
    pub stereo: [i16; 2],
    pub fetched_address: Option<u32>,
}

#[derive(Clone, Debug)]
struct Voice {
    volumes: [Volume; 2],
    pitch: u16,
    start: u16,
    envelope: Adsr,
    player: Player,
    started: bool,
}

#[derive(Clone, Debug)]
pub struct Voices {
    voices: [Voice; VOICES],
    sample_model: SampleModel,
    end_flags: u32,
    modulation: u32,
}

impl Voices {
    pub fn new(sample_model: SampleModel, envelope_model: EnvelopeModel) -> Self {
        Self {
            voices: std::array::from_fn(|_| Voice {
                volumes: [Volume::new(envelope_model); 2],
                pitch: 0,
                start: 0,
                envelope: Adsr::new(Registers { adsr1: 0, adsr2: 0 }, envelope_model),
                player: Player::new(sample_model),
                started: false,
            }),
            sample_model,
            end_flags: 0,
            modulation: 0,
        }
    }

    pub fn end_flags(&self) -> u32 {
        self.end_flags
    }

    /// Inspected emulator control-write behavior, not a hardware reset model.
    /// Only active envelopes are silenced. Preserve manual ENVX on an already
    /// off voice, sample position/history, gains, registers and ENDX.
    pub(crate) fn disable_active_envelopes(&mut self) {
        for voice in &mut self.voices {
            if voice.envelope.phase() != super::adsr::Phase::Off {
                voice.envelope.force_off();
            }
        }
    }
    pub fn set_modulation(&mut self, flags: u32) {
        self.modulation = flags & 0x00ff_fffe;
    }

    /// Apply already scheduled key masks. On wins if both bits are set. Latency
    /// and minimum retrigger spacing must be implemented by the scheduler.
    pub fn apply_keys(&mut self, on: u32, off: u32) {
        for (i, voice) in self.voices.iter_mut().enumerate() {
            let bit = 1 << i;
            if off & bit != 0 {
                voice.envelope.key_off();
            }
            if on & bit != 0 {
                voice.player.key_on(voice.start);
                voice.envelope.key_on();
                voice.started = true;
                self.end_flags &= !bit;
            }
        }
    }

    /// Halfword register offsets relative to this voice's sixteen-byte MMIO bank.
    pub fn read(&self, index: usize, offset: u32) -> Result<u16> {
        let voice = self
            .voices
            .get(index)
            .ok_or("SPU voice index outside 0..24")?;
        Ok(match offset {
            0 => voice.volumes[0].register(),
            2 => voice.volumes[1].register(),
            4 => voice.pitch,
            6 => voice.start,
            8 => voice.envelope.registers().adsr1,
            10 => voice.envelope.registers().adsr2,
            12 => voice.envelope.level() as u16,
            14 => voice.player.repeat_address(),
            _ => return Err(format!("SPU voice register offset {offset:#x} is invalid").into()),
        })
    }

    pub fn write(&mut self, index: usize, offset: u32, value: u16) -> Result<()> {
        let voice = self
            .voices
            .get_mut(index)
            .ok_or("SPU voice index outside 0..24")?;
        match offset {
            0 | 2 => {
                voice.volumes[(offset / 2) as usize].write(value)?;
            }
            4 => voice.pitch = value,
            6 => voice.start = value,
            8 | 10 => {
                let mut registers = voice.envelope.registers();
                if offset == 8 {
                    registers.adsr1 = value;
                } else {
                    registers.adsr2 = value;
                }
                voice.envelope.write_registers(registers);
            }
            12 => voice.envelope.write_level(value),
            14 => voice.player.write_repeat_address(value),
            _ => return Err(format!("SPU voice register offset {offset:#x} is invalid").into()),
        }
        Ok(())
    }

    pub fn read_volume(&self, index: usize, channel: usize) -> Result<u16> {
        Ok(self
            .voices
            .get(index)
            .and_then(|v| v.volumes.get(channel))
            .ok_or("SPU current-volume index outside 24 stereo voices")?
            .level() as u16)
    }
    /// Produce one frame for all voices. RAM is snapshotted at block fetches.
    /// No final mix is implied. Failure may follow earlier voices' state updates;
    /// the execution owner must stop, never retry this frame as if it was atomic.
    pub fn tick(&mut self, ram: &[u8], noise: Option<(u32, i16)>) -> Result<[Output; VOICES]> {
        if ram.len() != RAM_BYTES {
            return Err("SPU voices require exactly 512 KiB RAM".into());
        }
        let mut output = [Output::default(); VOICES];
        let mut preceding = 0;
        for (i, voice) in self.voices.iter_mut().enumerate() {
            if !voice.started {
                for volume in &mut voice.volumes {
                    volume.tick();
                }
                preceding = 0;
                continue;
            }
            let modulation = (self.modulation & (1 << i) != 0).then_some(preceding);
            let frame = voice
                .player
                .tick(ram, voice.pitch, modulation)
                .map_err(|e| format!("SPU voice {i}: {e}"))?;
            let noise_sample = noise
                .filter(|(mask, _)| mask & (1 << i) != 0)
                .map(|(_, level)| level);
            let mono = scale(noise_sample.unwrap_or(frame.sample), voice.envelope.level());
            output[i] = Output {
                mono,
                stereo: voice.volumes.map(|volume| scale(mono, volume.level())),
                fetched_address: frame.fetched_address,
            };
            preceding = mono;
            for volume in &mut voice.volumes {
                volume.tick();
            }
            voice.envelope.tick();
            if frame.loop_end {
                self.end_flags |= 1 << i;
            }
            if frame.mute && noise_sample.is_none() {
                match self.sample_model {
                    SampleModel::Published => {
                        voice.envelope.key_off();
                        voice.envelope.write_level(0);
                    }
                    SampleModel::EmulatorReference => voice.envelope.force_off(),
                }
            }
        }
        Ok(output)
    }
}

fn scale(sample: i16, level: i16) -> i16 {
    ((i32::from(sample) * i32::from(level)) >> 15).clamp(-32768, 32767) as i16
}
