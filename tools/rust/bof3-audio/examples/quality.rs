//! Compare published MP3 quality controls with an independent development decoder.

#[cfg(target_os = "linux")]
#[path = "../tests/codec/consumer.rs"]
mod consumer;

#[cfg(target_os = "linux")]
fn main() -> bof3_audio::Result<()> {
    use bof3_audio::{codec::mp3, digest::sha256_hex, interchange::wave::Wave};
    use oxideav_mp3::{
        frame::ChannelMode,
        quality::QualityPreset,
        stream_encoder::{Mp3Encoder, DEFAULT_OUTER_LOOP_THRESHOLD},
    };
    use serde_json::json;
    use std::{fs, path::Path};

    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 2 {
        return Err("usage: quality INPUT_WAV|--impulse NEW_OUTPUT_DIR".into());
    }
    let wave = if args[0] == "--impulse" {
        let mut pcm = vec![0; 8193 * 2];
        for ch in 0..2 {
            pcm[ch] = 22000 - ch as i16 * 6000;
            pcm[8192 * 2 + ch] = pcm[ch];
        }
        Wave::new(2, 48000, pcm)?
    } else {
        Wave::from_bytes(&fs::read(&args[0])?)?
    };
    let output = Path::new(&args[1]);
    fs::create_dir(output)?;
    let count = usize::from(wave.channels);
    let mode = match count {
        1 => ChannelMode::SingleChannel,
        2 => ChannelMode::Stereo,
        _ => return Err("quality probe requires mono/stereo PCM".into()),
    };
    fs::write(output.join("reference.wav"), wave.to_bytes()?)?;
    let mut rows = Vec::new();
    for bitrate in [128, 192, 256, 320] {
        // Apply the production configuration guard; a rejected configuration
        // is recorded explicitly, never substituted or sent to the encoder.
        if let Err(error) = mp3::encode(&wave, bitrate) {
            rows.push(json!({"bitrate":bitrate,"error":error.to_string()}));
            continue;
        }
        for preset in [
            QualityPreset::Fast,
            QualityPreset::Standard,
            QualityPreset::High,
            QualityPreset::Transparent,
        ] {
            let mut encoder = Mp3Encoder::new_with_outer_loop(
                bitrate,
                wave.sample_rate,
                mode,
                DEFAULT_OUTER_LOOP_THRESHOLD,
            )?;
            encoder.with_quality_preset(preset)?;
            encoder.push_samples(&wave.pcm)?;
            encoder.push_samples(&vec![0; 1152 * count])?;
            let mut bytes = Vec::new();
            encoder.finish(&mut bytes)?;
            let name = format!("{bitrate}-{preset:?}.mp3");
            let path = output.join(&name);
            fs::write(&path, &bytes)?;
            let structure = match mp3::stream::inspect(&bytes) {
                Ok(stream) => stream,
                Err(error) => {
                    rows.push(json!({"bitrate":bitrate,"preset":format!("{preset:?}"),
                        "path":name,"sha256":sha256_hex(&bytes),
                        "structure_error":error.to_string()}));
                    continue;
                }
            };
            // Raw frames have no gapless tag. Align by independently measured
            // encoder + decoder delay, without optimizing alignment per clip.
            let decoded = consumer::decode(&path);
            if decoded.rate != wave.sample_rate || decoded.channels != wave.channels {
                return Err("independent decoder changed the PCM format".into());
            }
            let start = (mp3::ENCODER_DELAY + mp3::DECODER_DELAY) * count;
            let pcm = decoded
                .pcm
                .get(start..start + wave.pcm.len())
                .ok_or("raw MP3 does not cover the complete aligned reference")?;
            let channels: Vec<_> = (0..count).map(|ch| {
                let mut signal = 0f64;
                let mut error = 0f64;
                let mut cross = 0f64;
                for frame in 0..wave.frames() {
                    let source = f64::from(wave.pcm[frame * count + ch]);
                    let value = f64::from(pcm[frame * count + ch]);
                    signal += source * source;
                    error += (value - source).powi(2);
                    cross += source * value;
                }
                let tail: Vec<_> = (wave.frames().saturating_sub(32)..wave.frames())
                    .map(|frame| pcm[frame * count + ch]).collect();
                json!({"channel":ch,"first":pcm[ch],
                    "last":pcm[(wave.frames()-1)*count+ch],"tail":tail,
                    "snr_db":if error>0.0 && signal>0.0 {Some(10.0*(signal/error).log10())} else {None},
                    "gain":if signal>0.0 {Some(cross/signal)} else {None},
                    "rms_error":(error/wave.frames() as f64).sqrt()})
            }).collect();
            rows.push(json!({"bitrate":bitrate,"preset":format!("{preset:?}"),
                "path":name,"sha256":sha256_hex(&bytes),"bytes":bytes.len(),
                "structure":structure,"raw_frames":decoded.pcm.len()/count,"channels":channels}));
        }
    }
    let reference: Vec<_> = wave
        .pcm
        .iter()
        .flat_map(|sample| sample.to_le_bytes())
        .collect();
    let report = json!({"schema":"bof3.audio.mp3-quality/v1",
        "reference_pcm_sha256":sha256_hex(&reference),"sample_rate":wave.sample_rate,
        "channels":wave.channels,"frames":wave.frames(),"encoder":"oxideav-mp3 0.1.3",
        "decoder":"installed libmpg123.so.0","alignment_frames":1057,
        "production_acceptance":false,"results":rows});
    let report = serde_json::to_string_pretty(&report)?;
    fs::write(output.join("quality.json"), &report)?;
    println!("{report}");
    Ok(())
}

#[cfg(not(target_os = "linux"))]
fn main() {
    eprintln!("quality probe requires the installed Linux mpg123 development oracle");
    std::process::exit(1);
}
