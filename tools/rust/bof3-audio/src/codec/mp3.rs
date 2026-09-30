//! MPEG-1 delivery encoding with complete filter tails and explicit gapless timing.

pub mod stream;

use crate::{interchange::wave::Wave, Result};
use oxideav_mp3::{
    demuxer::XingTagId,
    encoder::make_silent_header,
    frame::ChannelMode,
    quality::QualityPreset,
    stream_encoder::{Mp3Encoder, DEFAULT_OUTER_LOOP_THRESHOLD},
    xing_info::{build_info_frame, flag_bit, XingTagSpec},
};
use serde::Serialize;

/// The pinned encoder's filterbank/MDCT delay, excluding decoder synthesis.
pub const ENCODER_DELAY: usize = 528;
/// Standard Layer III synthesis delay, independently checked with mpg123/FFmpeg.
pub const DECODER_DELAY: usize = 529;
const FRAME_SAMPLES: usize = 1152;
const ENCODER: &[u8; 9] = b"OxAV0.1.3";

#[derive(Debug, Serialize)]
pub struct Timing {
    pub input_frames: usize,
    pub audio_frames: u32,
    pub encoder_delay: usize,
    pub encoder_padding: usize,
    pub decoder_delay: usize,
    pub flush_frames: usize,
    pub bitrate_kbps: u32,
}

#[derive(Debug)]
pub struct Encoded {
    pub bytes: Vec<u8>,
    pub timing: Timing,
}

/// Encode without resampling. Consumer support for gapless tags is separate
/// from MPEG conformance; FFmpeg 6.1.1 ignores the truthful OxideAV identifier.
pub fn encode(wave: &Wave, bitrate_kbps: u32) -> Result<Encoded> {
    let mode = match wave.channels {
        1 => ChannelMode::SingleChannel,
        2 => ChannelMode::Stereo,
        _ => return Err("MP3 requires one or two PCM channels".into()),
    };
    if !matches!(wave.sample_rate, 32000 | 44100 | 48000) {
        return Err(format!(
            "MP3 delivery requires 32000, 44100 or 48000 Hz; {} Hz needs explicit rate conversion",
            wave.sample_rate
        )
        .into());
    }
    if wave.pcm.is_empty() || !wave.pcm.len().is_multiple_of(usize::from(wave.channels)) {
        return Err("MP3 requires nonempty, complete interleaved PCM frames".into());
    }
    if !matches!(bitrate_kbps, 128 | 192 | 256 | 320) {
        return Err("MP3 delivery bitrate must be 128, 192, 256 or 320 kbit/s".into());
    }
    let header = make_silent_header(bitrate_kbps, wave.sample_rate, mode)?;
    let carrier_size = header
        .frame_len()
        .ok_or("MP3 carrier has no fixed length")?;
    // The unchanged 0.1.3 encoder budgets from frame capacity without capping
    // the 12-bit part2_3_length. Reject that configuration before it can emit
    // truncated side information. Never substitute a lower bitrate silently.
    let side_bytes = if wave.channels == 1 { 17 } else { 32 };
    let budget = (carrier_size - 4 - side_bytes) * 8 / (2 * usize::from(wave.channels)) - 16;
    if budget > 4095 {
        return Err(format!(
            "oxideav-mp3 0.1.3 cannot safely encode {} Hz, {} channel(s), {} kbit/s: its {}-bit granule budget exceeds the 4095-bit length field; choose an explicit representable bitrate or rate",
            wave.sample_rate, wave.channels, bitrate_kbps, budget
        ).into());
    }
    let input_frames = wave.frames();
    // One granule pair covers the 1057-frame pipeline delay and ensures at least
    // two audio frames even for a one-sample input. It is removed by the tag.
    let audio_frames: u32 = input_frames
        .checked_add(FRAME_SAMPLES)
        .ok_or("MP3 input frame count overflow")?
        .div_ceil(FRAME_SAMPLES)
        .try_into()?;
    let encoder_padding = usize::try_from(audio_frames)?
        .checked_mul(FRAME_SAMPLES)
        .and_then(|n| n.checked_sub(input_frames + ENCODER_DELAY))
        .ok_or("MP3 padding calculation overflow")?;
    if !(DECODER_DELAY..=4095).contains(&encoder_padding) {
        return Err("MP3 padding cannot be represented by the gapless tag".into());
    }
    let mut encoder = Mp3Encoder::new_with_outer_loop(
        bitrate_kbps,
        wave.sample_rate,
        mode,
        DEFAULT_OUTER_LOOP_THRESHOLD,
    )?;
    encoder.with_quality_preset(QualityPreset::High)?;
    encoder.push_samples(&wave.pcm)?;
    encoder.push_samples(&vec![0; FRAME_SAMPLES * usize::from(wave.channels)])?;
    let mut audio = Vec::new();
    encoder.finish(&mut audio)?;
    validate_frames(
        &audio,
        audio_frames,
        wave.sample_rate,
        wave.channels,
        bitrate_kbps,
    )?;

    let total_bytes: u32 = audio
        .len()
        .checked_add(carrier_size)
        .ok_or("MP3 byte count overflow")?
        .try_into()?;
    let spec = XingTagSpec {
        id: XingTagId::Info,
        flags: flag_bit::FRAMES | flag_bit::BYTES,
        frames: Some(audio_frames),
        bytes: Some(total_bytes),
        toc: None,
        quality: None,
    };
    let mut bytes = build_info_frame(&header, &spec)?;
    // Four header bytes, MPEG-1 side information, then the 16-byte Info payload.
    let extension = 4 + if wave.channels == 1 { 17 } else { 32 } + 16;
    let mut tag = [0u8; 36];
    tag[..9].copy_from_slice(ENCODER);
    tag[9] = 1; // Revision zero, constant bitrate.
    tag[20] = bitrate_kbps.min(255) as u8;
    tag[21] = (ENCODER_DELAY >> 4) as u8;
    tag[22] = ((ENCODER_DELAY << 4) | (encoder_padding >> 8)) as u8;
    tag[23] = encoder_padding as u8;
    tag[24] = (u8::from(wave.channels == 2) << 2)
        | (match wave.sample_rate {
            32000 => 0,
            44100 => 1,
            _ => 2,
        } << 6);
    tag[28..32].copy_from_slice(&total_bytes.to_be_bytes());
    tag[32..34].copy_from_slice(&crc(&audio).to_be_bytes());
    bytes
        .get_mut(extension..extension + tag.len())
        .ok_or("MP3 tag exceeds carrier capacity")?
        .copy_from_slice(&tag);
    let checksum = crc(&bytes[..extension + 34]);
    bytes[extension + 34..extension + 36].copy_from_slice(&checksum.to_be_bytes());
    bytes.extend(audio);
    Ok(Encoded {
        bytes,
        timing: Timing {
            input_frames,
            audio_frames,
            encoder_delay: ENCODER_DELAY,
            encoder_padding,
            decoder_delay: DECODER_DELAY,
            flush_frames: FRAME_SAMPLES,
            bitrate_kbps,
        },
    })
}

fn validate_frames(
    audio: &[u8],
    expected: u32,
    rate: u32,
    channels: u16,
    bitrate: u32,
) -> Result<()> {
    let inspected = stream::inspect(audio)?;
    if inspected.sample_rate != rate
        || inspected.channels != channels
        || inspected.bitrate_kbps != bitrate
    {
        return Err("MP3 encoder changed the requested audio format".into());
    }
    if inspected.mpeg_frames != expected {
        return Err("MP3 encoder frame count differs from input timing".into());
    }
    Ok(())
}

/// Reflected CRC-16/IBM, zero initial state, no final xor (Info extension CRCs).
fn crc(bytes: &[u8]) -> u16 {
    let mut value = 0u16;
    for &byte in bytes {
        value ^= u16::from(byte);
        for _ in 0..8 {
            value = (value >> 1) ^ if value & 1 != 0 { 0xa001 } else { 0 };
        }
    }
    value
}
