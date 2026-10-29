//! Build the C89 player with the canonical repository compiler and original US runtime.
mod graphics;

use crate::{
    machine::{bank, executable::Executable, profile::Profile},
    publication::{self, Publication},
    sequence::SequenceSet,
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{ffi::OsString, fs, path::PathBuf, process::Command};

pub const HELP: &str = "
  compile --mode music --archive EMI --executable PSX_EXE --sequence INDEX
    --output NEW_DIR [--layout 0|1|2] [--repo-root REPOSITORY] [--disc-root ROOT] [--json]
  Build player.exe with canonical GCC 2.7.2. Output includes the rebuildable C89
  project and supplied runtime/song bytes. The US runtime is retained in full.
  The player repeats forever. BIOS is supplied by the console or emulator.
";
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

pub fn run(mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let (mut mode, mut archive, mut executable, mut output) = (None, None, None, None);
    let (mut sequence, mut layout, mut root, mut disc) = (None, None, None, None);
    let mut json = false;
    while let Some(arg) = args.next() {
        let value = |args: &mut dyn Iterator<Item = OsString>| -> Result<OsString> {
            args.next()
                .ok_or_else(|| format!("{arg:?} needs a value").into())
        };
        match arg.to_str() {
            Some("--mode") if mode.is_none() => mode = Some(value(&mut args)?),
            Some("--archive") if archive.is_none() => {
                archive = Some(PathBuf::from(value(&mut args)?))
            }
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
            Some("--repo-root") if root.is_none() => root = Some(PathBuf::from(value(&mut args)?)),
            Some("--json") if !json => json = true,
            _ => return Err(format!("unknown or repeated compile option {arg:?}").into()),
        }
    }
    if mode.as_deref() != Some(std::ffi::OsStr::new("music")) {
        return Err("compile requires --mode music".into());
    }
    let sequence = sequence.ok_or("compile requires --sequence INDEX")?;
    let archive_path = archive.ok_or("compile requires --archive EMI")?;
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
    let archive = ArchiveImage::from_bytes(fs::read(&archive_path)?)?;
    let unique = |types: &[u16]| -> Result<usize> {
        let rows: Vec<_> = archive
            .entries()
            .iter()
            .enumerate()
            .filter(|(_, e)| types.contains(&e.file_type))
            .map(|(i, _)| i)
            .collect();
        match rows.as_slice() {
            [index] => Ok(*index),
            _ => Err(format!("compile needs one entry of types {types:?}, found {rows:?}").into()),
        }
    };
    let plan = crate::machine::reference::plan_bank(
        &exe,
        &archive,
        &bank::Options {
            header_entry: unique(&[6])?,
            body_entry: unique(&[7])?,
            sequence_entry: Some(unique(&[9, 10])?),
            layout,
        },
    )?;
    let sequences = SequenceSet::parse(plan.sequence)?;
    if sequence >= sequences.sequences.len() || sequence >= 4 || sequences.sequences.len() > 4 {
        return Err(
            "compile sequence must select an existing index within four US SEP slots".into(),
        );
    }
    let selected = &sequences.sequences[sequence];
    if selected.resolution == 0
        || selected.resolution > 32767
        || selected.time_numerator == 0
        || selected.time_denominator_power > 7
    {
        return Err("compile requires PPQN 1..32767 and a valid MIDI time signature".into());
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
        ("bank.vh", plan.header),
        ("bank.vb", plan.transfer),
        ("song.sep", plan.sequence),
    ] {
        publication::write_new(&path.join(name), bytes)?;
    }
    let slot = &plan.identity.slot;
    let mut config = String::new();
    for (name, value) in [
        ("LAYOUT", u32::from(plan.identity.layout)),
        ("BANK", u32::from(slot.vab_id)),
        ("SPU_BASE", slot.spu_base),
        ("HEADER_ADDRESS", slot.header_address),
        ("SEQUENCE_ADDRESS", slot.sequence_address),
        ("HEADER_BYTES", plan.header.len() as u32),
        ("BODY_BYTES", plan.identity.body_bytes as u32),
        ("SEQUENCE_BYTES", plan.sequence.len() as u32),
        ("SEQUENCE_COUNT", sequences.sequences.len() as u32),
        ("SEQUENCE_INDEX", sequence as u32),
        ("RESOLUTION", u32::from(selected.resolution)),
        ("NUMERATOR", u32::from(selected.time_numerator)),
        (
            "DENOMINATOR_POWER",
            u32::from(selected.time_denominator_power),
        ),
    ] {
        config.push_str(&format!("#define PLAYER_{name} {value:#x}\n"));
    }
    let song = archive_path
        .file_name()
        .and_then(|s| s.to_str())
        .ok_or("archive filename must be UTF-8")?;
    if song.is_empty()
        || song.len() > 36
        || !song
            .bytes()
            .all(|b| b.is_ascii_alphanumeric() || b" ._-".contains(&b))
    {
        return Err(
            "archive filename must contain 1..36 ASCII letters, digits, spaces, '.', '_' or '-'"
                .into(),
        );
    }
    // Octal byte escapes keep filenames literal in generated C, including quotes and backslashes.
    let literal: String = song.bytes().map(|b| format!("\\{b:03o}")).collect();
    config.push_str(&format!(
        "#define PLAYER_SONG \"{literal}\"\n#define PLAYER_VERSION \"{VERSION}\"\n"
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
        "song": archive_path.file_name().ok_or("archive needs a filename")?.to_string_lossy(),
        "graphics": graphics.identity,
        "timecode": {"smpte": "30_ndf", "midi": "bar:beat:tick", "glyph_sizes": [[8,8],[6,6]]},
        "sequence": sequence, "bank": plan.identity, "runtime_sha256": profile.exe_sha256,
        "executable_sha256": crate::digest::sha256_hex(result.bytes()),
        "compiler": "gcc-2.7.2-psx", "playback": "repeat_forever",
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
