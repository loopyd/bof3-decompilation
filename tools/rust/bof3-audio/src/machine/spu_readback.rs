//! Development-only ENVX observations at interpreter access cycles, not PCM time.
use super::{
    adsr::{Adsr, Model, Phase, Registers},
    spu_sample::{pitch_step, Model as SampleModel},
    spu_transfer::RAM_BYTES,
    spu_voices::VOICES,
};
use crate::Result;

const SAMPLE_CYCLES: u64 = 33_868_800 / 44_100;

/// Effective native settings captured around the reference execution.
/// Negative key-on delay follows the native zero-delay interpretation.
#[derive(Clone, Copy, Debug)]
pub struct Settings {
    pub key_on_delay: i32,
    pub interpolation: u32,
}

struct Cursor {
    block: Option<usize>,
    repeat: Option<usize>,
    remaining: u8,
    position: u32,
    step: u32,
    ignore_loop: bool,
    untracked: bool,
}

impl Cursor {
    fn consume(&mut self, ram: &[u8]) -> bool {
        if self.untracked {
            return true;
        }
        while self.position >= 0x10000 {
            if self.remaining == 0 {
                let Some(block) = self.block else {
                    return false;
                };
                let flags = ram[block + 1];
                if flags & 4 != 0 && !self.ignore_loop {
                    self.repeat = Some(block);
                }
                self.block = if flags & 1 == 0 {
                    Some((block + 16) % RAM_BYTES)
                } else if flags == 3 {
                    self.repeat
                } else {
                    None
                };
                self.remaining = 28;
            }
            self.remaining -= 1;
            self.position -= 0x10000;
        }
        true
    }
}

struct Key {
    cycle: u64,
    off: Option<u64>,
    steps: u64,
    envelope: Adsr,
    cursor: Cursor,
    ended: bool,
}

struct Voice {
    start: Option<usize>,
    repeat: Option<usize>,
    pitch: u16,
    registers: Registers,
    key: Option<Key>,
}

impl Voice {
    fn new() -> Self {
        Self {
            start: None,
            repeat: None,
            pitch: 0,
            registers: Registers { adsr1: 0, adsr2: 0 },
            key: None,
        }
    }

    fn advance(&mut self, cycle: u64, delay: u64, ram: &[u8]) -> Result<u16> {
        let Some(key) = &mut self.key else {
            return Ok(0);
        };
        let elapsed = cycle
            .checked_sub(key.cycle)
            .ok_or("reference ENVX cycle precedes key-on")?;
        let target = (elapsed / SAMPLE_CYCLES).saturating_sub(delay);
        if target < key.steps {
            return Err("reference ENVX cycles moved backwards".into());
        }
        let release = key
            .off
            .map(|off| ((off - key.cycle) / SAMPLE_CYCLES).saturating_sub(delay));
        while key.steps < target && !key.ended {
            if !key.cursor.consume(ram) {
                key.ended = true;
                break;
            }
            if release.is_some_and(|step| key.steps >= step)
                && !matches!(key.envelope.phase(), Phase::Release | Phase::Off)
            {
                if key.envelope.counter() != 0 {
                    return Err(
                        "reference ENVX release with fractional ADSR state is unmodeled".into(),
                    );
                }
                key.envelope.key_off();
            }
            key.envelope.tick();
            if !key.cursor.untracked {
                key.cursor.position += key.cursor.step;
            }
            key.steps += 1;
        }
        key.steps = target;
        Ok(if key.ended {
            0
        } else {
            key.envelope.level() as u16
        })
    }
}

pub(crate) struct Readback {
    settings: Settings,
    voices: [Voice; VOICES],
    modulation: u32,
}

impl Readback {
    pub(crate) fn new(settings: Settings) -> Self {
        Self {
            settings,
            voices: std::array::from_fn(|_| Voice::new()),
            modulation: 0,
        }
    }

    fn delay(&self) -> u64 {
        self.settings.key_on_delay.max(0) as u64
    }

    pub(crate) fn read(&mut self, voice: usize, cycle: u64, ram: &[u8]) -> Result<u16> {
        let delay = self.delay();
        self.voices[voice].advance(cycle, delay, ram)
    }

    /// Observe CPU-stamped writes before changing the ordinary port owner.
    /// A rejected history aborts execution; its elapsed state must not be replayed.
    pub(crate) fn write(&mut self, offset: u32, value: u16, cycle: u64, ram: &[u8]) -> Result<()> {
        let delay = self.delay();
        if offset < 0x180 {
            let voice = &mut self.voices[(offset / 16) as usize];
            match offset & 15 {
                4 => {
                    voice.advance(cycle, delay, ram)?;
                    voice.pitch = pitch_step(value, None, SampleModel::EmulatorReference);
                    if let Some(key) = &mut voice.key {
                        key.cursor.step = (u32::from(voice.pitch) << 4).max(1);
                    }
                }
                6 => voice.start = Some(usize::from(value & !1) * 8),
                8 | 10 => {
                    voice.advance(cycle, delay, ram)?;
                    if voice
                        .key
                        .as_ref()
                        .is_some_and(|key| !key.ended && key.envelope.phase() != Phase::Off)
                    {
                        return Err("reference ENVX active ADSR writes are unmodeled".into());
                    }
                    if offset & 15 == 8 {
                        voice.registers.adsr1 = value;
                    } else {
                        voice.registers.adsr2 = value;
                    }
                    if let Some(key) = &mut voice.key {
                        key.envelope.write_registers(voice.registers);
                    }
                }
                // The native CPU register handler ignores manual ENVX writes.
                12 => (),
                14 => {
                    voice.advance(cycle, delay, ram)?;
                    voice.repeat = Some(usize::from(value & !1) * 8);
                    if let Some(key) = &mut voice.key {
                        key.cursor.repeat = voice.repeat;
                        key.cursor.ignore_loop = true;
                    }
                }
                _ => (),
            }
            return Ok(());
        }
        let shift = (offset & 2) * 8;
        let mask = (u32::from(value) << shift) & 0x00ff_ffff;
        match offset & !2 {
            0x188 => {
                for (index, voice) in self.voices.iter_mut().enumerate() {
                    if mask & (1 << index) == 0 {
                        continue;
                    }
                    let Some(start) = voice.start else { continue };
                    let mut envelope = Adsr::new(voice.registers, Model::EmulatorReference);
                    envelope.key_on();
                    voice.key = Some(Key {
                        cycle,
                        off: None,
                        steps: 0,
                        envelope,
                        ended: false,
                        cursor: Cursor {
                            block: Some(start),
                            repeat: voice.repeat,
                            remaining: 0,
                            position: if self.settings.interpolation >= 2 {
                                0x30000
                            } else {
                                0x10000
                            },
                            step: (u32::from(voice.pitch) << 4).max(1),
                            ignore_loop: false,
                            untracked: self.modulation & (1 << index) != 0,
                        },
                    });
                }
            }
            0x18c => {
                for (index, voice) in self.voices.iter_mut().enumerate() {
                    if mask & (1 << index) != 0 {
                        if let Some(key) = &mut voice.key {
                            if key.off.is_none() && cycle > key.cycle {
                                key.off = Some(cycle);
                            }
                        }
                    }
                }
            }
            0x190 => {
                // Native FModOn assigns roles in ascending order within the
                // written half. A target makes the preceding voice a source,
                // even when that voice was selected earlier in this write.
                let first = shift as usize;
                for index in first..(first + 16).min(VOICES) {
                    let bit = 1 << index;
                    if mask & bit != 0 {
                        if index == 0 {
                            continue;
                        }
                        let voice = &mut self.voices[index];
                        voice.advance(cycle, delay, ram)?;
                        if let Some(key) = &mut voice.key {
                            key.cursor.untracked = true;
                        }
                        self.modulation = (self.modulation | bit) & !(bit >> 1);
                    } else {
                        self.modulation &= !bit;
                    }
                }
            }
            _ => (),
        }
        Ok(())
    }
}
