//! Scheduled voice register operations and 24 per-frame outputs, before final mix.
//! This component does not schedule MMIO, keys, capture, IRQs or reverb.
use super::{
    adsr::{Adsr, Model as EnvelopeModel},
    spu_sample::{Model as SampleModel, Player, Signal},
    spu_transfer::RAM_BYTES,
    spu_volume::VoiceVolume,
};
use crate::Result;

pub const VOICES: usize = 24;

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub enum ModulationRole {
    #[default]
    None,
    Target,
    Source,
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Output {
    /// Clipped post-envelope capture signal, retained for modulation sources.
    pub mono: i16,
    /// Per-voice sends stay wide until final mixing; sources send zero.
    pub stereo: [i32; 2],
    pub fetched_address: Option<u32>,
    /// End-flag comparison target before the fetched block updates repeat.
    pub fetched_loop_address: Option<u32>,
}

/// Dispatch choices retained while one native channel produces its 45 samples.
#[derive(Debug)]
pub(crate) struct NativeChannel {
    voice: usize,
    role: ModulationRole,
    noise: bool,
    silent: bool,
    terminated: bool,
    started: bool,
    installed_step: u32,
}

impl NativeChannel {
    pub(crate) fn voice(&self) -> usize {
        self.voice
    }

    pub(crate) fn role(&self) -> ModulationRole {
        self.role
    }

    pub(crate) fn started(&self) -> bool {
        self.started
    }

    pub(crate) fn installed_step(&self) -> u32 {
        self.installed_step
    }
}

pub(crate) struct NativeInput {
    pub control: u16,
    pub noise_level: i16,
    pub modulation: Option<i32>,
}

#[derive(Clone, Copy, Default)]
pub(crate) enum NativeCapture {
    #[default]
    Sample,
    DeferredSilence,
    SilenceAll,
    SilenceRemainder,
}

#[derive(Default)]
pub(crate) struct NativeOutput {
    pub output: Output,
    pub modulation: i32,
    pub modulation_consumed: bool,
    pub capture: NativeCapture,
    pub finished: bool,
}

#[derive(Clone, Debug)]
struct Voice {
    volumes: [VoiceVolume; 2],
    pitch: u16,
    start: u16,
    envelope: Adsr,
    player: Player,
    started: bool,
    startup_remaining: i32,
    native_channel_prepared: bool,
}

#[derive(Clone, Debug)]
pub struct Voices {
    voices: [Voice; VOICES],
    sample_model: SampleModel,
    end_flags: u32,
    modulation: [ModulationRole; VOICES],
    key_on_delay: Option<i32>,
}

impl Voices {
    /// Native-reference start and repeat addresses model a fresh native open,
    /// where RAM offset zero is valid before any guest register write.
    pub fn new(sample_model: SampleModel, envelope_model: EnvelopeModel) -> Self {
        Self {
            voices: std::array::from_fn(|_| Voice {
                volumes: [VoiceVolume::new(sample_model, envelope_model); 2],
                pitch: 0,
                start: 0,
                envelope: Adsr::fresh(envelope_model),
                player: Player::new(sample_model),
                started: false,
                startup_remaining: 0,
                native_channel_prepared: false,
            }),
            sample_model,
            end_flags: 0,
            modulation: [ModulationRole::None; VOICES],
            key_on_delay: None,
        }
    }

    pub fn end_flags(&self) -> u32 {
        self.end_flags
    }

    pub(crate) fn sample_model(&self) -> SampleModel {
        self.sample_model
    }

    pub fn configure_key_on_delay(&mut self, frames: i32) -> Result<()> {
        if self.sample_model != SampleModel::EmulatorReference {
            return Err("key-on delay requires native-reference voices".into());
        }
        if self.key_on_delay.is_some() {
            return Err("native key-on delay already configured".into());
        }
        self.key_on_delay = Some(frames);
        Ok(())
    }

    pub fn key_on_delay(&self) -> Option<i32> {
        self.key_on_delay
    }

    pub(crate) fn require_key_on_delay(&self) -> Result<i32> {
        if self.sample_model == SampleModel::Published {
            return Ok(0);
        }
        Ok(self
            .key_on_delay
            .ok_or("native SPU frames require an explicit key-on delay")?
            .max(0))
    }

    pub fn modulation_role(&self, index: usize) -> Result<ModulationRole> {
        self.modulation
            .get(index)
            .copied()
            .ok_or_else(|| "SPU voice index outside 0..24".into())
    }

    /// Apply one already scheduled PMON halfword write. Native assignments run
    /// in ascending order and can replace a preceding target with a source.
    pub fn write_modulation(&mut self, high: bool, value: u16) {
        let first = if high { 16 } else { 0 };
        let end = if high { VOICES } else { 16 };
        for index in first..end {
            if value & (1 << (index - first)) == 0 {
                self.modulation[index] = ModulationRole::None;
            } else if index != 0 {
                self.modulation[index] = ModulationRole::Target;
                if self.sample_model == SampleModel::EmulatorReference {
                    self.modulation[index - 1] = ModulationRole::Source;
                }
            }
        }
    }

    /// Apply already accepted component keys atomically. Native MMIO acceptance
    /// is separate. On wins if both bits are set; startup delay begins here.
    pub fn apply_keys(&mut self, on: u32, off: u32) -> Result<()> {
        self.require_key_on_delay()?;
        for (i, voice) in self.voices.iter_mut().enumerate() {
            if on & (1 << i) != 0 {
                voice.player.accept_key_on();
            }
        }
        self.end_flags &= !on;
        self.start_keys(on, off)
    }

    pub(crate) fn accept_native_key_on(&mut self, mask: u32) -> u32 {
        let mut accepted = 0;
        for (i, voice) in self.voices.iter_mut().enumerate() {
            let bit = 1 << i;
            if mask & bit != 0 {
                accepted |= bit;
                voice.player.accept_key_on();
            }
        }
        self.end_flags &= !accepted;
        accepted
    }

    pub(crate) fn start_keys(&mut self, on: u32, off: u32) -> Result<()> {
        let delay = self.require_key_on_delay()?;
        for (i, voice) in self.voices.iter_mut().enumerate() {
            let bit = 1 << i;
            if off & bit != 0 {
                voice.envelope.key_off();
            }
            if on & bit != 0 {
                voice.player.start(voice.start);
                voice.envelope.key_on();
                voice.started = true;
                voice.startup_remaining = delay;
            }
        }
        Ok(())
    }

    pub(crate) fn begin_native_channel(
        &mut self,
        index: usize,
        noise: bool,
        start: bool,
    ) -> Result<NativeChannel> {
        if self.sample_model != SampleModel::EmulatorReference {
            return Err("native channels require native-reference samples".into());
        }
        let delay = self.require_key_on_delay()?;
        let voice = self
            .voices
            .get_mut(index)
            .ok_or("SPU voice index outside 0..24")?;
        if voice.envelope.model() != EnvelopeModel::EmulatorReference {
            return Err("native channels require native-reference ADSR".into());
        }
        if !voice.native_channel_prepared {
            if voice.started {
                return Err(
                    "native channels must be selected before functional voice execution".into(),
                );
            }
            voice.player.prepare_native_channel()?;
            voice.native_channel_prepared = true;
        }
        let role = self.modulation[index];
        if start {
            voice.player.start(voice.start);
            voice.envelope.key_on();
            voice.started = true;
            voice.startup_remaining = delay;
        }
        let silent = voice.envelope.phase() == super::adsr::Phase::Off;
        if !silent {
            voice.player.install_pending_frequency();
        }
        Ok(NativeChannel {
            voice: index,
            role,
            noise,
            silent,
            terminated: false,
            started: start,
            installed_step: voice.player.pitch().step(),
        })
    }

    /// Consume one selected channel sample. Stop, ADSR controls and dry gains
    /// remain live; dispatch roles and the entry on/off branch stay latched.
    pub(crate) fn native_channel_sample(
        &mut self,
        channel: &mut NativeChannel,
        ram: &[u8],
        input: NativeInput,
        stop: impl FnOnce() -> bool,
    ) -> Result<NativeOutput> {
        if ram.len() != RAM_BYTES {
            return Err("SPU voices require exactly 512 KiB RAM".into());
        }
        if channel.terminated {
            return Err("native channel already completed".into());
        }
        let index = channel.voice;
        let voice = &mut self.voices[index];
        if !channel.silent && voice.startup_remaining > 0 {
            voice.startup_remaining -= 1;
            return Ok(NativeOutput::default());
        }
        let source = channel.role == ModulationRole::Source;
        let modulation_consumed = !channel.silent
            && channel.role == ModulationRole::Target
            && input.modulation.is_some_and(|value| value != 0);
        let frame = if channel.silent {
            voice.player.tick_silent(ram)
        } else {
            voice.player.native_channel_sample(
                ram,
                if channel.role == ModulationRole::Target {
                    input.modulation
                } else {
                    None
                },
                if source {
                    Signal::ModulationSource
                } else {
                    Signal::Ordinary {
                        unmuted: input.control & 0x4000 != 0,
                    }
                },
            )
        }
        .map_err(|error| format!("SPU voice {index}: {error}"))?;
        if frame.loop_end {
            self.end_flags |= 1 << index;
        }
        let mut output = Output {
            fetched_address: frame.fetched_address,
            fetched_loop_address: frame.fetched_loop_address,
            ..Output::default()
        };
        if frame.stopped {
            channel.terminated = true;
            if !channel.silent {
                voice.envelope.force_off();
            }
            return Ok(NativeOutput {
                output,
                modulation_consumed,
                capture: if channel.silent {
                    NativeCapture::SilenceAll
                } else {
                    NativeCapture::SilenceRemainder
                },
                finished: true,
                ..NativeOutput::default()
            });
        }
        if channel.silent {
            return Ok(NativeOutput {
                output,
                capture: NativeCapture::DeferredSilence,
                ..NativeOutput::default()
            });
        }
        if stop() {
            voice.envelope.key_off();
        }
        let raw = if channel.noise {
            i32::from(input.noise_level)
        } else {
            frame.sample
        };
        let sample = raw
            .checked_mul(i32::from(voice.envelope.sample()))
            .ok_or_else(|| {
                format!("SPU voice {index}: envelope product exceeds signed 32-bit domain")
            })?
            >> 15;
        output.mono = sample.clamp(i32::from(i16::MIN), i32::from(i16::MAX)) as i16;
        if source {
            return Ok(NativeOutput {
                output,
                modulation: sample,
                ..NativeOutput::default()
            });
        }
        output.stereo = [
            voice.volumes[0].apply(sample)?,
            voice.volumes[1].apply(sample)?,
        ];
        Ok(NativeOutput {
            output,
            modulation_consumed,
            ..NativeOutput::default()
        })
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
            4 => {
                voice.pitch = value;
                voice.player.write_pitch(value);
            }
            6 => voice.start = value,
            8 => voice.envelope.write_adsr1(value),
            10 => voice.envelope.write_adsr2(value),
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
    pub fn tick(
        &mut self,
        ram: &[u8],
        noise: Option<(u32, i16)>,
        unmuted: bool,
    ) -> Result<[Output; VOICES]> {
        self.require_key_on_delay()?;
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
            if self.sample_model == SampleModel::EmulatorReference
                && voice.envelope.phase() == super::adsr::Phase::Off
            {
                let frame = voice
                    .player
                    .tick_silent(ram)
                    .map_err(|e| format!("SPU voice {i}: {e}"))?;
                if frame.loop_end {
                    self.end_flags |= 1 << i;
                }
                output[i].fetched_address = frame.fetched_address;
                output[i].fetched_loop_address = frame.fetched_loop_address;
                preceding = 0;
                continue;
            }
            if voice.startup_remaining > 0 {
                voice.player.install_pending_frequency();
                voice.startup_remaining -= 1;
                preceding = 0;
                continue;
            }
            let role = self.modulation[i];
            let modulation = (role == ModulationRole::Target).then_some(preceding);
            let signal = if role == ModulationRole::Source {
                Signal::ModulationSource
            } else {
                Signal::Ordinary { unmuted }
            };
            let frame = voice
                .player
                .tick(ram, modulation, signal)
                .map_err(|e| format!("SPU voice {i}: {e}"))?;
            if frame.loop_end {
                self.end_flags |= 1 << i;
            }
            if frame.stopped {
                voice.envelope.force_off();
                output[i].fetched_address = frame.fetched_address;
                output[i].fetched_loop_address = frame.fetched_loop_address;
                preceding = 0;
                continue;
            }
            let noise_sample = noise
                .filter(|(mask, _)| mask & (1 << i) != 0)
                .map(|(_, level)| level);
            let raw = noise_sample.map(i32::from).unwrap_or(frame.sample);
            let signal = raw
                .checked_mul(i32::from(voice.envelope.sample()))
                .ok_or_else(|| {
                    format!("SPU voice {i}: envelope product exceeds signed 32-bit domain")
                })?
                >> 15;
            let mono = signal.clamp(i32::from(i16::MIN), i32::from(i16::MAX)) as i16;
            output[i] = Output {
                mono,
                stereo: if role == ModulationRole::Source {
                    [0, 0]
                } else {
                    [
                        voice.volumes[0].apply(signal)?,
                        voice.volumes[1].apply(signal)?,
                    ]
                },
                fetched_address: frame.fetched_address,
                fetched_loop_address: frame.fetched_loop_address,
            };
            preceding = match self.sample_model {
                SampleModel::Published => i32::from(mono),
                SampleModel::EmulatorReference if role == ModulationRole::Source => signal,
                SampleModel::EmulatorReference => 0,
            };
            for volume in &mut voice.volumes {
                volume.tick();
            }
            if frame.mute && noise_sample.is_none() {
                voice.envelope.key_off();
                voice.envelope.write_level(0);
            }
        }
        Ok(output)
    }
}
