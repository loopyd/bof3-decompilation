//! Shared decoded-PCM edit detection and bounded ADPCM allocation reconstruction.
use crate::{codec::adpcm, codec::adpcm::encoder, Result};
use serde::Serialize;
use std::ops::Range;

#[derive(Debug, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum LoopPolicy {
    PreserveOriginalEncoding,
    StabilizePcmLoop,
    ReencodeWithoutLoop,
}

#[derive(Debug, Serialize)]
pub struct LoopReport {
    pub original: Option<adpcm::SampleLoop>,
    pub output: Option<adpcm::SampleLoop>,
    pub policy: LoopPolicy,
}

impl LoopReport {
    pub fn preserved(sample_loop: Option<adpcm::SampleLoop>) -> Self {
        Self {
            original: sample_loop,
            output: sample_loop,
            policy: LoopPolicy::PreserveOriginalEncoding,
        }
    }
}

pub struct Rebuilt {
    pub bytes: Vec<u8>,
    pub changed: bool,
    pub encoding: Option<encoder::Report>,
    pub preserved_tail_bytes: usize,
    pub replaced_original_tail_bytes: usize,
    pub loop_reconstruction: LoopReport,
}

pub fn rebuild(original: &[u8], pcm: &[i16], sample_loop: Option<Range<usize>>) -> Result<Rebuilt> {
    let decoded = adpcm::decode_sample(original)?;
    let original_loop = decoded
        .sample_loop
        .map(|r| r.start_frame..r.end_frame_exclusive);
    if decoded.pcm == pcm && original_loop == sample_loop {
        return Ok(Rebuilt {
            bytes: original.to_vec(),
            changed: false,
            encoding: None,
            preserved_tail_bytes: decoded.trailing_bytes,
            replaced_original_tail_bytes: 0,
            loop_reconstruction: LoopReport::preserved(decoded.sample_loop),
        });
    }
    if pcm.is_empty() {
        return Err(
            "pack: deleting a nonempty sample requires allocation changes; use explicit silence"
                .into(),
        );
    }
    let encoded = encoder::encode(
        pcm,
        &encoder::Options {
            sample_loop: sample_loop.clone(),
            capacity_bytes: Some(original.len()),
            ..Default::default()
        },
    )?;
    let mut bytes = original.to_vec();
    bytes[..encoded.bytes.len()].copy_from_slice(&encoded.bytes);
    let check = adpcm::decode_sample(&bytes)?;
    let expected = adpcm::decode_sample(&encoded.bytes)?;
    if check.pcm != expected.pcm
        || check
            .sample_loop
            .map(|r| r.start_frame..r.end_frame_exclusive)
            != sample_loop
    {
        return Err("pack: rebuilt sample failed decode/loop verification".into());
    }
    Ok(Rebuilt {
        bytes,
        changed: true,
        preserved_tail_bytes: original.len() - encoded.bytes.len(),
        replaced_original_tail_bytes: encoded.bytes.len().saturating_sub(decoded.decoded_bytes),
        encoding: Some(encoded.report),
        loop_reconstruction: LoopReport {
            original: decoded.sample_loop,
            output: check.sample_loop,
            policy: if check.sample_loop.is_some() {
                LoopPolicy::StabilizePcmLoop
            } else {
                LoopPolicy::ReencodeWithoutLoop
            },
        },
    })
}
