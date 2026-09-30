//! PSX SPU ADPCM sample decoding, before pitch interpolation, ADSR or mixing.
//! Sample data has no intrinsic playback rate. Unsupported header values fail.
//! Coefficients/rounding were checked against independent SPU documentation and
//! DuckStation's decoder; hardware/rendering acceptance remains separate.

pub mod encoder;

use crate::Result;
use serde::Serialize;

pub const BLOCK_BYTES: usize = 16;
pub const BLOCK_FRAMES: usize = 28;
const FILTERS: [(i32, i32); 5] = [(0, 0), (60, 0), (115, -52), (98, -55), (122, -60)];

/// Shared reconstruction arithmetic for validated predictor indices.
pub(crate) fn prediction(history: History, filter: usize) -> i32 {
    let (first, second) = FILTERS[filter];
    ((i32::from(history.previous) * first) >> 6)
        + ((i32::from(history.previous_previous) * second) >> 6)
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize)]
pub struct History {
    pub previous: i16,
    pub previous_previous: i16,
}

impl History {
    /// Decode one physical block; flags are the caller's control-flow concern.
    /// Validation happens before changing history.
    pub fn decode_block(&mut self, block: &[u8; BLOCK_BYTES]) -> Result<[i16; BLOCK_FRAMES]> {
        let filter = usize::from(block[0] >> 4);
        let shift = block[0] & 15;
        if filter >= FILTERS.len() {
            return Err(format!("unsupported PSX ADPCM predictor {filter}").into());
        }
        if shift > 12 {
            return Err(format!(
                "unsupported PSX ADPCM shift {shift}; reserved hardware behavior is not modeled"
            )
            .into());
        }
        if block[1] & !7 != 0 {
            return Err(format!("unsupported PSX ADPCM flag bits {:#04x}", block[1] & !7).into());
        }
        let mut pcm = [0; BLOCK_FRAMES];
        let mut previous = i32::from(self.previous);
        let mut older = i32::from(self.previous_previous);
        for (index, sample) in pcm.iter_mut().enumerate() {
            let nibble = (block[2 + index / 2] >> ((index % 2) * 4)) & 15;
            let residual = i32::from((u16::from(nibble) << 12) as i16) >> shift;
            // Each predictor product is shifted separately before summation.
            let value = residual
                + prediction(
                    History {
                        previous: previous as i16,
                        previous_previous: older as i16,
                    },
                    filter,
                );
            *sample = value.clamp(i32::from(i16::MIN), i32::from(i16::MAX)) as i16;
            older = previous;
            previous = i32::from(*sample);
        }
        self.previous = previous as i16;
        self.previous_previous = older as i16;
        Ok(pcm)
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct SampleLoop {
    pub start_frame: usize,
    /// Internal ranges are exclusive; RIFF smpl requires end_frame_exclusive - 1.
    pub end_frame_exclusive: usize,
    /// False means repeating a fixed PCM slice differs from re-decoding ADPCM
    /// with the predictor history retained across the loop jump.
    pub pcm_repeat_is_stable: bool,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Termination {
    Empty,
    EndMute,
    EndRepeat,
    BoundedWithoutEnd,
}

#[derive(Debug)]
pub struct DecodedSample {
    pub pcm: Vec<i16>,
    pub sample_loop: Option<SampleLoop>,
    pub termination: Termination,
    pub decoded_bytes: usize,
    /// Undecoded trailing data remains owned by the original sample allocation.
    pub trailing_bytes: usize,
    pub final_history: History,
}

/// Decode a bounded sample from zero predictor history through its first end
/// marker. Loop flags do not prescribe rate, envelopes, key-off or duration.
pub fn decode_sample(encoded: &[u8]) -> Result<DecodedSample> {
    if !encoded.len().is_multiple_of(BLOCK_BYTES) {
        return Err(format!(
            "PSX ADPCM sample has {} bytes; complete {BLOCK_BYTES}-byte blocks required",
            encoded.len()
        )
        .into());
    }
    let frames = (encoded.len() / BLOCK_BYTES)
        .checked_mul(BLOCK_FRAMES)
        .ok_or("PSX ADPCM decoded frame count overflow")?;
    let mut pcm = Vec::new();
    pcm.try_reserve(frames)?;
    let mut history = History::default();
    let mut loop_block = None;
    let mut sample_loop = None;
    let mut decoded_bytes = 0;
    let mut termination = if encoded.is_empty() {
        Termination::Empty
    } else {
        Termination::BoundedWithoutEnd
    };
    for (index, block) in encoded.as_chunks::<BLOCK_BYTES>().0.iter().enumerate() {
        let samples = history.decode_block(block).map_err(|error| {
            format!(
                "PSX ADPCM block {index} at byte {}: {error}",
                index * BLOCK_BYTES
            )
        })?;
        if block[1] & 4 != 0 {
            loop_block = Some(index);
        }
        pcm.extend(samples);
        decoded_bytes += BLOCK_BYTES;
        if block[1] & 1 != 0 {
            termination = if block[1] & 2 == 0 {
                Termination::EndMute
            } else {
                Termination::EndRepeat
            };
            if termination == Termination::EndRepeat {
                let start_block = loop_block.ok_or_else(|| format!(
                    "PSX ADPCM block {index} repeats without a local loop-start marker; a game repeat-address context is required"
                ))?;
                let start_frame = start_block * BLOCK_FRAMES;
                let loop_bytes: &[u8; BLOCK_BYTES] =
                    encoded[start_block * BLOCK_BYTES..][..BLOCK_BYTES].try_into()?;
                let repeated = history.decode_block_copy(loop_bytes)?;
                sample_loop = Some(SampleLoop {
                    start_frame,
                    end_frame_exclusive: pcm.len(),
                    pcm_repeat_is_stable: repeated == pcm[start_frame..start_frame + BLOCK_FRAMES],
                });
            }
            break;
        }
    }
    Ok(DecodedSample {
        pcm,
        sample_loop,
        termination,
        decoded_bytes,
        trailing_bytes: encoded.len() - decoded_bytes,
        final_history: history,
    })
}

impl History {
    fn decode_block_copy(mut self, block: &[u8; BLOCK_BYTES]) -> Result<[i16; BLOCK_FRAMES]> {
        self.decode_block(block)
    }
}
