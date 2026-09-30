//! Explicitly selected, frame-driven reverb arithmetic; not hardware acceptance.
//! Register names follow psx-spx; quantization follows the inspected emulator.
use super::spu_transfer::RAM_BYTES;
use crate::Result;
use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Model {
    EmulatorReference,
}

// Published numeric 39-tap FIR. Odd taps are zero except the center.
const FIR: [i32; 39] = [
    -1, 0, 2, 0, -10, 0, 35, 0, -103, 0, 266, 0, -616, 0, 1332, 0, -2960, 0, 10246, 16384, 10246,
    0, -2960, 0, 1332, 0, -616, 0, 266, 0, -103, 0, 35, 0, -10, 0, 2, 0, -1,
];

pub struct Reverb {
    registers: [u16; 32],
    base: u16,
    cursor: u32, // Halfwords.
    phase: usize,
    input: [[i16; 64]; 2],
    output: [[i16; 32]; 2],
}

impl Reverb {
    /// Zeroed filter histories and phase zero are explicit component startup
    /// state, not recovered BIOS state. Configure once through the interconnect.
    pub fn new(_model: Model) -> Self {
        Self {
            registers: [0; 32],
            base: 0,
            cursor: 0,
            phase: 0,
            input: [[0; 64]; 2],
            output: [[0; 32]; 2],
        }
    }
    pub fn read(&self, offset: u32) -> Result<u16> {
        match offset {
            0x1a2 => Ok(self.base),
            0x1c0..=0x1fe if offset & 1 == 0 => Ok(self.registers[((offset - 0x1c0) / 2) as usize]),
            _ => Err("unsupported SPU reverb register".into()),
        }
    }
    pub fn write(&mut self, offset: u32, value: u16) -> Result<()> {
        match offset {
            0x1a2 => {
                self.base = value;
                self.cursor = u32::from(value) * 4;
            }
            0x1c0..=0x1fe if offset & 1 == 0 => {
                self.registers[((offset - 0x1c0) / 2) as usize] = value
            }
            _ => return Err("unsupported SPU reverb register".into()),
        }
        Ok(())
    }
    pub fn cursor_bytes(&self) -> u32 {
        self.cursor * 2
    }

    fn address(&self, register: u16, displacement: i32) -> Result<usize> {
        let offset = (i64::from(register) * 4 + i64::from(displacement)) & 0x3ffff;
        let base = u32::from(self.base) * 4;
        let sum = self.cursor + offset as u32;
        let wrapped = (sum + if sum >= 0x40000 { base } else { 0 }) & 0x3ffff;
        // One conditional base addition, not modulo by work-area size.
        // Large/negative offsets can alias below base in this source model;
        // hardware behavior remains an acceptance question.
        Ok(wrapped as usize * 2)
    }
    fn load(&self, ram: &[u8], register: u16, displacement: i32) -> Result<i32> {
        let at = self.address(register, displacement)?;
        Ok(i32::from(i16::from_le_bytes([ram[at], ram[at + 1]])))
    }
    fn save(&self, ram: &mut [u8], register: u16, value: i16) -> Result<()> {
        let at = self.address(register, 0)?;
        ram[at..at + 2].copy_from_slice(&value.to_le_bytes());
        Ok(())
    }

    /// One 22.05 kHz memory-network step. Writes are gated independently of
    /// reads/output/cursor advancement. Errors can follow earlier RAM writes.
    pub fn tick_half_rate(
        &mut self,
        ram: &mut [u8],
        input: [i16; 2],
        write_enabled: bool,
    ) -> Result<[i16; 2]> {
        if ram.len() != RAM_BYTES {
            return Err("SPU reverb requires exactly 512 KiB RAM".into());
        }
        let r = self.registers;
        let gain = |index: usize| i64::from(r[index] as i16);
        let mut result = [0; 2];
        for channel in 0..2 {
            if write_enabled {
                // SAME and DIFF reflection branches; cross-side DIFF input.
                for (source, destination) in [
                    (16 + channel, 10 + channel),
                    (24 + (channel ^ 1), 18 + channel),
                ] {
                    let reflected = clip(
                        (((i64::from(self.load(ram, r[source], 0)?) * gain(7)) >> 14)
                            + ((i64::from(input[channel]) * gain(30 + channel)) >> 14))
                            >> 1,
                    );
                    let previous = i64::from(self.load(ram, r[destination], -1)?);
                    // The -32768 reflection coefficient has a documented sign
                    // anomaly. This branch records the emulator's numeric model.
                    let complement = if gain(2) == -32768 {
                        if previous == -32768 {
                            0
                        } else {
                            previous * -65536
                        }
                    } else {
                        previous * (32768 - gain(2))
                    };
                    let value =
                        clip((((i64::from(reflected) * gain(2)) >> 14) + (complement >> 14)) >> 1);
                    self.save(ram, r[destination], value)?;
                }
            }
            let mut comb = 0i64;
            for (source, coefficient) in [(12, 3), (14, 4), (20, 5), (22, 6)] {
                comb +=
                    (i64::from(self.load(ram, r[source + channel], 0)?) * gain(coefficient)) >> 14;
            }
            let delayed1 = i64::from(self.load(ram, r[26 + channel].wrapping_sub(r[0]), 0)?);
            let delayed2 = i64::from(self.load(ram, r[28 + channel].wrapping_sub(r[1]), 0)?);
            let inverse = |index| {
                if gain(index) == -32768 {
                    32767
                } else {
                    -gain(index)
                }
            };
            let stage1 = clip((comb + ((delayed1 * inverse(8)) >> 14)) >> 1);
            let stage2 = clip(
                delayed1
                    + ((((i64::from(stage1) * gain(8)) >> 14) + ((delayed2 * inverse(9)) >> 14))
                        >> 1),
            );
            result[channel] = clip(delayed2 + ((i64::from(stage2) * gain(9)) >> 15));
            if write_enabled {
                self.save(ram, r[26 + channel], stage1)?;
                self.save(ram, r[28 + channel], stage2)?;
            }
        }
        self.cursor = (self.cursor + 1) & 0x3ffff;
        if self.cursor == 0 {
            self.cursor = u32::from(self.base) * 4;
        }
        Ok(result)
    }

    /// One 44.1 kHz frame; old filter history survives preset/base writes and
    /// write-disable transitions. Output is before the separate wet return gain.
    pub fn tick(
        &mut self,
        ram: &mut [u8],
        input: [i16; 2],
        write_enabled: bool,
    ) -> Result<[i16; 2]> {
        if ram.len() != RAM_BYTES {
            return Err("SPU reverb requires exactly 512 KiB RAM".into());
        }
        for (channel, sample) in input.into_iter().enumerate() {
            self.input[channel][self.phase] = sample;
        }
        let mut result = [0; 2];
        if self.phase & 1 != 0 {
            let down = std::array::from_fn(|channel| {
                let sum: i64 = FIR
                    .iter()
                    .enumerate()
                    .map(|(tap, &weight)| {
                        i64::from(self.input[channel][(self.phase + 64 - 38 + tap) & 63])
                            * i64::from(weight)
                    })
                    .sum();
                clip(sum >> 15)
            });
            let wet = self.tick_half_rate(ram, down, write_enabled)?;
            for channel in 0..2 {
                self.output[channel][self.phase / 2] = wet[channel];
                let sum: i64 = (0..20)
                    .map(|tap| {
                        i64::from(self.output[channel][(self.phase / 2 + 32 - 19 + tap) & 31])
                            * i64::from(FIR[tap * 2])
                    })
                    .sum();
                result[channel] = clip(sum >> 14);
            }
        } else {
            for (channel, sample) in result.iter_mut().enumerate() {
                *sample = self.output[channel][(self.phase / 2 + 32 - 10) & 31];
            }
        }
        self.phase = (self.phase + 1) & 63;
        Ok(result)
    }
}

fn clip(value: i64) -> i16 {
    value.clamp(-32768, 32767) as i16
}
