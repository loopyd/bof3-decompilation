//! CLI dispatch. Incomplete migration operations fail before creating output.

use crate::machine::{executable::Executable, profile::Profile};
use crate::{archive::MediaImage, verify::Report, Result};
use std::{ffi::OsString, path::PathBuf};

const HELP: &str = "bof3-audio — BOF3 audio migration\n\
usage: bof3-audio <index|query|map|extract|render|pack|verify> --mode audio|music [OPTIONS]\n\
\nAvailable during foundation development:\n\
  verify --mode audio|music --archive FILE [--against ORIGINAL] [--executable PSX_EXE] [--json]\n\
    Check EMI/XA container structure and optional whole-file byte equality.\n\
 Audio payloads, identities, translation, encoding, and rendering are not yet verified.\n\
 Known truncated XA snapshots fail a separate media-completeness check.\n\
\nPC and duration/loop-bounded US PSX music rendering are available; independent PSX fidelity remains unverified. Bank/XA/music packing includes bounded SF2 sample and tone gain/pan edits.\n";

pub fn run(arguments: impl Iterator<Item = OsString>) -> Result<u8> {
    let mut args = arguments.peekable();
    let command = args.next().ok_or("missing operation; use --help")?;
    if command == "--help" || command == "-h" {
        print!(
            "{HELP}{}{}{}{}",
            crate::catalog_cli::HELP,
            crate::extract_cli::HELP,
            crate::render_cli::HELP,
            crate::pack_cli::HELP
        );
        return Ok(0);
    }
    let operation = command.to_str().ok_or("operation must be UTF-8")?;
    if ![
        "index", "query", "map", "extract", "render", "pack", "verify",
    ]
    .contains(&operation)
    {
        return Err(format!("unknown operation {operation:?}").into());
    }
    if args.peek().is_some_and(|arg| arg == "--help") {
        print!(
            "{HELP}{}{}{}{}",
            crate::catalog_cli::HELP,
            crate::extract_cli::HELP,
            crate::render_cli::HELP,
            crate::pack_cli::HELP
        );
        return Ok(0);
    }
    if operation == "index" || operation == "query" || operation == "map" {
        return crate::catalog_cli::run(operation, args);
    }
    if operation == "extract" {
        return crate::extract_cli::run(args);
    }
    if operation == "render" {
        return crate::render_cli::run(args);
    }
    if operation == "pack" {
        return crate::pack_cli::run(args);
    }
    if operation != "verify" {
        return Err(
            format!("{operation}: not implemented; Rust migration acceptance is pending").into(),
        );
    }
    let mut mode = None;
    let mut archive = None;
    let mut against = None;
    let mut executable = None;
    let mut json = false;
    while let Some(arg) = args.next() {
        match arg.to_str() {
            Some("--mode") if mode.is_none() => {
                mode = Some(args.next().ok_or("--mode needs audio or music")?)
            }
            Some("--archive") if archive.is_none() => {
                archive = Some(PathBuf::from(args.next().ok_or("--archive needs a path")?))
            }
            Some("--against") if against.is_none() => {
                against = Some(PathBuf::from(args.next().ok_or("--against needs a path")?))
            }
            Some("--json") if !json => json = true,
            Some("--executable") if executable.is_none() => {
                executable = Some(PathBuf::from(
                    args.next().ok_or("--executable needs a path")?,
                ));
            }
            _ => return Err(format!("unknown or repeated option {arg:?}").into()),
        }
    }
    let mode = mode.ok_or("--mode audio|music is required")?;
    if mode != "audio" && mode != "music" {
        return Err("--mode must be audio or music".into());
    }
    let path = archive.ok_or("--archive is required")?;
    let image = MediaImage::read(&path)?;
    if mode == "music" && matches!(image, MediaImage::Xa(_)) {
        return Err("XA streams require --mode audio".into());
    }
    let original = against.as_ref().map(std::fs::read).transpose()?;
    let mut report = Report::inspect(path.display().to_string(), &image, original.as_deref());
    if let Some(executable) = executable {
        let image = Executable::from_bytes(std::fs::read(executable)?)?;
        report.runtime_profile = Some(Profile::identify(&image)?);
    }
    if json {
        println!("{}", serde_json::to_string_pretty(&report)?);
    } else {
        println!(
            "{}: {} container valid, {} bytes, {} entries/sectors",
            report.source, report.container, report.bytes, report.entries_or_sectors
        );
        if let Some(comparison) = &report.byte_equality {
            println!(
                "byte equality: {}; first difference: {:?}",
                comparison.equal, comparison.first_difference
            );
        }
        println!("SHA-256: {}", report.sha256);
        if let Some(completeness) = &report.media_completeness {
            println!(
                "Media completeness: {}; {}/{} sectors; {} missing audio sectors",
                completeness.status,
                completeness.actual_sectors,
                completeness.expected_sectors,
                completeness.missing_audio_sectors
            );
        } else {
            println!("Media completeness: not checked (unrecognized whole-file identity)");
        }
        if let Some(profile) = &report.runtime_profile {
            println!(
                "runtime profile: {}; bootstrap: {}",
                profile.id, profile.bootstrap
            );
        }
        println!(
            "Audio payloads, identities, translation, sample encoding and rendering: not checked"
        );
    }
    Ok(u8::from(
        report
            .byte_equality
            .is_some_and(|comparison| !comparison.equal)
            || report
                .media_completeness
                .as_ref()
                .is_some_and(|reference| reference.is_truncated()),
    ))
}
