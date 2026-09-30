//! Lossy PCM-to-SPU ADPCM encoding for edited samples, not unchanged archives.
//! Uses the decoder's integer reconstruction model and reports measured loss.

use crate::{
    codec::adpcm::{self, History, BLOCK_BYTES, BLOCK_FRAMES},
    codec::quantization::{measure, search, Loss},
    Result,
};
use serde::Serialize;
use std::ops::Range;

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Padding {
    #[default]
    Reject,
    Zero,
}

#[derive(Clone, Debug, Default)]
pub struct Options {
    /// Forward loop, in PCM frames, with an exclusive end at the sample end.
    pub sample_loop: Option<Range<usize>>,
    /// Non-looping final blocks may be zero-padded only by explicit request.
    pub padding: Padding,
    /// Available encoded sample bytes, excluding unrelated allocation padding.
    pub capacity_bytes: Option<usize>,
}

#[derive(Clone, Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub input_frames: usize,
    pub encoded_frames: usize,
    pub padded_frames: usize,
    pub encoded_bytes: usize,
    pub capacity_bytes: Option<usize>,
    pub initial_history: History,
    pub sample_loop: Option<adpcm::SampleLoop>,
    pub predictor_blocks: [usize; 5],
    pub loss: Loss,
    /// Quantization of the explicitly added zero targets, if any.
    pub padding_loss: Option<Loss>,
    pub limitations: Vec<&'static str>,
}

#[derive(Debug)]
pub struct Encoded {
    pub bytes: Vec<u8>,
    pub report: Report,
}

pub fn encode(pcm: &[i16], options: &Options) -> Result<Encoded> {
    if let Some(range) = &options.sample_loop {
        if range.start >= range.end || range.end != pcm.len() {
            return Err("PSX ADPCM encode: forward loop must be nonempty and end at the sample end; post-loop tails require runtime context".into());
        }
        if !range.start.is_multiple_of(BLOCK_FRAMES) || !range.end.is_multiple_of(BLOCK_FRAMES) {
            return Err("PSX ADPCM encode: loop endpoints must align to 28-frame blocks; loops are never padded or moved".into());
        }
    }
    let blocks = pcm.len().div_ceil(BLOCK_FRAMES);
    let frames = blocks
        .checked_mul(BLOCK_FRAMES)
        .ok_or("PSX ADPCM encode: frame count overflow")?;
    let byte_count = blocks
        .checked_mul(BLOCK_BYTES)
        .ok_or("PSX ADPCM encode: encoded size overflow")?;
    if frames != pcm.len() && options.padding == Padding::Reject {
        return Err(format!(
            "PSX ADPCM encode: {} frames do not align to 28; explicit zero padding is required",
            pcm.len()
        )
        .into());
    }
    if let Some(capacity) = options.capacity_bytes {
        if !capacity.is_multiple_of(BLOCK_BYTES) {
            return Err("PSX ADPCM encode: capacity must align to 16-byte blocks".into());
        }
        if byte_count > capacity {
            return Err(format!("PSX ADPCM encode: needs {byte_count} bytes, exceeding capacity {capacity}; sample was not truncated").into());
        }
    }
    let loop_block = options.sample_loop.as_ref().map(|r| r.start / BLOCK_FRAMES);
    let mut history = History::default();
    let mut bytes = Vec::new();
    bytes.try_reserve_exact(byte_count)?;
    let mut predictor_blocks = [0; 5];
    for (index, input) in pcm.chunks(BLOCK_FRAMES).enumerate() {
        let mut target = [0; BLOCK_FRAMES];
        target[..input.len()].copy_from_slice(input);
        // A filter-zero loop entry removes both incoming history values within
        // this block. Every later block then has identical history each pass.
        let independent_entry = loop_block == Some(index);
        let best = search(
            &target,
            history,
            4,
            if independent_entry { 1 } else { 5 },
            adpcm::prediction,
        );
        let mut block = [0; BLOCK_BYTES];
        block[0] = best.parameter;
        for (i, code) in best.codes.into_iter().enumerate() {
            block[2 + i / 2] |= ((code as u8) & 15) << ((i % 2) * 4);
        }
        if independent_entry {
            block[1] |= 4;
        }
        if index + 1 == blocks {
            block[1] |= if loop_block.is_some() { 3 } else { 1 };
        }
        // The decoder is the shared arithmetic owner. Reject any divergence
        // between candidate scoring and actual decoding before returning data.
        let decoded = history.decode_block(&block)?;
        if decoded != best.pcm || history != best.history {
            return Err("PSX ADPCM encode: candidate reconstruction verification failed".into());
        }
        predictor_blocks[usize::from(block[0] >> 4)] += 1;
        bytes.extend(block);
    }
    let decoded = adpcm::decode_sample(&bytes)?;
    if decoded.pcm.len() != frames
        || decoded
            .sample_loop
            .map(|l| l.start_frame..l.end_frame_exclusive)
            != options.sample_loop
        || decoded.sample_loop.is_some_and(|l| !l.pcm_repeat_is_stable)
    {
        return Err("PSX ADPCM encode: sample structure verification failed".into());
    }
    let loss = measure(pcm, &decoded.pcm[..pcm.len()]);
    let padding_loss = (frames != pcm.len()).then(|| {
        measure(
            &[0; BLOCK_FRAMES][..frames - pcm.len()],
            &decoded.pcm[pcm.len()..],
        )
    });
    Ok(Encoded {
        bytes,
        report: Report {
            schema: "bof3.audio.psx-adpcm-encoding/v1",
            input_frames: pcm.len(),
            encoded_frames: frames,
            padded_frames: frames - pcm.len(),
            encoded_bytes: byte_count,
            capacity_bytes: options.capacity_bytes,
            initial_history: History::default(),
            sample_loop: decoded.sample_loop,
            predictor_blocks,
            loss,
            padding_loss,
            limitations: vec![
                "Lossy edited-sample encoding; unchanged media must reuse its preserved encoding and allocation bytes.",
                "Each block searches predictors/shifts using greedy nearest reconstructed samples, not a global optimum over nibble sequences or blocks.",
                "Zero initial predictor history is the export reference; SPU pitch interpolation, envelopes, mixing and game voice context are not modeled here.",
                "Loop entry uses predictor zero for identical decoded PCM on every traversal; this can increase local encoding error.",
                "Loss covers original input frames before pitch or resampling; explicit padding is reported separately and adds decoded duration.",
            ],
        },
    })
}
