//! Explicitly selected, frame-driven reverb arithmetic; not hardware acceptance.
//! Register names follow psx-spx; arithmetic follows the current native unit.
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
    start: i32,
    cursor: i32, // Halfwords.
    left_tick: bool,
    input: [[i32; 39]; 2],
    output: [[i32; 20]; 2],
}

impl Reverb {
    /// Zeroed filter histories and left-first phase are explicit component
    /// startup state, not recovered BIOS state.
    pub fn new(_model: Model) -> Self {
        Self {
            registers: [0; 32],
            base: 0,
            start: 0,
            cursor: 0,
            left_tick: false,
            input: [[0; 39]; 2],
            output: [[0; 20]; 2],
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
                let start = if value <= 0x200 || value == 0xffff {
                    0
                } else {
                    i32::from(value) * 4
                };
                if self.start != start {
                    self.start = start;
                    self.cursor = start;
                }
            }
            0x1c0..=0x1fe if offset & 1 == 0 => {
                self.registers[((offset - 0x1c0) / 2) as usize] = value
            }
            _ => return Err("unsupported SPU reverb register".into()),
        }
        Ok(())
    }
    pub fn cursor_bytes(&self) -> u32 {
        self.cursor as u32 * 2
    }

    fn address(&self, offset: i32, displacement: i32) -> usize {
        let mut address = offset * 4 + self.cursor + displacement;
        while address > 0x3ffff {
            address = self.start + address - 0x40000;
        }
        while address < self.start {
            address = 0x3ffff - (self.start - address);
        }
        // The reference has asymmetric lower/upper folds, including the lower
        // fold's one-halfword difference. This is not ordinary ring modulo.
        address as usize * 2
    }
    fn load(&self, ram: &[u8], offset: i32, displacement: i32) -> i32 {
        let at = self.address(offset, displacement);
        i32::from(i16::from_le_bytes([ram[at], ram[at + 1]]))
    }
    fn save(&self, ram: &mut [u8], offset: i32, value: i32) {
        let at = self.address(offset, 0);
        ram[at..at + 2].copy_from_slice(&(clip(value) as i16).to_le_bytes());
    }

    fn network(&self, ram: &mut [u8], input: i32, channel: usize, write_enabled: bool) -> i32 {
        let r = self.registers.map(|value| i32::from(value as i16));
        let same = clip(
            product(self.load(ram, r[16 + channel], 0), r[7]) + product(input, r[30 + channel]),
        );
        let diff = clip(
            product(self.load(ram, r[24 + (channel ^ 1)], 0), r[7])
                + product(input, r[30 + channel]),
        );
        if write_enabled {
            for (destination, value) in [(10 + channel, same), (18 + channel, diff)] {
                self.save(
                    ram,
                    r[destination],
                    product(value, r[2])
                        + product(self.load(ram, r[destination], -1), 32768 - r[2]),
                );
            }
        }
        let mut out = clip(
            [(12, 3), (14, 4), (20, 5), (22, 6)]
                .into_iter()
                .map(|(source, coefficient)| {
                    product(self.load(ram, r[source + channel], 0), r[coefficient])
                })
                .sum(),
        );
        for (destination, delay, coefficient) in [(26 + channel, 0, 8), (28 + channel, 1, 9)] {
            // The second tap is read after the first stage's write, including
            // when their addresses alias. Offset subtraction stays signed.
            let tap = self.load(ram, r[destination] - r[delay], 0);
            out = clip(out - product(tap, r[coefficient]));
            if write_enabled {
                self.save(ram, r[destination], out);
            }
            out = clip(product(out, r[coefficient]) + tap);
        }
        out
    }

    fn interpolate(&self, channel: usize) -> i32 {
        (self.output[channel]
            .iter()
            .zip(FIR.iter().step_by(2))
            .map(|(&sample, &weight)| i64::from(sample) * i64::from(weight))
            .sum::<i64>()
            >> 14) as i32
    }
    fn right_output(&self) -> i32 {
        if self.left_tick {
            self.output[1][9]
        } else {
            self.interpolate(1)
        }
    }

    /// One 44.1 kHz frame with wide sends and already scheduled signed wet gains.
    /// Gain precedes interpolation. Writes are gated independently of reads,
    /// output and cursor advancement. Preset/base writes preserve FIR histories.
    pub fn tick(
        &mut self,
        ram: &mut [u8],
        input: [i32; 2],
        write_enabled: bool,
        wet_gain: [i16; 2],
    ) -> Result<[i32; 2]> {
        if ram.len() != RAM_BYTES {
            return Err("SPU reverb requires exactly 512 KiB RAM".into());
        }
        if self.start == 0 {
            // The native disabled-base path freezes histories and phase. Its
            // right output still reads that frozen history, even with left zero.
            return Ok([0, self.right_output()]);
        }
        for (history, sample) in self.input.iter_mut().zip(input) {
            history.copy_within(..38, 1);
            history[0] = sample;
        }
        self.left_tick = !self.left_tick;
        let channel = usize::from(!self.left_tick);
        let down = (self.input[channel]
            .iter()
            .zip(FIR)
            .map(|(&sample, weight)| i64::from(sample) * i64::from(weight))
            .sum::<i64>()
            >> 15)
            .clamp(-32768, 32767) as i32;
        let wet = product(
            self.network(ram, down, channel, write_enabled),
            i32::from(wet_gain[channel]),
        );
        self.output[channel].copy_within(..19, 1);
        self.output[channel][0] = wet;
        if !self.left_tick {
            self.cursor += 1;
            if self.cursor > 0x3ffff {
                self.cursor = self.start;
            }
        }
        let left = if self.left_tick {
            self.interpolate(0)
        } else {
            self.output[0][9]
        };
        Ok([left, self.right_output()])
    }
}

fn product(sample: i32, gain: i32) -> i32 {
    ((i64::from(sample) * i64::from(gain)) >> 15) as i32
}
fn clip(value: i32) -> i32 {
    value.clamp(-32768, 32767)
}
