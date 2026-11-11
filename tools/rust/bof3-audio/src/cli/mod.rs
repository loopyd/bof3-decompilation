//! CLI dispatch for owned-driver operations and the development player.

use crate::archive::MediaImage;
use crate::machine::driver::Image;
use crate::verification::Report;
use crate::Result;
use std::{ffi::OsString, path::PathBuf};

const HELP: &str = "bof3-audio — BOF3 audio preservation and playback\n\
usage: bof3-audio <index|query|map|extract|render|pack|verify|doctor|compile> [OPTIONS]\n\
\nOwned driver setup and verification:\n\
  doctor --driver DIRECTORY [--json]\n\
  verify --mode audio|music --archive FILE [--against ORIGINAL] [--driver DIRECTORY] [--json]\n\
    Report structure, identity, completeness, whole-file, translation, encoding and rendering evidence separately.\n\
    Missing evidence stays explicit; known truncated XA snapshots fail completeness.\n\
\nPC and bounded US PSX music, SFX and XA rendering are available.\n\
Bank/XA/music packing preserves unchanged files and supports the documented sample, instrument and sequence edits.\n\
Native equivalence covers declared cold and combined playback contexts; hardware fidelity and arbitrary incoming states remain unverified.\n";

pub fn run(arguments: impl Iterator<Item = OsString>) -> Result<u8> {
    let mut args = arguments.peekable();
    let command = args.next().ok_or("missing operation; use --help")?;
    if command == "--help" || command == "-h" {
        print!(
            "{HELP}{}{}{}{}{}",
            crate::cli::catalog::HELP,
            crate::cli::extraction::HELP,
            crate::cli::rendering::HELP,
            crate::cli::packing::HELP,
            crate::player::HELP
        );
        return Ok(0);
    }
    let operation = command.to_str().ok_or("operation must be UTF-8")?;
    if ![
        "index", "query", "map", "extract", "render", "pack", "compile", "verify", "doctor",
    ]
    .contains(&operation)
    {
        return Err(format!("unknown operation {operation:?}").into());
    }
    if args.peek().is_some_and(|arg| arg == "--help") {
        print!(
            "{HELP}{}{}{}{}{}",
            crate::cli::catalog::HELP,
            crate::cli::extraction::HELP,
            crate::cli::rendering::HELP,
            crate::cli::packing::HELP,
            crate::player::HELP
        );
        return Ok(0);
    }
    if operation == "doctor" {
        return diagnose(args);
    }
    if operation == "index" || operation == "query" || operation == "map" {
        return crate::cli::catalog::run(operation, args);
    }
    if operation == "extract" {
        return crate::cli::extraction::run(args);
    }
    if operation == "render" {
        return crate::cli::rendering::run(args);
    }
    if operation == "compile" {
        return crate::player::run(args);
    }
    if operation == "pack" {
        return crate::cli::packing::run(args);
    }
    let mut mode = None;
    let mut archive = None;
    let mut against = None;
    let mut driver = None;
    let mut json = false;
    while let Some(arg) = args.next() {
        match arg.to_str() {
            Some("--executable" | "--bios") => {
                return Err(
                    "verify no longer accepts --executable or --bios; use --driver DIRECTORY"
                        .into(),
                )
            }
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
            Some("--driver") if driver.is_none() => {
                driver = Some(PathBuf::from(args.next().ok_or("--driver needs a path")?));
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
    if let Some(driver) = driver {
        let image = Image::read(&driver)?;
        report.source_profile = Some(crate::catalog::mapping::Mapping::read(&image)?.profile);
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
        for (name, check) in report.checks.categories() {
            println!("{name}: {}. {}", check.status, check.reason);
        }
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
        if let Some(profile) = &report.source_profile {
            println!(
                "owned driver source profile: {}; mapping scope: {}",
                profile.id, profile.bootstrap
            );
        }
    }
    Ok(u8::from(report.checks.has_failures()))
}

fn diagnose(mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let mut directory = None;
    let mut json = false;
    while let Some(arg) = args.next() {
        match arg.to_str() {
            Some("--driver") if directory.is_none() => {
                directory = Some(PathBuf::from(
                    args.next().ok_or("--driver needs a directory")?,
                ))
            }
            Some("--json") if !json => json = true,
            _ => {
                return Err(format!(
                    "unknown, repeated or inapplicable audio doctor option {arg:?}"
                )
                .into())
            }
        }
    }
    let image = Image::read(&directory.ok_or("audio doctor requires --driver DIRECTORY")?)?;
    let mapping = crate::catalog::mapping::Mapping::read(&image)?;
    crate::voice::tuning::Reference::from_driver(&image, None)?;
    let report = serde_json::json!({
        "schema": "bof3.audio-doctor/v1", "driver": image.evidence(),
        "source_profile": mapping.profile,
        "scope": "owned artifact, catalog mapping and isolated pitch readiness; playback fidelity is not established",
    });
    if json {
        println!("{}", serde_json::to_string_pretty(&report)?);
    } else {
        println!("owned audio driver: mapping and pitch inputs verified");
    }
    Ok(0)
}

mod catalog;
mod extraction;
mod packing;
mod rendering;
