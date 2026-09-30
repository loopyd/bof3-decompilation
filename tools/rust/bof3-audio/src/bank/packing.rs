//! Parse WAV sample edits and encode only supported changes within an allocation.
use crate::{
    codec::adpcm, codec::adpcm::encoder, digest::sha256_hex, document::manifest::Element,
    interchange::wave, Result,
};
use serde::Serialize;
use serde_json::json;

#[derive(Debug, Serialize)]
pub struct Report {
    pub sample_id: u16,
    pub changed: bool,
    pub input_wav_sha256: String,
    pub original_encoded_sha256: String,
    pub output_encoded_sha256: String,
    pub preserved_tail_bytes: usize,
    pub replaced_original_tail_bytes: usize,
    pub encoding: Option<encoder::Report>,
}

#[derive(Debug)]
pub struct Packed {
    pub bytes: Vec<u8>,
    pub report: Report,
}

/// Original metadata remains evidence; PCM and `smpl` loops are edited in WAV.
pub fn pack(
    node: &Element,
    sample: &crate::bank::Sample,
    original: &[u8],
    rate: u32,
    wav_bytes: &[u8],
) -> Result<Packed> {
    node.shape(
        &[
            "sample_id",
            "body_offset",
            "encoded_bytes",
            "path",
            "encoded_sha256",
            "wav_sha256",
            "frames",
            "decoded_bytes",
            "trailing_bytes",
            "termination",
        ],
        &["sample_loop"],
        false,
    )?;
    if node.name != "sample" || original.len() != sample.encoded_bytes {
        return Err("pack: invalid sample allocation".into());
    }
    let decoded = adpcm::decode_sample(original)?;
    let mut expected = wave::Wave::new(1, rate, decoded.pcm)?;
    let mut sampler = wave::Sampler::new(rate, 60)?;
    if let Some(loop_) = decoded.sample_loop {
        sampler.loops.push(wave::SampleLoop::forward(
            u32::try_from(loop_.start_frame)?,
            u32::try_from(loop_.end_frame_exclusive)?,
        )?);
        let metadata = node.child("sample_loop")?;
        metadata.scalars(&loop_, &[], &[])?;
        metadata.shape(
            &["start_frame", "end_frame_exclusive", "pcm_repeat_is_stable"],
            &[],
            false,
        )?;
    } else if !node.children.is_empty() {
        return Err(
            "pack: XML sample loop conflicts with preserved encoding; edit the WAV smpl loop"
                .into(),
        );
    }
    expected.sampler = Some(sampler);
    node.attribute("path")?;
    let original_hash = sha256_hex(original);
    node.scalars(&json!({
        "sample_id": sample.sample_id, "body_offset": sample.body_offset, "encoded_bytes": sample.encoded_bytes,
        "encoded_sha256": original_hash, "wav_sha256": sha256_hex(&expected.to_bytes()?),
        "frames": expected.frames(), "decoded_bytes": decoded.decoded_bytes,
        "trailing_bytes": decoded.trailing_bytes, "termination": decoded.termination,
    }), &[], &["path"])?;
    let wav = wave::Wave::from_bytes(wav_bytes)?;
    if wav.channels != 1 || wav.sample_rate != rate {
        return Err("pack: sample WAV must remain mono PCM16 at its XML export reference rate; resampling is unsupported".into());
    }
    if wav.opaque_chunks().next().is_some() || wav.has_extended_fact() {
        return Err("pack: ancillary WAV metadata cannot be represented in a VAB; remove unsupported chunks explicitly".into());
    }
    if let Some(actual) = &wav.sampler {
        let mut basic = actual.clone();
        basic.loops.clear();
        if basic != wave::Sampler::new(rate, 60)? {
            return Err(
                "pack: edited WAV smpl pitch, period, vendor, timecode or sampler data unsupported"
                    .into(),
            );
        }
    }
    let loop_ = wav.single_forward_loop()?;
    if loop_.is_some_and(|l| l.identifier != 0) {
        return Err("pack: nonzero WAV loop identifier cannot be represented in VAB".into());
    }
    let range = loop_.map(|l| l.start_frame as usize..l.end_frame_exclusive as usize);
    let rebuilt = crate::sample::editing::rebuild(original, &wav.pcm, range)?;
    Ok(Packed {
        report: Report {
            sample_id: sample.sample_id,
            changed: rebuilt.changed,
            input_wav_sha256: sha256_hex(wav_bytes),
            original_encoded_sha256: original_hash,
            output_encoded_sha256: sha256_hex(&rebuilt.bytes),
            preserved_tail_bytes: rebuilt.preserved_tail_bytes,
            replaced_original_tail_bytes: rebuilt.replaced_original_tail_bytes,
            encoding: rebuilt.encoding,
        },
        bytes: rebuilt.bytes,
    })
}
