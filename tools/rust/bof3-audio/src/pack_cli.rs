//! Command-line bank, XA and music packing from preserved extraction manifests.
use crate::{document::manifest, pack, Result};
use std::{ffi::OsString, path::PathBuf};

pub const HELP: &str = "
Audio packing:
  pack --mode audio --input FOLDER [--executable PSX_EXE] --output NEW_DIR [--json]
  Bank folders/roots and XA roots preserve unchanged EMI/STR bytes.
  WAV edits must fit allocations; XA cues require compatible history boundaries.
  Executable required for banks and recorded XA runtime metadata.
Music packing:
  pack --mode music --input FOLDER --executable PSX_EXE --output NEW_DIR [--json]
  Song folders/music roots support fixed-layout MIDI, SF2 PCM/loops and invertible tone gain/pan.
  Topology, tuning, envelope, timing/channel and inconsistent shared edits fail explicitly.
";

pub fn run(mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let (mut mode, mut input, mut executable, mut output) = (None, None, None, None);
    let mut json = false;
    while let Some(arg) = args.next() {
        match arg.to_str() {
            Some("--mode") if mode.is_none() => {
                mode = Some(args.next().ok_or("--mode needs audio or music")?)
            }
            Some("--input") if input.is_none() => {
                input = Some(PathBuf::from(args.next().ok_or("--input needs a folder")?))
            }
            Some("--executable") if executable.is_none() => {
                executable = Some(PathBuf::from(
                    args.next().ok_or("--executable needs a path")?,
                ))
            }
            Some("--output") if output.is_none() => {
                output = Some(PathBuf::from(
                    args.next().ok_or("--output needs a new directory")?,
                ))
            }
            Some("--json") if !json => json = true,
            _ => return Err(format!("pack: unknown or repeated option {arg:?}").into()),
        }
    }
    let mode = mode.ok_or("--mode audio or music is required")?;
    if mode != "audio" && mode != "music" {
        return Err("pack: unsupported mode; choose audio or music".into());
    }
    let input = input.ok_or("--input is required")?;
    let output = output.ok_or("--output is required")?;
    if mode == "music" {
        let report = crate::music::packing::songs(&pack::Options {
            input,
            output,
            executable: executable
                .ok_or("--executable is required for music metadata validation")?,
        })?;
        if json {
            println!("{}", serde_json::to_string_pretty(&report)?);
        } else {
            println!(
                "Published {} EMI archives from {} songs to {}",
                report.archives.len(),
                report.songs.len(),
                report.output
            );
            for archive in &report.archives {
                println!(
                    "{} -> {}: byte equality {}; {} changed entries",
                    archive.source,
                    archive.path,
                    archive.byte_equal,
                    archive.changed_entries.len()
                );
            }
            println!(
                "Reconstruction and verification details: {}/pack.json",
                report.output
            );
            for limitation in report.limitations {
                println!("Limitation: {limitation}");
            }
        }
        return Ok(0);
    }
    if input.join("audio.xml").exists() && input.join("bank.xml").exists() {
        return Err("pack: folder contains both audio.xml and bank.xml; input is ambiguous".into());
    }
    if input.join("audio.xml").exists() {
        let root = manifest::read(&manifest::relative_file(&input, &input, "audio.xml")?)?;
        if matches!(root.attribute("kind")?, "xa_stream" | "xa_cue") {
            let report = crate::xa::packing::assets(&crate::xa::packing::Options {
                input,
                executable,
                output,
            })?;
            if json {
                println!("{}", serde_json::to_string_pretty(&report)?);
            } else {
                println!(
                    "Published {} STR sources from {} assets to {}",
                    report.sources.len(),
                    report.assets.len(),
                    report.output
                );
                for source in &report.sources {
                    println!(
                        "{} -> {}: byte equality {}; {} changed sectors",
                        source.source,
                        source.path,
                        source.byte_equal,
                        source.changed_sectors.len()
                    );
                }
                println!(
                    "Encoding loss and verification details: {}/pack.json",
                    report.output
                );
                for limitation in report.limitations {
                    println!("Limitation: {limitation}");
                }
            }
            return Ok(0);
        }
    }
    let report = pack::banks(&pack::Options {
        input,
        executable: executable.ok_or("--executable is required for runtime metadata validation")?,
        output,
    })?;
    if json {
        println!("{}", serde_json::to_string_pretty(&report)?);
    } else {
        let changed = report
            .banks
            .iter()
            .flat_map(|b| &b.samples)
            .filter(|s| s.changed)
            .count();
        println!(
            "Published {} EMI archives from {} banks to {}; {changed} samples encoded",
            report.archives.len(),
            report.banks.len(),
            report.output
        );
        for archive in &report.archives {
            println!(
                "{} -> {}: byte equality {}",
                archive.source, archive.path, archive.byte_equal
            );
        }
        println!(
            "Encoding loss and verification details: {}/pack.json",
            report.output
        );
        for limitation in report.limitations {
            println!("Limitation: {limitation}");
        }
    }
    Ok(0)
}
