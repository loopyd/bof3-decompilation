//! Scheduled fixed/swept SPU gain. Register bus latency remains caller-owned.
use crate::machine::spu::sample::Model as SampleModel;
use crate::machine::spu::voice::envelope::{Model, Rate};
use crate::Result;

/// Per-voice gain policy. Native-reference voices use the native fixed sweep
/// approximation; main-volume envelopes remain owned by `Volume` below.
#[derive(Clone, Copy, Debug)]
pub struct VoiceVolume {
    state: VoiceState,
}

#[derive(Clone, Copy, Debug)]
enum VoiceState {
    Published(Volume),
    EmulatorReference { register: u16, level: i16 },
}

impl VoiceVolume {
    pub fn new(sample: SampleModel, envelope: Model) -> Self {
        Self {
            state: match sample {
                SampleModel::Published => VoiceState::Published(Volume::new(envelope)),
                SampleModel::EmulatorReference => VoiceState::EmulatorReference {
                    register: 0,
                    level: 0,
                },
            },
        }
    }

    pub fn register(&self) -> u16 {
        match self.state {
            VoiceState::Published(volume) => volume.register(),
            VoiceState::EmulatorReference { register, .. } => register,
        }
    }

    /// Component current-level scale is Q15 for both policies. Native internal
    /// levels are Q14; this conversion does not establish native MMIO readback.
    pub fn level(&self) -> i16 {
        match self.state {
            VoiceState::Published(volume) => volume.level(),
            VoiceState::EmulatorReference { level, .. } => level * 2,
        }
    }

    pub fn write(&mut self, raw: u16) -> Result<()> {
        match &mut self.state {
            VoiceState::Published(volume) => volume.write(raw),
            VoiceState::EmulatorReference { register, level } => {
                *register = raw;
                *level = native_voice_level(raw);
                Ok(())
            }
        }
    }

    pub fn tick(&mut self) {
        if let VoiceState::Published(volume) = &mut self.state {
            volume.tick();
        }
    }

    pub fn apply(&self, sample: i32) -> Result<i32> {
        let (level, native) = match self.state {
            VoiceState::Published(volume) => (volume.level(), false),
            VoiceState::EmulatorReference { level, .. } => (level, true),
        };
        let sample = if native {
            sample
        } else {
            sample.clamp(-32768, 32767)
        };
        let product = sample
            .checked_mul(i32::from(level))
            .ok_or("SPU voice gain product exceeds signed 32-bit domain")?;
        Ok(if native {
            product / 0x4000
        } else {
            (product >> 15).clamp(-32768, 32767)
        })
    }
}

fn native_voice_level(raw: u16) -> i16 {
    if raw & 0x8000 == 0 {
        return ((raw & 0x3fff) as i16) - if raw & 0x4000 != 0 { 0x4000 } else { 0 };
    }
    let direction = if raw & 0x2000 != 0 { -1 } else { 1 };
    let phase = if raw & 0x1000 != 0 { !raw } else { raw };
    let step = ((phase & 127) + 1) as i16 / 2;
    (step + step / (2 * direction)) * 128
}

#[derive(Clone, Copy, Debug, Default)]
pub struct Volume {
    register: u16,
    level: i16,
    counter: u32,
    active: bool,
    model: Option<Model>,
}

impl Volume {
    pub fn new(model: Model) -> Self {
        Self {
            model: Some(model),
            ..Self::default()
        }
    }
    pub fn configure(&mut self, model: Model) -> Result<()> {
        if self.model.is_some() {
            return Err("SPU volume arithmetic already configured".into());
        }
        self.model = Some(model);
        Ok(())
    }
    pub fn register(&self) -> u16 {
        self.register
    }
    pub fn level(&self) -> i16 {
        self.level
    }
    pub fn counter(&self) -> u32 {
        self.counter
    }
    pub fn active(&self) -> bool {
        self.active
    }

    /// This is the effective register write, after any device-side delay.
    /// Sweep writes retain current gain and restart the rate counter.
    pub fn write(&mut self, raw: u16) -> Result<()> {
        if raw & 0x8000 != 0 && self.model.is_none() {
            return Err("SPU volume sweeps require an explicit envelope arithmetic model".into());
        }
        self.register = raw;
        self.counter = 0;
        self.active = raw & 0x8000 != 0 && raw & 127 != 127;
        if raw & 0x8000 == 0 {
            self.level = raw.wrapping_shl(1) as i16;
        }
        Ok(())
    }
    /// Direct current-volume writes do not rearm a stopped sweep.
    pub fn write_level(&mut self, raw: u16) {
        self.level = raw as i16;
    }

    /// One envelope clock, after the caller has used the current gain.
    pub fn tick(&mut self) {
        if !self.active {
            return;
        }
        let decreasing = self.register & 0x2000 != 0;
        let negative_phase = self.register & 0x1000 != 0;
        let rate = Rate {
            shift: ((self.register >> 2) & 31) as u8,
            step: (self.register & 3) as u8,
            decreasing,
            exponential: self.register & 0x4000 != 0,
            frozen: false,
            negative_phase,
        };
        let (delta, increment) = rate.delta_and_increment(self.level, self.model.unwrap());
        self.counter += increment;
        if self.counter & 0x8000 == 0 {
            return;
        }
        self.counter = 0;
        let value = i32::from(self.level) + delta;
        if decreasing {
            self.level = if negative_phase {
                value.clamp(-32768, 0)
            } else {
                value.max(0)
            } as i16;
            self.active = self.level != 0;
        } else {
            self.level = value.clamp(-32768, 32767) as i16;
            self.active = self.level != if delta < 0 { -32768 } else { 32767 };
        }
    }
}
