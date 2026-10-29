//! PC interchange and owned-driver rendering with verified publication.
use crate::{
    codec::vorbis,
    digest::sha256_hex,
    interchange::wave::Wave,
    machine::{driver::Image, effects::Note, executable::Executable},
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
  render --mode music --engine psx --archive EMI --driver DIRECTORY
    --sequence INDEX --output NEW_DIR [--duration SECONDS | --loops N] [--layout 0|1|2]
    [--tail SECONDS] [--timeout SECONDS] [--json]
  render --mode sfx --engine psx --archive EMI --driver DIRECTORY
    --header-entry INDEX --body-entry INDEX --history cold --duration SECONDS
    --voice V --program P --tone T --key K --fine F --left L --right R
    --output NEW_DIR [--layout 0|1|2] [--tail SECONDS] [--timeout SECONDS] [--json]
  For a local game cue, replace the seven note options with --auxiliary-entry INDEX --cue-row ROW.
  [--format wav|ogg] [--quality 0.7]
  Writes render.wav and render.json after validation, plus render.ogg for --format ogg.
Output must not exist. Vorbis quality is fixed at the independently measured 0.7.
PC MIDI defaults: 44100 Hz, one file play; explicit --repeats resets the synth.
PC archive defaults: two infinite-loop traversals, intro once, finite loops retained.
Archive translation requires --allow-approximations; only the selected sequence plays.
  Both engines default to a 2-second tail and 600-second audio safety limit.
  PSX layout defaults to the first game layout that fits the archive; --layout overrides it.
PSX uses the verified US profile and explicit reference clocks at 44100 Hz.
  PSX defaults to two infinite-loop traversals or the first end marker, with encoded finite loops retained.
--duration sets a fixed body length instead of --loops. Independent PCM fidelity remains open.
  SFX requires a cold bank and one explicit note or local cue; repeated histories and cross-bank cues are unsupported.
  MP3 delivery and vocal/XA rendering remain unsupported.
";

pub fn run(mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let (mut mode, mut engine) = (None, None);
    let (mut midi, mut font, mut output) = (None, None, None);
    let (mut archive, mut executable, mut driver) = (None, None, None);
    let (mut sequence, mut layout, mut loops) = (None, None, None);
    let (mut rate, mut repeats) = (None, None);
    let (mut duration, mut tail, mut timeout) = (None, None, None);
    let (mut format, mut quality) = (None, None);
    let (mut header_entry, mut body_entry, mut auxiliary_entry, mut cue_row) =
        (None, None, None, None);
    let (mut voice, mut program, mut tone, mut key, mut fine, mut left, mut right) =
        (None, None, None, None, None, None, None);
    let mut history = None;
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
            "--driver" if driver.is_none() => driver = Some(PathBuf::from(value(&mut args, flag)?)),
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
            "--format" if format.is_none() => format = Some(string(&mut args, flag)?),
            "--quality" if quality.is_none() => {
                quality = Some(string(&mut args, flag)?.parse::<f32>()?)
            }
            "--json" if !json => json = true,
            "--header-entry" if header_entry.is_none() => {
                header_entry = Some(string(&mut args, flag)?.parse()?)
            }
            "--body-entry" if body_entry.is_none() => {
                body_entry = Some(string(&mut args, flag)?.parse()?)
            }
            "--auxiliary-entry" if auxiliary_entry.is_none() => {
                auxiliary_entry = Some(string(&mut args, flag)?.parse()?)
            }
            "--cue-row" if cue_row.is_none() => cue_row = Some(string(&mut args, flag)?.parse()?),
            "--voice" if voice.is_none() => voice = Some(string(&mut args, flag)?.parse()?),
            "--program" if program.is_none() => program = Some(string(&mut args, flag)?.parse()?),
            "--tone" if tone.is_none() => tone = Some(string(&mut args, flag)?.parse()?),
            "--key" if key.is_none() => key = Some(string(&mut args, flag)?.parse()?),
            "--fine" if fine.is_none() => fine = Some(string(&mut args, flag)?.parse()?),
            "--left" if left.is_none() => left = Some(string(&mut args, flag)?.parse()?),
            "--right" if right.is_none() => right = Some(string(&mut args, flag)?.parse()?),
            "--history" if history.is_none() => history = Some(string(&mut args, flag)?),
            "--allow-approximations" if !allow_approximations => allow_approximations = true,
            _ => return Err(format!("unknown or duplicate render option {flag:?}").into()),
        }
    }
    let sfx = mode.as_deref() == Some("sfx");
    if !matches!(
        (mode.as_deref(), engine.as_deref()),
        (Some("music"), Some("pc" | "psx")) | (Some("sfx"), Some("psx"))
    ) {
        return Err("render requires --mode music --engine pc|psx, or --mode sfx --engine psx; other combinations are not implemented".into());
    }
    let note_options = [voice, program, tone, key, fine, left, right];
    if !sfx
        && (header_entry.is_some()
            || body_entry.is_some()
            || auxiliary_entry.is_some()
            || cue_row.is_some()
            || history.is_some()
            || note_options.iter().any(Option::is_some))
    {
        return Err("render: bank entry, note, cue and history options require --mode sfx".into());
    }
    let output = output.ok_or("render requires --output NEW_DIR")?;
    publication::require_absent(&output)?;
    let rate = rate.unwrap_or(44100);
    let ogg = match format.as_deref().unwrap_or("wav") {
        "wav" if quality.is_none() => false,
        "wav" => return Err("render: --quality requires --format ogg".into()),
        "ogg" => {
            if quality.is_some_and(|value| value != 0.7) {
                return Err("render: accepted Vorbis delivery quality is 0.7; other settings remain unaccepted".into());
            }
            if !matches!(rate, 32000 | 44100 | 48000) {
                return Err("render: Vorbis delivery supports 32000, 44100 or 48000 Hz without implicit rate conversion".into());
            }
            true
        }
        "mp3" => {
            return Err(
                "render: MP3 content quality remains unaccepted; delivery is disabled".into(),
            )
        }
        _ => return Err("render: --format must be wav or ogg".into()),
    };
    let release_frames = frames(tail.as_deref().unwrap_or("2"), rate)?;
    let safety_frames = frames(timeout.as_deref().unwrap_or("600"), rate)?;
    let duration_frames = duration.as_deref().map(|s| frames(s, rate)).transpose()?;
    let (wave, mut report) = if sfx {
        if midi.is_some()
            || font.is_some()
            || executable.is_some()
            || sequence.is_some()
            || loops.is_some()
            || repeats.is_some()
            || allow_approximations
        {
            return Err("PSX effects render accepts bank and playback options; MIDI, SoundFont, executable, sequence, loops, repeats and approximation options are invalid".into());
        }
        if rate != psx_render::SAMPLE_RATE {
            return Err("PSX effects render requires 44100 Hz output".into());
        }
        if history.as_deref() != Some("cold") {
            return Err("PSX effects render requires --history cold; other playback histories are unsupported".into());
        }
        let selection = match (auxiliary_entry, cue_row) {
            (Some(auxiliary_entry), Some(row)) if note_options.iter().all(Option::is_none) => psx_render::EffectsSelection::GameCue { auxiliary_entry, row },
            (None, None) => psx_render::EffectsSelection::SdkNote { note: Note {
                voice: voice.ok_or("PSX effects render: SDK notes require --voice")?,
                program: program.ok_or("PSX effects render: SDK notes require --program")?,
                tone: tone.ok_or("PSX effects render: SDK notes require --tone")?,
                key: key.ok_or("PSX effects render: SDK notes require --key")?,
                fine: fine.ok_or("PSX effects render: SDK notes require --fine")?,
                left: left.ok_or("PSX effects render: SDK notes require --left")?,
                right: right.ok_or("PSX effects render: SDK notes require --right")?,
            } },
            _ => return Err("PSX effects render: select either all seven SDK note options or --auxiliary-entry with --cue-row".into()),
        };
        let options = psx_render::EffectsOptions {
            header_entry: header_entry.ok_or(
                "PSX effects render requires --header-entry INDEX, qualified by --archive",
            )?,
            body_entry: body_entry
                .ok_or("PSX effects render requires --body-entry INDEX, qualified by --archive")?,
            layout,
            selection,
            history: psx_render::EffectsHistory::Cold,
            duration_frames: duration_frames
                .ok_or("PSX effects render requires --duration SECONDS")?,
            release_frames,
            safety_frames,
        };
        options.validate()?;
        let image = Image::read(&driver.ok_or("PSX effects render requires --driver DIRECTORY")?)?;
        let archive = ArchiveImage::from_bytes(std::fs::read(
            archive.ok_or("PSX effects render requires --archive")?,
        )?)?;
        let rendered = psx_render::render_effects(&image, &archive, &options)?;
        (rendered.wave, serde_json::to_value(rendered.report)?)
    } else if let (Some("pc"), Some(path)) = (engine.as_deref(), archive.as_ref()) {
        if midi.is_some()
            || font.is_some()
            || driver.is_some()
            || layout.is_some()
            || repeats.is_some()
        {
            return Err("PC archive render accepts --loops or --duration; MIDI/SoundFont/--repeats are a separate input mode, driver/layout are PSX-only".into());
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
            || driver.is_some()
            || sequence.is_some()
            || layout.is_some()
            || loops.is_some()
            || allow_approximations
        {
            return Err("PC render accepts MIDI/SoundFont inputs; archive, executable, driver, sequence, loop and layout options are invalid for PC MIDI rendering".into());
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
        if executable.is_some() {
            return Err("PSX render no longer accepts --executable; use --driver DIRECTORY".into());
        }
        let image = Image::read(&driver.ok_or("PSX render requires --driver DIRECTORY")?)?;
        let archive = ArchiveImage::from_bytes(std::fs::read(
            archive.ok_or("PSX render requires --archive")?,
        )?)?;
        let rendered = psx_render::render(&image, &archive, &options)?;
        (rendered.wave, serde_json::to_value(rendered.report)?)
    };
    let bytes = wave.to_bytes()?;
    let encoded = ogg.then(|| vorbis::encode(&wave, 0.7)).transpose()?;
    if let Some(encoded) = &encoded {
        report["delivery"] = serde_json::json!({
            "schema": "bof3.audio.delivery/v1",
            "format": "ogg",
            "encoder": "oxideav-vorbis 0.0.12",
            "quality": 0.7,
            "coupling": false,
            "lossy": true,
            "reference_wav_sha256": sha256_hex(&bytes),
            "encoded_sha256": sha256_hex(&encoded.bytes),
            "encoded_bytes": encoded.bytes.len(),
            "stream": encoded.stream,
            "decoded_error": "not measured for this output; representative independent-consumer quality accepted"
        });
    }
    let report_bytes = serde_json::to_vec_pretty(&report)?;
    let transaction = Publication::new(&output)?;
    let wave_path = transaction.staging.join("render.wav");
    publication::write_new(&wave_path, &bytes)?;
    publication::write_new(&transaction.staging.join("render.json"), &report_bytes)?;
    if Wave::from_bytes(&std::fs::read(wave_path)?)? != wave {
        return Err("render: written WAV did not verify; output not published".into());
    }
    if let Some(encoded) = &encoded {
        let path = transaction.staging.join("render.ogg");
        publication::write_new(&path, &encoded.bytes)?;
        let written = std::fs::read(path)?;
        if written != encoded.bytes || vorbis::inspect(&written)? != encoded.stream {
            return Err("render: written Vorbis did not verify; output not published".into());
        }
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
