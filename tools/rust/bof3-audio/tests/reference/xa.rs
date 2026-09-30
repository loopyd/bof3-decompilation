//! Independent integer/vector model ported from the retained pre-migration oracles.
//! Shares only numeric FIR coefficients, never production decoding or queue state.

#[path = "../../src/machine/xa_filter.rs"]
mod coefficients;
use bof3_audio::digest::sha256_hex;

pub fn sample_hash(coding: u8, combined: bool) -> String {
    let bits = if coding & 16 != 0 { 8 } else { 4 };
    let channels = if coding & 1 != 0 { 2 } else { 1 };
    let mut history = [[0i32; 2]; 2];
    let mut pcm = Vec::new();
    for sector in 0..3 {
        for group in 0..18 {
            let words: Vec<_> = (0..28)
                .map(|row| {
                    let mut bytes = [0u8; 4];
                    for (column, byte) in bytes.iter_mut().enumerate() {
                        *byte = ((row * 4 + column) * 37 + group * 13 + sector * 53) as u8;
                    }
                    u32::from_le_bytes(bytes)
                })
                .collect();
            let mut units = Vec::new();
            for unit in 0..32 / bits {
                let (positive, negative) =
                    [(0, 0), (60, 0), (115, -52), (98, -55)][(unit + group) % 4];
                let shift = (sector + unit + group) % 13;
                let channel = unit % channels;
                let [mut old, mut older] = history[channel];
                let mut values = Vec::new();
                for &word in &words {
                    let mut value = ((word >> (unit * bits)) % (1 << bits)) as i32;
                    if value >= 1 << (bits - 1) {
                        value -= 1 << bits;
                    }
                    let residual = (value * (1 << (16 - bits))).div_euclid(1 << shift);
                    let predicted = if combined {
                        (old * positive + older * negative + 32).div_euclid(64)
                    } else {
                        (old * positive).div_euclid(64) + (older * negative).div_euclid(64)
                    };
                    let result = (residual + predicted).clamp(-32768, 32767);
                    values.push(result as i16);
                    older = old;
                    old = result;
                }
                history[channel] = [old, older];
                units.push(values);
            }
            for base in (0..units.len()).step_by(channels) {
                for row in 0..28 {
                    for unit in &units[base..base + channels] {
                        pcm.extend(unit[row].to_le_bytes());
                    }
                }
            }
        }
    }
    sha256_hex(&pcm)
}

struct Reference {
    predictors: [[i32; 2]; 2],
    rings: [[i32; 32]; 2],
    cursor: usize,
    phase: usize,
}
impl Reference {
    fn new() -> Self {
        Self {
            predictors: [[0; 2]; 2],
            rings: [[0; 32]; 2],
            cursor: 0,
            phase: 6,
        }
    }
    fn decode(&mut self, coding: u8, seed: usize) -> Vec<Vec<i16>> {
        let channels = if coding & 1 != 0 { 2 } else { 1 };
        let depth = if coding & 16 != 0 { 8 } else { 4 };
        let units = 32 / depth;
        let mut pcm = Vec::new();
        for group in 0..18 {
            let mut blocks = Vec::new();
            for unit in 0..units {
                let lane = unit % channels;
                let history = &mut self.predictors[lane];
                let mut samples = Vec::new();
                for frame in 0..28 {
                    let column = if depth == 8 { unit } else { unit / 2 };
                    let packed = (group * 17 + (frame * 4 + column) * 29 + seed * 43) & 255;
                    let value = if depth == 8 {
                        packed
                    } else {
                        (packed >> (4 * (unit % 2))) & 15
                    } as i32;
                    let signed = if value & (1 << (depth - 1)) != 0 {
                        value - (1 << depth)
                    } else {
                        value
                    };
                    let residual = (signed << (16 - depth)) >> 8;
                    let sample = (residual + ((history[0] * 60) >> 6)).clamp(-32768, 32767);
                    *history = [sample, history[0]];
                    samples.push(sample as i16);
                }
                blocks.push(samples);
            }
            for first in (0..units).step_by(channels) {
                for row in 0..28 {
                    pcm.push(
                        blocks[first..first + channels]
                            .iter()
                            .map(|b| b[row])
                            .collect(),
                    );
                }
            }
        }
        pcm
    }
    fn interpolate(&self, half: bool, table: usize, channel: usize) -> i16 {
        let weights: &[i16] = if half {
            &coefficients::HALF_RATE[table]
        } else {
            &coefficients::ZIGZAG[table]
        };
        let mut sum = 0i64;
        for (tap, &weight) in weights.iter().enumerate() {
            let index = if half {
                (self.cursor + 32 - 25 + tap) % 32
            } else {
                (self.cursor + 32 - tap) % 32
            };
            let product = i64::from(self.rings[channel][index]) * i64::from(weight);
            sum += if half { product } else { product >> 15 };
        }
        if half {
            sum >>= 15;
        }
        sum.clamp(-32768, 32767) as i16
    }
    fn emit(&self, half: bool, stereo: bool, table: usize) -> [i16; 2] {
        let left = self.interpolate(half, table, 0);
        [
            left,
            if stereo {
                self.interpolate(half, table, 1)
            } else {
                left
            },
        ]
    }
    fn resample(&mut self, coding: u8, pcm: &[Vec<i16>]) -> Vec<[i16; 2]> {
        let half = coding & 4 != 0;
        let stereo = coding & 1 != 0;
        let mut output = Vec::new();
        for frame in pcm {
            if half {
                while self.phase < 7 {
                    output.push(self.emit(half, stereo, self.phase));
                    self.phase += 3;
                }
                self.phase -= 7;
                self.cursor = (self.cursor + 1) % 32;
            }
            for (channel, &sample) in frame.iter().enumerate() {
                self.rings[channel][self.cursor] = i32::from(sample);
            }
            if half {
                output.push(self.emit(half, stereo, self.phase));
                self.phase += 3;
            } else {
                self.cursor = (self.cursor + 1) % 32;
                self.phase -= 1;
                if self.phase == 0 {
                    self.phase = 6;
                    for table in 0..7 {
                        output.push(self.emit(half, stereo, table));
                    }
                }
            }
        }
        output
    }
}

pub fn transition(first: u8, second: u8, admission: &str) -> ([usize; 3], String) {
    let mut reference = Reference::new();
    let mut counts = [0; 3];
    let mut output = Vec::new();
    for (stage, coding) in [first, second, first].into_iter().enumerate() {
        if stage != 1 || admission != "dropped" {
            let pcm = reference.decode(coding, stage + 1);
            if stage != 1 || admission == "queued" {
                let frames = reference.resample(coding, &pcm);
                counts[stage] = frames.len();
                output.extend(frames);
            }
        }
    }
    (counts, hash(&output))
}
fn hash(frames: &[[i16; 2]]) -> String {
    sha256_hex(
        &frames
            .iter()
            .flat_map(|f| f.iter().flat_map(|s| s.to_le_bytes()))
            .collect::<Vec<_>>(),
    )
}

#[test]
fn fixed_interpolation_vectors_preserve_original_reference_hashes() {
    let pcm: Vec<_> = (0..120)
        .map(|n| {
            vec![
                ((n * 7919 + 12345) % 65536 - 32768) as i16,
                ((n * 3571 + 54321) % 65536 - 32768) as i16,
            ]
        })
        .collect();
    for (coding, count, expected) in [
        (
            1,
            140,
            "02c0564a53770ad10903fd71db17581e259586d9b47d3256a0718d2487ad981b",
        ),
        (
            5,
            279,
            "c6f7caec5c80a90968773f8fac24149bb039ec81746ad3d0a5ef70814ad165a4",
        ),
    ] {
        let frames = Reference::new().resample(coding, &pcm);
        assert_eq!(frames.len(), count);
        assert_eq!(hash(&frames), expected);
    }
}
