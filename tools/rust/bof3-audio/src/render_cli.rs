//! PC interchange and original-runtime rendering with verified publication.
use crate::{
    interchange::wave::Wave,
    machine::{executable::Executable, firmware::Image},
    pc_archive,
    pc_render::{self, Options},
    psx_render,
    publication::{self, Publication},
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{ffi::OsString, path::PathBuf};

pub const HELP: &str = "
  render --mode music --engine pc --midi FILE --soundfont FILE --output NEW_DIR
    [--sample-rate HZ] [--repeats N] [--duration SECONDS] [--tail SECONDS]
  [--timeout SECONDS] [--json]
render --mode music --engine pc --archive EMI --executable PSX_EXE
  --sequence INDEX --allow-approximations --output NEW_DIR
  [--duration SECONDS | --loops N] [--sample-rate HZ] [--tail SECONDS]
  [--timeout SECONDS] [--json]
  render --mode music --engine psx --archive EMI --executable PSX_EXE --bios ROM
    --sequence INDEX --output NEW_DIR [--duration SECONDS | --loops N] [--layout 0|1|2]
    [--tail SECONDS] [--timeout SECONDS] [--json]
  Writes render.wav and render.json after validation. Output must not exist.
PC MIDI defaults: 44100 Hz, one file play; explicit --repeats resets the synth.
PC archive defaults: two infinite-loop traversals, intro once, finite loops retained.
Archive translation requires --allow-approximations; only the selected sequence plays.
  Both engines default to a 2-second tail and 600-second audio safety limit.
  PSX layout defaults to the first game layout that fits the archive; --layout overrides it.
PSX uses the verified US profile and explicit reference clocks at 44100 Hz.
  PSX defaults to two infinite-loop traversals or the first end marker, with encoded finite loops retained.
--duration sets a fixed body length instead of --loops. Independent PCM fidelity remains open.
  MP3/Ogg delivery and audio/SFX/XA rendering remain unsupported.
";

pub fn run(mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let (mut mode, mut engine) = (None, None);
    let (mut midi, mut font, mut output) = (None, None, None);
    let (mut archive, mut executable, mut bios) = (None, None, None);
    let (mut sequence, mut layout, mut loops) = (None, None, None);
    let (mut rate, mut repeats) = (None, None);
    let (mut duration, mut tail, mut timeout) = (None, None, None);
    let mut json = false;
    let mut allow_approximations = false;
    while let Some(arg) = args.next() {
        let flag = arg.to_str().ok_or("render option must be UTF-8")?;
        match flag {
            "--mode" if mode.is_none() => mode = Some(string(&mut args, flag)?),
            "--engine" if engine.is_none() => engine = Some(string(&mut args, flag)?),
            "--midi" if midi.is_none() => midi = Some(PathBuf::from(value(&mut args, flag)?)),
            "--soundfont" if font.is_none() => font = Some(PathBuf::from(value(&mut args, flag)?)),
            "--output" if output.is_none() => output = Some(PathBuf::from(value(&mut args, flag)?)),
            "--archive" if archive.is_none() => {
                archive = Some(PathBuf::from(value(&mut args, flag)?))
            }
            "--executable" if executable.is_none() => {
                executable = Some(PathBuf::from(value(&mut args, flag)?))
            }
            "--bios" if bios.is_none() => bios = Some(PathBuf::from(value(&mut args, flag)?)),
            "--sequence" if sequence.is_none() => {
                sequence = Some(string(&mut args, flag)?.parse()?)
            }
            "--loops" if loops.is_none() => loops = Some(string(&mut args, flag)?.parse()?),
            "--layout" if layout.is_none() => layout = Some(string(&mut args, flag)?.parse()?),
            "--sample-rate" if rate.is_none() => rate = Some(string(&mut args, flag)?.parse()?),
            "--repeats" if repeats.is_none() => repeats = Some(string(&mut args, flag)?.parse()?),
            "--duration" if duration.is_none() => duration = Some(string(&mut args, flag)?),
            "--tail" if tail.is_none() => tail = Some(string(&mut args, flag)?),
            "--timeout" if timeout.is_none() => timeout = Some(string(&mut args, flag)?),
            "--json" if !json => json = true,
            "--allow-approximations" if !allow_approximations => allow_approximations = true,
            _ => return Err(format!("unknown or duplicate render option {flag:?}").into()),
        }
    }
    if mode.as_deref() != Some("music") || !matches!(engine.as_deref(), Some("pc" | "psx")) {
        return Err("render requires --mode music and --engine pc|psx; other rendering modes are not implemented".into());
    }
    let output = output.ok_or("render requires --output NEW_DIR")?;
    publication::require_absent(&output)?;
    let rate = rate.unwrap_or(44100);
    let release_frames = frames(tail.as_deref().unwrap_or("2"), rate)?;
    let safety_frames = frames(timeout.as_deref().unwrap_or("600"), rate)?;
    let duration_frames = duration.as_deref().map(|s| frames(s, rate)).transpose()?;
    let (wave, report) = if let (Some("pc"), Some(path)) = (engine.as_deref(), archive.as_ref()) {
        if midi.is_some()
            || font.is_some()
            || bios.is_some()
            || layout.is_some()
            || repeats.is_some()
        {
            return Err("PC archive render accepts --loops or --duration; MIDI/SoundFont/--repeats are a separate input mode, BIOS/layout are PSX-only".into());
        }
        let options = pc_archive::Options {
            sequence: sequence
                .ok_or("PC archive render requires --sequence INDEX, qualified by --archive")?,
            loops,
            allow_approximations,
            playback: Options {
                sample_rate: rate,
                repeats: 1,
                duration_frames,
                release_frames,
                safety_frames,
            },
        };
        options.validate()?;
        let executable = Executable::from_bytes(std::fs::read(
            executable.ok_or("PC archive render requires --executable")?,
        )?)?;
        let image = ArchiveImage::from_bytes(std::fs::read(path)?)?;
        let rendered = pc_archive::render(&executable, &path.to_string_lossy(), &image, &options)?;
        (rendered.wave, serde_json::to_value(rendered.report)?)
    } else if engine.as_deref() == Some("pc") {
        if executable.is_some()
            || bios.is_some()
            || sequence.is_some()
            || layout.is_some()
            || loops.is_some()
            || allow_approximations
        {
            return Err("PC render accepts MIDI/SoundFont inputs; archive, executable, BIOS, sequence, loop and layout options belong to PSX rendering".into());
        }
        let rendered = pc_render::render(
            &std::fs::read(midi.ok_or("render requires --midi")?)?,
            &std::fs::read(font.ok_or("render requires --soundfont")?)?,
            &Options {
                sample_rate: rate,
                repeats: repeats.unwrap_or(1),
                duration_frames,
                release_frames,
                safety_frames,
            },
        )?;
        (rendered.wave, serde_json::to_value(rendered.report)?)
    } else {
        if midi.is_some() || font.is_some() || repeats.is_some() || allow_approximations {
            return Err("PSX render accepts an archive; MIDI/SoundFont and whole-file --repeats are PC options. use --loops for PSX sequence loops".into());
        }
        if rate != psx_render::SAMPLE_RATE {
            return Err("PSX render requires 44100 Hz output".into());
        }
        let options = psx_render::Options {
            sequence: sequence
                .ok_or("PSX render requires --sequence INDEX, qualified by --archive")?,
            layout,
            body: match (duration_frames, loops) {
                (Some(_), Some(_)) => {
                    return Err("PSX render: --duration and --loops are mutually exclusive".into())
                }
                (Some(frames), None) => psx_render::Body::Duration(frames),
                (None, count) => psx_render::Body::Loops(count.unwrap_or(2)),
            },
            release_frames,
            safety_frames,
        };
        options.validate()?;
        let executable = Executable::from_bytes(std::fs::read(
            executable.ok_or("PSX render requires --executable")?,
        )?)?;
        let bios = Image::from_bytes(std::fs::read(bios.ok_or("PSX render requires --bios")?)?)?;
        let archive = ArchiveImage::from_bytes(std::fs::read(
            archive.ok_or("PSX render requires --archive")?,
        )?)?;
        let rendered = psx_render::render(&executable, bios, &archive, &options)?;
        (rendered.wave, serde_json::to_value(rendered.report)?)
    };
    let bytes = wave.to_bytes()?;
    let report_bytes = serde_json::to_vec_pretty(&report)?;
    let transaction = Publication::new(&output)?;
    let wave_path = transaction.staging.join("render.wav");
    publication::write_new(&wave_path, &bytes)?;
    publication::write_new(&transaction.staging.join("render.json"), &report_bytes)?;
    if Wave::from_bytes(&std::fs::read(wave_path)?)? != wave {
        return Err("render: written WAV did not verify; output not published".into());
    }
    if std::fs::read(transaction.staging.join("render.json"))? != report_bytes {
        return Err("render: written report did not verify; output not published".into());
    }
    transaction.publish()?;
    if json {
        println!("{}", String::from_utf8(report_bytes)?);
    } else {
        println!(
            "{}: {} stereo frames at {} Hz",
            output.display(),
            wave.frames(),
            rate
        );
        println!(
            "Body: {} frames; tail: {}; audio safety limit: {}",
            report["body_frames"], report["release_frames"], report["safety_frames"]
        );
        if let Some(limitations) = report["limitations"].as_array() {
            for limitation in limitations {
                println!("Note: {}", limitation.as_str().unwrap_or(""));
            }
        }
    }
    Ok(0)
}

fn frames(seconds: &str, rate: u32) -> Result<u64> {
    let value: f64 = seconds.parse()?;
    let frames = value * f64::from(rate);
    if !value.is_finite() || value < 0.0 || frames >= u64::MAX as f64 {
        return Err("render: seconds must be finite, nonnegative and representable".into());
    }
    Ok(frames.round() as u64)
}
fn value(args: &mut impl Iterator<Item = OsString>, flag: &str) -> Result<OsString> {
    let value = args.next().ok_or_else(|| format!("{flag} needs a value"))?;
    if value.to_str().is_some_and(|s| s.starts_with("--")) {
        return Err(format!("{flag} needs a value").into());
    }
    Ok(value)
}
fn string(args: &mut impl Iterator<Item = OsString>, flag: &str) -> Result<String> {
    value(args, flag)?
        .into_string()
        .map_err(|_| format!("{flag} needs UTF-8").into())
}
