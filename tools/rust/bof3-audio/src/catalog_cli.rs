//! Read-only catalog command surface.

use crate::{
    catalog::model::Asset, catalog::model::Catalog, machine::executable::Executable, Result,
};
use serde::Serialize;
use std::{ffi::OsString, path::PathBuf};

pub const HELP: &str =
    "\n index --mode audio|music (--disc-root DIR | --archive FILE...) [--kind KIND] [--json]\n\
 query --mode audio|music (--disc-root DIR | --archive FILE...) --id ID [--kind KIND] [--json]\n\
 map --mode audio|music (--disc-root DIR | --archive FILE...) --executable PSX_EXE [--json]\n\
 Optional --executable PSX_EXE adds verified US loader associations to index/query.\n\
 Music query accepts --cue NUMBER instead of --id, with --executable; supported US cues: 0..164.\n\
 KIND: bank, sample, song, sequence, xa_stream, xa_cue (requires --executable). Repeat --archive for multiple files.\n\
 Numeric IDs: bank/song entry number, sample ID, SEP sequence ID, XA channel, packed XA cue ID (decimal or 0x-prefixed hex).\n\
 Ambiguous numbers require a qualified ID from index; sequence qualified IDs use sequence index.\n\
 Disc-root identities use relative paths; explicit archives use canonical absolute paths.\n\
 Original music archives resolve by whole-file SHA-256. Other song identities, SFX/vocal classification,\n\
 active runtime layout, and XA scheduler endpoints remain unresolved.\n";

pub fn run(operation: &str, mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let mut mode = None;
    let mut root = None;
    let mut archives = Vec::new();
    let mut kind = None;
    let mut id = None;
    let mut cue = None;
    let mut executable = None;
    let mut json = false;
    while let Some(arg) = args.next() {
        match arg.to_str() {
            Some("--mode") if mode.is_none() => mode = Some(string_value(&mut args, "--mode")?),
            Some("--disc-root") if root.is_none() => root = Some(PathBuf::from(value(&mut args, "--disc-root")?)),
            Some("--archive") => archives.push(PathBuf::from(value(&mut args, "--archive")?)),
            Some("--kind") if operation != "map" && kind.is_none() => kind = Some(string_value(&mut args, "--kind")?),
            Some("--executable") if executable.is_none() => executable = Some(PathBuf::from(value(&mut args, "--executable")?)),
            Some("--id") if operation == "query" && id.is_none() => id = Some(string_value(&mut args, "--id")?),
            Some("--cue") if operation == "query" && cue.is_none() => cue = Some(string_value(&mut args, "--cue")?.parse::<u16>()?),
            Some("--json") if !json => json = true,
            Some("--type") => return Err("--type sfx|vocals: classification is not yet verified; use an unfiltered inventory with explicit unresolved content types".into()),
            _ => return Err(format!("unknown, repeated, or inapplicable option {arg:?}").into()),
        }
    }
    let mode = mode.ok_or("--mode required")?;
    if mode != "audio" && mode != "music" {
        return Err("--mode must be audio or music".into());
    }
    if kind.as_deref().is_some_and(|v| {
        !["bank", "sample", "song", "sequence", "xa_stream", "xa_cue"].contains(&v)
    }) {
        return Err(
            "unknown --kind; use bank, sample, song, sequence, xa_stream, or xa_cue".into(),
        );
    }
    if (mode == "audio" && matches!(kind.as_deref(), Some("song" | "sequence")))
        || (mode == "music" && matches!(kind.as_deref(), Some("xa_stream" | "xa_cue")))
    {
        return Err("--kind conflicts with --mode".into());
    }
    if operation == "query" && id.is_some() == cue.is_some() {
        return Err("query requires exactly one of --id or --cue".into());
    }
    if cue.is_some()
        && (mode != "music"
            || executable.is_none()
            || kind.as_deref().is_some_and(|k| k != "sequence"))
    {
        return Err(
            "--cue requires --mode music, --executable, and sequence kind (if specified)".into(),
        );
    }
    if operation == "map" && executable.is_none() {
        return Err("map requires --executable to identify a verified runtime profile".into());
    }
    if kind.as_deref() == Some("xa_cue") && executable.is_none() {
        return Err("--kind xa_cue requires --executable for verified cue extents".into());
    }
    let mut catalog = Catalog::read(root.as_deref(), &archives)?;
    if let Some(path) = executable {
        let exe = Executable::from_bytes(std::fs::read(path)?)?;
        let mapping = crate::catalog::loader::resolve(&mut catalog, &exe)?;
        catalog
            .unresolved
            .retain(|message| !message.starts_with("game bank/song IDs"));
        catalog.unresolved.push("runtime mapping covers loader associations, original music cues, and supported XA cue extents; edited/unrecognized media identities and other mappings remain unresolved");
        catalog.runtime_mapping = Some(mapping);
    }
    if operation == "map" {
        let mapping = catalog
            .runtime_mapping
            .as_ref()
            .expect("required executable was parsed");
        if json {
            println!("{}", serde_json::to_string_pretty(mapping)?);
        } else {
            println!("{}: {}", mapping.schema, mapping.profile.id);
            for reference in &mapping.references {
                println!(
                    "{}@0x{:08X} (EXE +0x{:X}): {}",
                    reference.target,
                    reference.address,
                    reference.executable_file_offset,
                    reference.role
                );
            }
            for association in &mapping.associations {
                println!(
                    "{}: load slot {}, game bank {}, bodies [{}], songs [{}]",
                    association.bank,
                    association.load_slot,
                    association.game_bank_id,
                    association.body_entries.join(", "),
                    association.songs.join(", ")
                );
            }
            for group in &mapping.shared_banks {
                println!("Identical bank payloads: {} (VH {} bytes, VB {} bytes; game identities remain separate)",
                    group.banks.join(", "), group.header_bytes, group.body_bytes);
            }
            for cue in &mapping.music.cues {
                println!(
                    "{}: {} -> bank {}, sequence {} [{}] {}",
                    cue.id,
                    cue.disc_path,
                    cue.game_bank_id,
                    cue.sequence_index,
                    cue.sequence_assets.join(", "),
                    cue.resolution
                );
            }
            for record in &mapping.xa.cues {
                for binding in &record.bindings {
                    println!(
                        "XA {:#06x}: {} file {}, channel {}, sectors {}..={} stride {}: {} missing sectors; {}",
                        record.cue.packed_id, binding.source, record.cue.filter_file,
                        record.cue.channel, record.cue.sector_start, record.cue.last_sector,
                        record.cue.sector_stride, binding.missing_selected_sectors,
                        binding.asset.as_deref().unwrap_or("unavailable")
                    );
                }
            }
            for unresolved in &mapping.unresolved {
                println!("Unresolved: {unresolved}");
            }
        }
        return Ok(0);
    }
    if operation == "query" {
        let asset = if let Some(cue) = cue {
            catalog.select_cue(cue)?
        } else {
            catalog.select(
                &mode,
                kind.as_deref(),
                id.as_deref().expect("query selector validated"),
            )?
        };
        if json {
            #[derive(Serialize)]
            struct Query<'a> {
                schema: &'static str,
                asset: &'a Asset,
                unresolved: &'a [&'static str],
                runtime_mapping: Option<&'a crate::catalog::model::Report>,
            }
            println!(
                "{}",
                serde_json::to_string_pretty(&Query {
                    schema: "bof3.audio-query/v1",
                    asset,
                    unresolved: &catalog.unresolved,
                    runtime_mapping: catalog.runtime_mapping.as_ref(),
                })?
            );
        } else {
            print_asset(asset);
            println!("{}", serde_json::to_string_pretty(&asset.data)?);
            print_unresolved(&catalog);
        }
    } else {
        catalog.assets.retain(|a| {
            a.matches_mode(&mode) && kind.as_deref().is_none_or(|kind| a.kind() == kind)
        });
        if json {
            println!("{}", serde_json::to_string_pretty(&catalog)?);
        } else {
            println!(
                "{}: {} sources, {} assets",
                catalog.schema,
                catalog.sources.len(),
                catalog.assets.len()
            );
            for asset in &catalog.assets {
                print_asset(asset);
            }
            print_unresolved(&catalog);
        }
    }
    Ok(0)
}

fn print_asset(asset: &Asset) {
    println!(
        "{}\t{}\tcontent={}\tgame_bank={:?}\tgame_songs={:?}",
        asset.id,
        asset.kind(),
        asset.content_type,
        asset.game_bank_id,
        asset.game_song_ids
    );
}

fn print_unresolved(catalog: &Catalog) {
    for message in &catalog.unresolved {
        println!("Unresolved: {message}");
    }
}

fn value(args: &mut impl Iterator<Item = OsString>, flag: &str) -> Result<OsString> {
    let value = args
        .next()
        .ok_or_else(|| format!("{flag} requires a value"))?;
    if value.to_str().is_some_and(|v| v.starts_with("--")) {
        return Err(format!("{flag} requires a value before {value:?}").into());
    }
    Ok(value)
}

fn string_value(args: &mut impl Iterator<Item = OsString>, flag: &str) -> Result<String> {
    value(args, flag)?
        .into_string()
        .map_err(|_| format!("{flag} must be UTF-8").into())
}
