//! Build the C89 player with the canonical repository compiler and original US runtime.
mod graphics;
mod sfx;
mod vocal;

use crate::{
    Result,
    machine::{bank, image::executable::Executable, image::profile::Profile},
    publication::{self, Publication},
    sequence::{SequenceSet, events::Kind, timeline},
};
use emi_ex_v2::image::ArchiveImage;
use std::{ffi::OsString, fs, path::PathBuf, process::Command};

pub const HELP: &str = "
  Development compiler (uses the original US runtime):
  compile --mode music --archive EMI --executable PSX_EXE --sequence INDEX
    --output NEW_DIR [--layout 0|1|2] [--repo-root REPOSITORY] [--disc-root ROOT] [--json]
  compile --mode sfx --archive EMI [--archive EMI ...] --executable PSX_EXE
    --output NEW_DIR [--layout 0|1|2] [--repo-root REPOSITORY] [--disc-root ROOT] [--json]
  compile --mode vocal --disc CUE --executable PSX_EXE
    --output NEW_DIR [--repo-root REPOSITORY] [--disc-root ROOT] [--json]
  SFX mode loads banks without a SEP. Left/Right switches banks, Up/Down selects
  physical samples, Cross plays a uniquely bound SDK tone, and Square halts.
  Missing or ambiguous tone and cue bindings remain unresolved.
  Vocal mode plays the qualified Japanese XA cue 0x2000 from the original US disc.
  Mount the qualified CUE during playback. Cross plays or retriggers, Circle
  stops, and Square toggles repeat. --disc-root supplies graphics assets.
  Build player.exe with canonical GCC 2.7.2. Output includes the rebuildable C89
  project and supplied runtime/song bytes. The US runtime is retained in full.
  Up/Down select tracks, Cross toggles play/pause, and Square restarts. Left/Right
  seek backward/forward five seconds with muted playback up to the destination.
  L1 toggles controls help. BIOS is supplied by the console or emulator.
";
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

pub fn run(mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let (mut mode, mut executable, mut output) = (None, None, None);
    let mut archive_paths = Vec::new();
    let (mut sequence, mut layout, mut root, mut disc) = (None, None, None, None);
    let mut cue_sheet = None;
    let mut json = false;
    while let Some(arg) = args.next() {
        let value = |args: &mut dyn Iterator<Item = OsString>| -> Result<OsString> {
            args.next()
                .ok_or_else(|| format!("{arg:?} needs a value").into())
        };
        match arg.to_str() {
            Some("--mode") if mode.is_none() => mode = Some(value(&mut args)?),
            Some("--archive") => archive_paths.push(PathBuf::from(value(&mut args)?)),
            Some("--executable") if executable.is_none() => {
                executable = Some(PathBuf::from(value(&mut args)?))
            }
            Some("--output") if output.is_none() => output = Some(PathBuf::from(value(&mut args)?)),
            Some("--sequence") if sequence.is_none() => {
                sequence = Some(
                    value(&mut args)?
                        .to_str()
                        .ok_or("invalid sequence")?
                        .parse::<usize>()?,
                )
            }
            Some("--layout") if layout.is_none() => {
                layout = Some(
                    value(&mut args)?
                        .to_str()
                        .ok_or("invalid layout")?
                        .parse::<u8>()?,
                )
            }
            Some("--disc-root") if disc.is_none() => disc = Some(PathBuf::from(value(&mut args)?)),
            Some("--disc") if cue_sheet.is_none() => {
                cue_sheet = Some(PathBuf::from(value(&mut args)?))
            }
            Some("--repo-root") if root.is_none() => root = Some(PathBuf::from(value(&mut args)?)),
            Some("--json") if !json => json = true,
            _ => return Err(format!("unknown or repeated compile option {arg:?}").into()),
        }
    }
    let mode = match mode.as_deref().and_then(|value| value.to_str()) {
        Some(mode @ ("music" | "sfx" | "vocal")) => mode,
        _ => return Err("compile requires --mode music, sfx or vocal".into()),
    };
    let (music, effects) = (mode == "music", mode == "sfx");
    if mode == "vocal" {
        if cue_sheet.is_none()
            || !archive_paths.is_empty()
            || sequence.is_some()
            || layout.is_some()
        {
            return Err(
                "compile vocal requires --disc CUE and rejects --archive, --sequence and --layout"
                    .into(),
            );
        }
    } else {
        if cue_sheet.is_some() {
            return Err("compile --disc is for vocal mode; use --disc-root for graphics".into());
        }
        if archive_paths.is_empty() || (music && archive_paths.len() != 1) {
            return Err("compile requires one music archive or one or more SFX archives".into());
        }
    }
    let sequence = if music {
        sequence.ok_or("compile music requires --sequence INDEX")?
    } else {
        if sequence.is_some() {
            return Err("compile SFX does not accept --sequence".into());
        }
        0
    };
    let output = output.ok_or("compile requires --output NEW_DIR")?;
    publication::require_absent(&output)?;
    let executable = executable.ok_or("compile requires --executable")?;
    let graphics = graphics::prepare(&disc.unwrap_or_else(|| {
        executable
            .parent()
            .unwrap_or(std::path::Path::new("."))
            .to_path_buf()
    }))?;
    let exe = Executable::from_bytes(fs::read(&executable)?)?;
    let profile = Profile::identify(&exe)?;
    let vocal = cue_sheet
        .as_deref()
        .map(|sheet| vocal::prepare(&exe, sheet))
        .transpose()?;
    let archives: Vec<_> = archive_paths
        .iter()
        .map(|path| Ok(ArchiveImage::from_bytes(fs::read(path)?)?))
        .collect::<Result<_>>()?;
    let names = archive_paths
        .iter()
        .map(|path| source_name(path))
        .collect::<Result<Vec<_>>>()?;
    let plans: Vec<_> = archives
        .iter()
        .map(|archive| {
            crate::machine::reference::plan_bank(
                &exe,
                archive,
                &bank::Options {
                    header_entry: unique(archive, &[6])?,
                    body_entry: unique(archive, &[7])?,
                    sequence_entry: if effects {
                        None
                    } else {
                        Some(unique(archive, &[9, 10])?)
                    },
                    layout,
                },
            )
        })
        .collect::<Result<_>>()?;
    let plan = plans.first();
    let sequence_bytes = if music { plan.unwrap().sequence } else { &[] };
    let sequences = if music {
        SequenceSet::parse(sequence_bytes)?.sequences
    } else {
        Vec::new()
    };
    if music && (sequence >= sequences.len() || sequences.len() > 4) {
        return Err(
            "compile sequence must select an existing index within four US SEP slots".into(),
        );
    }
    let mut timing = Vec::with_capacity(sequences.len());
    for selected in &sequences {
        if selected.resolution == 0
            || selected.resolution > 32767
            || selected.time_numerator == 0
            || selected.time_denominator_power > 7
        {
            return Err(
                "compile requires PPQN 1..32767 and a valid MIDI time signature for every track"
                    .into(),
            );
        }
        let bytes =
            &sequence_bytes[selected.data_offset..selected.data_offset + selected.data_bytes];
        let track = timeline::trace(bytes, &timeline::Limits::default())
            .map_err(|error| format!("compile track {}: {error}", selected.sequence_index + 1))?;
        let mut channels = 0u16;
        for step in &track.steps {
            if matches!(step.kind, Kind::Note { velocity, .. } if velocity != 0) {
                channels |= 1 << (step.status & 0xf);
            }
        }
        timing.push(format!(
            "{{{}, {}, {}, {channels:#06x}}}",
            selected.resolution, selected.time_numerator, selected.time_denominator_power
        ));
    }
    let root = fs::canonicalize(
        root.unwrap_or_else(|| PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../../..")),
    )?;
    let compiler = root.join("toolchains/gcc-2.7.2-psx/gcc");
    let version = Command::new(&compiler).arg("--version").output()?;
    if !version.status.success()
        || String::from_utf8_lossy(&version.stdout).lines().next() != Some("2.7.2")
    {
        return Err("compile requires the canonical GCC 2.7.2 PSX toolchain".into());
    }
    let transaction = Publication::new(&output)?;
    let path = &transaction.staging;
    for (name, bytes) in [
        (
            "player.c",
            include_bytes!("../../player/player.c").as_slice(),
        ),
        ("psx.h", include_bytes!("../../player/psx.h").as_slice()),
        ("audio.c", include_bytes!("../../player/audio.c").as_slice()),
        ("audio.h", include_bytes!("../../player/audio.h").as_slice()),
        ("bank.c", include_bytes!("../../player/bank.c").as_slice()),
        ("bank.h", include_bytes!("../../player/bank.h").as_slice()),
        ("sfx.c", include_bytes!("../../player/sfx.c").as_slice()),
        ("sfx.h", include_bytes!("../../player/sfx.h").as_slice()),
        ("vocal.c", include_bytes!("../../player/vocal.c").as_slice()),
        ("vocal.h", include_bytes!("../../player/vocal.h").as_slice()),
        (
            "controls.c",
            include_bytes!("../../player/controls.c").as_slice(),
        ),
        (
            "controls.h",
            include_bytes!("../../player/controls.h").as_slice(),
        ),
        (
            "graphics.c",
            include_bytes!("../../player/graphics.c").as_slice(),
        ),
        (
            "graphics.h",
            include_bytes!("../../player/graphics.h").as_slice(),
        ),
        ("hud.c", include_bytes!("../../player/hud.c").as_slice()),
        ("hud.h", include_bytes!("../../player/hud.h").as_slice()),
        ("start.s", include_bytes!("../../player/start.s").as_slice()),
        (
            "assets.s",
            include_bytes!("../../player/assets.s").as_slice(),
        ),
        (
            "player.ld",
            include_bytes!("../../player/player.ld").as_slice(),
        ),
        (
            "Makefile",
            include_bytes!("../../player/Makefile").as_slice(),
        ),
        (
            "README.md",
            include_bytes!("../../player/README.md").as_slice(),
        ),
        ("font.bin", graphics.font.as_slice()),
        ("palette.bin", graphics.palette.as_slice()),
        ("logo.bin", graphics.logo.as_slice()),
        ("runtime.bin", exe.text()),
    ] {
        publication::write_new(&path.join(name), bytes)?;
    }
    let banks = if effects {
        sfx::stage(path, &exe, &archives, &plans, &names)?
    } else {
        if let Some(plan) = plan {
            for (name, bytes) in [
                ("bank.vh", plan.header),
                ("bank.vb", plan.transfer),
                ("song.sep", plan.sequence),
            ] {
                publication::write_new(&path.join(name), bytes)?;
            }
            publication::write_new(&path.join("banks.s"), b".balign 64\n.globl player_header\nplayer_header: .incbin \"bank.vh\"\n.balign 64\n.globl player_body\nplayer_body: .incbin \"bank.vb\"\n.balign 4\n.globl player_sequence\nplayer_sequence: .incbin \"song.sep\"\n")?;
        } else {
            publication::write_new(&path.join("banks.s"), b"")?;
        }
        serde_json::Value::Null
    };
    publication::write_new(
        &path.join("runtime.s"),
        if music {
            b".incbin \"runtime.bin\", 0, 0xd421c\n.word __note_call\n.incbin \"runtime.bin\", 0xd4220\n".as_slice()
        } else {
            b".incbin \"runtime.bin\"\n".as_slice()
        },
    )?;
    let mut config = String::new();
    for (name, value) in [
        ("MODE_MUSIC", u32::from(music)),
        ("MODE_SFX", u32::from(effects)),
        ("MODE_VOCAL", u32::from(vocal.is_some())),
        ("BANK_COUNT", plans.len() as u32),
        ("SEQUENCE_COUNT", sequences.len() as u32),
        ("SEQUENCE_INDEX", sequence as u32),
    ] {
        config.push_str(&format!("#define PLAYER_{name} {value:#x}\n"));
    }
    if let Some(plan) = plan {
        let slot = &plan.identity.slot;
        for (name, value) in [
            ("LAYOUT", u32::from(plan.identity.layout)),
            ("BANK", u32::from(slot.vab_id)),
            ("SPU_BASE", slot.spu_base),
            ("HEADER_ADDRESS", slot.header_address),
            ("SEQUENCE_ADDRESS", slot.sequence_address),
            ("HEADER_BYTES", plan.header.len() as u32),
            ("BODY_BYTES", plan.identity.body_bytes as u32),
            ("SEQUENCE_BYTES", plan.sequence.len() as u32),
        ] {
            config.push_str(&format!("#define PLAYER_{name} {value:#x}\n"));
        }
    }
    if let Some(media) = &vocal {
        config.push_str(&format!(
            "#define PLAYER_VOCAL_CUE {id:#06x}\n#define PLAYER_VOCAL_ID \"{id:04X}\"\n#define PLAYER_VOCAL_FORMAT \"FILE {}  CHANNEL {}  CODING {:02X}\"\n",
            media.cue.filter_file, media.cue.channel, media.coding, id = media.cue.packed_id,
        ));
    }
    config.push_str(&format!(
        "#define PLAYER_TIMING {{{}}}\n",
        timing.join(", ")
    ));
    let source = vocal
        .as_ref()
        .map(|media| media.cue.disc_path)
        .or_else(|| names.first().map(String::as_str))
        .ok_or("compile requires a source")?;
    let literal = literal(source);
    config.push_str(&format!(
        "#define PLAYER_SONG {literal}\n#define PLAYER_VERSION \"{VERSION}\"\n"
    ));
    publication::write_new(&path.join("config.h"), config.as_bytes())?;
    let build = Command::new("make")
        .current_dir(path)
        .arg(format!("REPO={}", root.display()))
        .arg("all")
        .output()?;
    if !build.status.success() {
        return Err(format!(
            "player build failed:\n{}{}",
            String::from_utf8_lossy(&build.stdout),
            String::from_utf8_lossy(&build.stderr)
        )
        .into());
    }
    publication::write_new(
        &path.join("build.log"),
        &[build.stdout.as_slice(), build.stderr.as_slice()].concat(),
    )?;
    let result = Executable::from_bytes(fs::read(path.join("player.exe"))?)?;
    if result.header().entry_pc != 0x80010000
        || result.header().load_address != 0x80010000
        || result.header().stack_base != 0x80096000
    {
        return Err("compiled player has unexpected PS-X EXE load/entry/stack".into());
    }
    let report = serde_json::json!({
        "schema": "bof3.audio-player/v1", "version": VERSION, "author": "deitydragon",
        "song": source,
        "graphics": graphics.identity,
        "timecode": {"smpte": "30_ndf", "midi": music.then_some("bar:beat:tick"), "glyph_sizes": [[8,8],[6,6]]},
        "mode": mode, "vocal": vocal,
        "sequence": music.then_some(sequence), "bank": plan.map(|plan| &plan.identity), "banks": banks, "runtime_sha256": profile.exe_sha256,
        "tracks": sequences.len(),
        "controls": match mode {
            "sfx" => serde_json::json!({"left_right": "bank", "up_down": "physical_sample", "cross": "play_selected_sample", "square": "halt", "l1": "modal_help"}),
            "vocal" => serde_json::json!({"left_right": "none", "up_down": "single_qualified_cue", "cross": "play_retrigger", "circle": "stop", "square": "toggle_repeat", "l1": "modal_help"}),
            _ => serde_json::json!({"up_down": "track", "left_right": "seek_5_seconds", "cross": "play_pause", "square": "restart", "l1": "modal_help"}),
        },
        "executable_sha256": crate::digest::sha256_hex(result.bytes()),
        "compiler": "gcc-2.7.2-psx", "playback": match mode { "sfx" => "manual_game_cue_or_sdk_note", "vocal" => "original_xa", _ => "repeat_forever" },
        "runtime_patches": if music { vec!["midi_voice_meter_note_call"] } else { Vec::<&str>::new() },
        "runtime": "complete_original_us_image", "independent_fidelity_validated": false,
    });
    publication::write_new(
        &path.join("player.json"),
        serde_json::to_string_pretty(&report)?.as_bytes(),
    )?;
    transaction.publish()?;
    if json {
        println!("{}", serde_json::to_string_pretty(&report)?);
    } else {
        println!("{}", output.join("player.exe").display());
    }
    Ok(0)
}

fn unique(archive: &ArchiveImage, types: &[u16]) -> Result<usize> {
    let rows: Vec<_> = archive
        .entries()
        .iter()
        .enumerate()
        .filter(|(_, entry)| types.contains(&entry.file_type))
        .map(|(index, _)| index)
        .collect();
    match rows.as_slice() {
        [index] => Ok(*index),
        _ => Err(format!("compile needs one entry of types {types:?}, found {rows:?}").into()),
    }
}

fn source_name(path: &std::path::Path) -> Result<String> {
    let name = path
        .file_name()
        .and_then(|name| name.to_str())
        .ok_or("archive filename must be UTF-8")?;
    if name.is_empty()
        || name.len() > 36
        || !name
            .bytes()
            .all(|b| b.is_ascii_alphanumeric() || b" ._-".contains(&b))
    {
        return Err(
            "archive filename must contain 1..36 ASCII letters, digits, spaces, '.', '_' or '-'"
                .into(),
        );
    }
    Ok(name.to_owned())
}

fn literal(value: &str) -> String {
    let escaped: String = value.bytes().map(|byte| format!("\\{byte:03o}")).collect();
    format!("\"{escaped}\"")
}
