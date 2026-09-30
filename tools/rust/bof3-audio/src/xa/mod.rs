//! XA sector ADPCM decoding at the encoded rate, before CD resampling and mixing.

pub mod cue;
pub mod encoder;
pub mod extraction;
pub(crate) mod manifest;
pub mod packing;
pub mod reconstruction;
pub mod reference;
pub mod selection;

use crate::{archive::XA_SECTOR_BYTES, codec::adpcm::History, Result};
use serde::Serialize;

const GROUPS: usize = 18;
const GROUP_BYTES: usize = 128;
const UNIT_FRAMES: usize = 28;
const PREDICTORS: [(i32, i32); 4] = [(0, 0), (60, 0), (115, -52), (98, -55)];

pub(crate) fn prediction(history: History, filter: usize, arithmetic: Arithmetic) -> i32 {
    let (positive, negative) = PREDICTORS[filter];
    let previous = i32::from(history.previous) * positive;
    let older = i32::from(history.previous_previous) * negative;
    match arithmetic {
        Arithmetic::SplitFloor => (previous >> 6) + (older >> 6),
        Arithmetic::CombinedRounded => (previous + older + 32) >> 6,
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
pub struct Stream {
    pub file: u8,
    pub channel: u8,
    pub coding: u8,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct Format {
    channels: u16,
    sample_rate: u32,
    bits_per_sample: u8,
    emphasis: bool,
}

impl Format {
    pub fn from_coding(coding: u8) -> Result<Self> {
        if coding & 0xaa != 0 {
            return Err(
                format!("XA coding {coding:#04x}: reserved channel/rate/depth or bit 7").into(),
            );
        }
        Ok(Self {
            channels: if coding & 1 != 0 { 2 } else { 1 },
            sample_rate: if coding & 4 != 0 { 18900 } else { 37800 },
            bits_per_sample: if coding & 0x10 != 0 { 8 } else { 4 },
            emphasis: coding & 0x40 != 0,
        })
    }

    pub fn channels(self) -> u16 {
        self.channels
    }
    pub fn sample_rate(self) -> u32 {
        self.sample_rate
    }
    pub fn bits_per_sample(self) -> u8 {
        self.bits_per_sample
    }
    pub fn emphasis(self) -> bool {
        self.emphasis
    }

    pub fn frames_per_sector(self) -> usize {
        GROUPS * UNIT_FRAMES * (32 / usize::from(self.bits_per_sample)) / usize::from(self.channels)
    }
}

/// Reference implementations disagree. Callers must select and record the
/// arithmetic explicitly; neither variant alone establishes hardware fidelity.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Arithmetic {
    /// Separately floor each predictor product, as in DuckStation's CD decoder.
    SplitFloor,
    /// Round the combined predictor, as in PSX-SPX pseudocode and FFmpeg 6.1.1.
    CombinedRounded,
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize)]
pub struct Histories {
    pub left: History,
    pub right: History,
}

#[derive(Clone, Debug)]
pub struct Decoder {
    stream: Stream,
    format: Format,
    arithmetic: Arithmetic,
    histories: Histories,
}

impl Decoder {
    /// Initial history is an explicit playback/export context. Zero is a useful
    /// standalone export convention, not proof of the game's seek/reset state.
    pub fn new(stream: Stream, arithmetic: Arithmetic, histories: Histories) -> Result<Self> {
        let format = Format::from_coding(stream.coding)?;
        if format.emphasis {
            return Err(
                "XA emphasis filtering is not implemented; refusing silent omission".into(),
            );
        }
        Ok(Self {
            stream,
            format,
            arithmetic,
            histories,
        })
    }

    pub fn format(&self) -> Format {
        self.format
    }
    pub fn stream(&self) -> Stream {
        self.stream
    }
    pub fn arithmetic(&self) -> Arithmetic {
        self.arithmetic
    }
    pub fn histories(&self) -> Histories {
        self.histories
    }
    pub fn reset(&mut self, histories: Histories) {
        self.histories = histories;
    }

    /// Decode exactly one extracted 2336-byte sector. Only selected audio may
    /// enter this decoder. EOF remains audio and does not implicitly reset it.
    /// Errors leave history unchanged, including failures in the final group.
    pub fn decode_sector(&mut self, sector: &[u8]) -> Result<Vec<i16>> {
        if sector.len() != XA_SECTOR_BYTES {
            return Err("XA decode requires one whole 2336-byte sector".into());
        }
        if sector[..4] != sector[4..8] {
            return Err("XA sector subheader copies disagree".into());
        }
        if sector[2] & 0x2e != 0x24 {
            return Err("XA decode requires Form 2 audio without video/data submode bits".into());
        }
        if [sector[0], sector[1], sector[3]]
            != [self.stream.file, self.stream.channel, self.stream.coding]
        {
            return Err("XA sector file/channel/coding does not match this decoder; select or reset contexts explicitly".into());
        }
        let channels = usize::from(self.format.channels);
        let units = 32 / usize::from(self.format.bits_per_sample);
        let samples_per_group = units * UNIT_FRAMES;
        let mut pcm = vec![0; samples_per_group * GROUPS];
        let mut histories = self.histories;
        for group_index in 0..GROUPS {
            let start = 8 + group_index * GROUP_BYTES;
            let group = &sector[start..start + GROUP_BYTES];
            if group[..4] != group[4..8] || (units == 8 && group[8..12] != group[12..16]) {
                return Err(format!("XA group {group_index}: parameter copies disagree; error correction is not implemented").into());
            }
            for unit in 0..units {
                let parameter = group[4 + unit];
                let shift = parameter & 15;
                let filter = usize::from(parameter >> 4);
                if shift > 12 || filter >= PREDICTORS.len() {
                    return Err(format!("XA group {group_index} unit {unit}: unsupported parameter {parameter:#04x} (reserved shift or predictor)").into());
                }
                let history = if channels == 2 && unit % 2 == 1 {
                    &mut histories.right
                } else {
                    &mut histories.left
                };
                for frame in 0..UNIT_FRAMES {
                    // Four columns, each holding one 8-bit unit or two 4-bit
                    // units. Every unit's 28 values form a consecutive block.
                    let packed = group[16 + frame * 4 + if units == 8 { unit / 2 } else { unit }];
                    let residual = if units == 8 {
                        let nibble = (packed >> ((unit % 2) * 4)) & 15;
                        i32::from((i16::from(nibble) << 12) >> shift)
                    } else {
                        (i32::from(packed as i8) * 256) >> shift
                    };
                    let sample = (residual + prediction(*history, filter, self.arithmetic))
                        .clamp(-32768, 32767) as i16;
                    history.previous_previous = history.previous;
                    history.previous = sample;
                    let at = group_index * samples_per_group
                        + (unit / channels * UNIT_FRAMES + frame) * channels
                        + unit % channels;
                    pcm[at] = sample;
                }
            }
        }
        self.histories = histories;
        Ok(pcm)
    }
}
