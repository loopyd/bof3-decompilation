//! Evaluate delivery encoders against retained PCM; this is not a production renderer.

use bof3_audio::{
    codec::{mp3, rate, vorbis},
    digest::sha256_hex,
    interchange::wave::Wave,
    Result,
};
use serde_json::{json, Value};
use std::{fs, path::Path, time::Instant};

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if !matches!(args.len(), 2 | 3) {
        return Err("usage: encoding INPUT_WAV NEW_OUTPUT_DIR [TARGET_RATE]".into());
    }
    let input = fs::read(&args[0])?;
    let source = Wave::from_bytes(&input)?;
    let target_rate = args
        .get(2)
        .map(|value| -> Result<u32> {
            Ok(value.to_str().ok_or("target rate must be UTF-8")?.parse()?)
        })
        .transpose()?;
    let converted = target_rate
        .map(|target| rate::convert(&source.pcm, source.channels, source.sample_rate, target))
        .transpose()?;
    let (wave, conversion) = match converted {
        Some(converted) => (
            Wave::new(source.channels, converted.report.output_rate, converted.pcm)?,
            Some(converted.report),
        ),
        None => (source, None),
    };
    if wave.frames() == 0 {
        return Err("encoder evaluation needs nonempty PCM".into());
    }
    let output = Path::new(&args[1]);
    fs::create_dir(output)?;
    fs::write(output.join("source.wav"), &input)?;
    let reference = wave.to_bytes()?;
    fs::write(output.join("reference.wav"), &reference)?;
    let mut results = Vec::new();
    for codec in ["mp3", "ogg"] {
        let started = Instant::now();
        let encoded = match codec {
            "mp3" => mp3::encode(&wave, 192).map(|encoded| (encoded.bytes, json!(encoded.timing))),
            _ => vorbis::encode(&wave, 0.7).map(|encoded| (encoded.bytes, json!(encoded.stream))),
        };
        let row: Value = match encoded {
            Ok((bytes, timing)) => {
                fs::write(output.join(format!("encoded.{codec}")), &bytes)?;
                json!({"format": codec, "bytes": bytes.len(),
                    "sha256": sha256_hex(&bytes), "seconds": started.elapsed().as_secs_f64(),
                    "timing": timing})
            }
            Err(error) => json!({"format": codec, "error": error.to_string(),
                "seconds": started.elapsed().as_secs_f64()}),
        };
        results.push(row);
    }
    let failed = results.iter().any(|row| row.get("error").is_some());
    let report = json!({
        "schema": "bof3.audio.delivery-evaluation/v3",
        "source_sha256": sha256_hex(&input),
        "reference_sha256": sha256_hex(&reference), "conversion": conversion,
        "sample_rate": wave.sample_rate, "channels": wave.channels, "frames": wave.frames(),
        "mp3": {"version": "0.1.3", "bitrate_kbps": 192, "preset": "High",
            "delay_signaling": true, "extra_flush_frames": 1152,
            "encoder_identifier": "OxAV0.1.3"},
        "vorbis": {"version": "0.0.12", "quality": 0.7},
        "results": results, "production_acceptance": false,
        "remaining": ["independent decoding", "fidelity", "delay and padding", "presentation duration"]
    });
    let report = serde_json::to_string_pretty(&report)?;
    fs::write(output.join("encoding.json"), &report)?;
    println!("{report}");
    if failed {
        return Err(
            "delivery encoder evaluation failed; retained diagnostics are not acceptance".into(),
        );
    }
    Ok(())
}
