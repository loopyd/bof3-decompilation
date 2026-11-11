//! Decode final delivery bytes and measure error without fitted timing or gain.

use super::{Encoded, DECODER_DELAY, ENCODER_DELAY, FRAME_SAMPLES};
use crate::{digest::sha256_hex, interchange::wave::Wave, Result};
use oxideav_core::{Decoder, Demuxer, Error, Frame};
use oxideav_mp3::{make_decoder, Mp3Demuxer};
use serde::Serialize;
use std::io::Cursor;

const MINIMUM_SNR_DB: f64 = 30.0;
const MAXIMUM_GAIN_ERROR: f64 = 0.01;
const MAXIMUM_SILENT_PEAK: i32 = 1;

#[derive(Debug, Serialize)]
pub struct Channel {
    pub channel: usize,
    pub silent: bool,
    pub exact: bool,
    pub snr_db: Option<f64>,
    pub gain: Option<f64>,
    pub rms_error: f64,
    pub peak_error: i32,
    pub accepted: bool,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub decoder: &'static str,
    pub minimum_snr_db: f64,
    pub maximum_gain_error: f64,
    pub maximum_silent_peak: i32,
    pub packets: usize,
    pub gross_frames: usize,
    pub leading_trim: usize,
    pub trailing_trim: usize,
    pub presentation_frames: usize,
    pub decoded_pcm_sha256: String,
    pub channels: Vec<Channel>,
}

fn drain(
    decoder: &mut dyn Decoder,
    channels: usize,
    final_drain: bool,
    limit: usize,
    pcm: &mut Vec<i16>,
) -> Result<()> {
    loop {
        match decoder.receive_frame() {
            Ok(Frame::Audio(frame)) => {
                let frames = frame.samples as usize;
                let plane_bytes = frames.checked_mul(2).ok_or("MP3 decoded size overflow")?;
                let samples = frames
                    .checked_mul(channels)
                    .ok_or("MP3 decoded size overflow")?;
                if frames == 0
                    || frame.data.len() != channels
                    || frame.data.iter().any(|p| p.len() != plane_bytes)
                    || samples > limit - pcm.len()
                {
                    return Err("MP3 decoder returned an invalid PCM shape or length".into());
                }
                for i in 0..frames {
                    for plane in &frame.data {
                        pcm.push(i16::from_le_bytes([plane[i * 2], plane[i * 2 + 1]]));
                    }
                }
            }
            Ok(_) => return Err("MP3 decoder returned non-audio output".into()),
            Err(Error::NeedMore) if !final_drain => return Ok(()),
            Err(Error::Eof) if final_drain => return Ok(()),
            Err(error) => return Err(error.into()),
        }
    }
}

pub(super) fn validate(wave: &Wave, encoded: &Encoded) -> Result<Report> {
    let timing = &encoded.timing;
    let stream = super::stream::inspect(&encoded.bytes)?;
    let audio_frames = usize::try_from(timing.audio_frames)?;
    if stream.sample_rate != wave.sample_rate
        || stream.channels != wave.channels
        || stream.bitrate_kbps != 192
        || stream.mpeg_frames as usize != audio_frames + 1
        || timing.input_frames != wave.frames()
    {
        return Err("MP3 delivery stream differs from its declared presentation".into());
    }
    let channels = usize::from(wave.channels);
    let gross_frames = audio_frames
        .checked_mul(FRAME_SAMPLES)
        .ok_or("MP3 decoded size overflow")?;
    let limit = gross_frames
        .checked_mul(channels)
        .ok_or("MP3 decoded size overflow")?;
    let leading_trim = ENCODER_DELAY + DECODER_DELAY;
    let trailing_trim = timing
        .encoder_padding
        .checked_sub(DECODER_DELAY)
        .ok_or("MP3 invalid trailing trim")?;
    if gross_frames
        .checked_sub(leading_trim)
        .and_then(|n| n.checked_sub(trailing_trim))
        != Some(wave.frames())
    {
        return Err("MP3 gapless timing differs from input length".into());
    }
    let mut demux = Mp3Demuxer::open(Box::new(Cursor::new(encoded.bytes.clone())))?;
    if demux.streams().len() != 1 {
        return Err("MP3 delivery requires exactly one audio stream".into());
    }
    let params = demux.streams()[0].params.clone();
    if params.sample_rate != Some(wave.sample_rate) || params.channels != Some(wave.channels) {
        return Err("MP3 demuxed format differs from input".into());
    }
    let mut decoder = make_decoder(&params)?;
    let mut pcm = Vec::with_capacity(limit);
    let mut packets = 0;
    loop {
        let packet = match demux.next_packet() {
            Ok(packet) => packet,
            Err(Error::Eof) => break,
            Err(error) => return Err(error.into()),
        };
        packets += 1;
        if packets > audio_frames {
            return Err("MP3 delivery has excess audio packets".into());
        }
        decoder.send_packet(&packet)?;
        drain(decoder.as_mut(), channels, false, limit, &mut pcm)?;
    }
    decoder.flush()?;
    drain(decoder.as_mut(), channels, true, limit, &mut pcm)?;
    if packets != audio_frames || pcm.len() != limit {
        return Err("MP3 decoded format or gross frame count differs".into());
    }
    let decoded = &pcm[leading_trim * channels..(gross_frames - trailing_trim) * channels];
    let mut measurements = Vec::with_capacity(channels);
    for channel in 0..channels {
        let (mut signal, mut error, mut cross) = (0.0, 0.0, 0.0);
        let mut peak_error = 0;
        for i in (channel..wave.pcm.len()).step_by(channels) {
            let source = f64::from(wave.pcm[i]);
            let actual = f64::from(decoded[i]);
            signal += source * source;
            error += (actual - source).powi(2);
            cross += source * actual;
            peak_error = peak_error.max((i32::from(decoded[i]) - i32::from(wave.pcm[i])).abs());
        }
        let snr_db = (signal > 0.0 && error > 0.0).then(|| 10.0 * (signal / error).log10());
        let gain = (signal > 0.0).then(|| cross / signal);
        let accepted = if signal == 0.0 {
            peak_error <= MAXIMUM_SILENT_PEAK
        } else {
            (error == 0.0 || snr_db.is_some_and(|snr| snr >= MINIMUM_SNR_DB))
                && gain.is_some_and(|gain| (gain - 1.0).abs() <= MAXIMUM_GAIN_ERROR)
        };
        measurements.push(Channel {
            channel,
            silent: signal == 0.0,
            exact: error == 0.0,
            snr_db,
            gain,
            rms_error: (error / wave.frames() as f64).sqrt(),
            peak_error,
            accepted,
        });
    }
    if measurements.iter().any(|channel| !channel.accepted) {
        return Err(format!(
            "MP3 delivery quality failed; no output published: {}",
            serde_json::to_string(&measurements)?
        )
        .into());
    }
    let bytes = decoded
        .iter()
        .flat_map(|v| v.to_le_bytes())
        .collect::<Vec<_>>();
    Ok(Report {
        decoder: "oxideav-mp3 0.1.3 / oxideav-core 0.1.36",
        minimum_snr_db: MINIMUM_SNR_DB,
        maximum_gain_error: MAXIMUM_GAIN_ERROR,
        maximum_silent_peak: MAXIMUM_SILENT_PEAK,
        packets,
        gross_frames,
        leading_trim,
        trailing_trim,
        presentation_frames: wave.frames(),
        decoded_pcm_sha256: sha256_hex(&bytes),
        channels: measurements,
    })
}
