//! Bank, XA and explicitly approximate music extraction arguments.

use crate::{
    bank::extraction::{self, Options},
    Result,
};
use std::{ffi::OsString, path::PathBuf};

pub const HELP: &str =
    "\n extract --mode audio --kind bank (--disc-root DIR | --archive FILE...)\n\
   --executable PSX_EXE --output NEW_DIR [--id ID] [--reference-rate HZ] [--json]\n\
 Writes one WAV/XML folder per selected bank and a root audio.xml manifest.\n\
 Default reference rate: 44100 Hz (SPU pitch 4096); unity note 60 is an export convention.\n\
 Full original EMI bytes are preserved inside each bank.xml. Output must not exist.\n\
 XA: --kind xa_stream|xa_cue --xa-arithmetic split-floor|combined-rounded\n\
     --output NEW_DIR [--id ID] [--executable PSX_EXE]\n\
 XA cues require the executable; streams can use it to retain known cue placement.\n\
 XA uses its encoded rate and zero initial history per exported asset. Known truncated STRs fail.\n\
 Music: --mode music [--kind song] --allow-approximations --executable PSX_EXE\n\
   (--disc-root DIR | --archive FILE...) --output NEW_DIR [--id SONG_ID] [--loops N]\n\
 One SF2 and independent format-1 MIDI files per song; loops default to two traversals.\n\
 Music edits cannot yet be packed; full fidelity and --type classification remain unverified.\n";

pub fn run(mut args: impl Iterator<Item = OsString>) -> Result<u8> {
    let mut mode = None;
    let mut kind = None;
    let mut disc_root = None;
    let mut archives = Vec::new();
    let mut executable = None;
    let mut output = None;
    let mut id = None;
    let mut rate = None;
    let mut arithmetic = None;
    let mut json = false;
    let mut approximations = false;
    let mut loops = None;
    while let Some(arg) = args.next() {
        match arg.to_str() {
            Some("--mode") if mode.is_none() => mode = Some(string(&mut args, "--mode")?),
            Some("--kind") if kind.is_none() => kind = Some(string(&mut args, "--kind")?),
            Some("--disc-root") if disc_root.is_none() => {
                disc_root = Some(PathBuf::from(value(&mut args, "--disc-root")?))
            }
            Some("--archive") => archives.push(PathBuf::from(value(&mut args, "--archive")?)),
            Some("--executable") if executable.is_none() => {
                executable = Some(PathBuf::from(value(&mut args, "--executable")?))
            }
            Some("--output") if output.is_none() => {
                output = Some(PathBuf::from(value(&mut args, "--output")?))
            }
            Some("--id") if id.is_none() => id = Some(string(&mut args, "--id")?),
            Some("--reference-rate") if rate.is_none() => {
                rate = Some(string(&mut args, "--reference-rate")?.parse()?)
            }
            Some("--xa-arithmetic") if arithmetic.is_none() => {
                arithmetic = Some(match string(&mut args, "--xa-arithmetic")?.as_str() {
                    "split-floor" => crate::xa::Arithmetic::SplitFloor,
                    "combined-rounded" => crate::xa::Arithmetic::CombinedRounded,
                    _ => {
                        return Err(
                            "--xa-arithmetic requires split-floor or combined-rounded".into()
                        )
                    }
                });
            }
            Some("--json") if !json => json = true,
            Some("--allow-approximations") if !approximations => approximations = true,
            Some("--loops") if loops.is_none() => {
                loops = Some(string(&mut args, "--loops")?.parse()?)
            }
            Some("--type") => {
                return Err("--type sfx|vocals: classification is not yet verified".into())
            }
            _ => return Err(format!("unknown or repeated extract option {arg:?}").into()),
        }
    }
    if mode.as_deref() == Some("music") {
        if kind.as_deref().is_some_and(|k| k != "song") || rate.is_some() || arithmetic.is_some() {
            return Err(
                "music extract accepts --kind song; bank/XA rate/arithmetic flags do not apply"
                    .into(),
            );
        }
        let report = crate::music::extraction::songs(&crate::music::extraction::Options {
            disc_root,
            archives,
            executable: executable.ok_or("music extract requires --executable")?,
            output: output.ok_or("music extract requires --output")?,
            id,
            loops: loops.unwrap_or(2),
            allow_approximations: approximations,
        })?;
        if json {
            println!("{}", serde_json::to_string_pretty(&report)?);
        } else {
            println!(
                "{}: {} song folders, {} independently selectable MIDI sequences",
                report.output,
                report.songs.len(),
                report
                    .songs
                    .iter()
                    .map(|s| s.sequences.len())
                    .sum::<usize>()
            );
            for limitation in report.limitations {
                println!("Limitation: {limitation}");
            }
        }
        return Ok(0);
    }
    if mode.as_deref() != Some("audio") {
        return Err("extract requires --mode audio or music".into());
    }
    if approximations || loops.is_some() {
        return Err("--allow-approximations and --loops apply only to music extraction".into());
    }
    if matches!(kind.as_deref(), Some("xa_stream" | "xa_cue")) {
        if rate.is_some() {
            return Err("--reference-rate is for banks; XA is exported at its encoded rate".into());
        }
        let report = crate::xa::extraction::extract(&crate::xa::extraction::Options {
            disc_root,
            archives,
            executable,
            output: output.ok_or("extract requires --output")?,
            id,
            kind: if kind.as_deref() == Some("xa_cue") {
                crate::xa::extraction::Kind::Cue
            } else {
                crate::xa::extraction::Kind::Stream
            },
            arithmetic: arithmetic.ok_or(
                "XA extraction requires explicit --xa-arithmetic split-floor|combined-rounded",
            )?,
        })?;
        if json {
            println!("{}", serde_json::to_string_pretty(&report)?);
        } else {
            println!(
                "{}: {} sources, {} XA assets, {} selected sectors, {} PCM frames",
                report.output, report.sources, report.assets, report.sectors, report.pcm_frames
            );
            println!(
                "Arithmetic: {:?}; history: {}",
                report.arithmetic, report.initial_history
            );
            for source in report.unverified_sources {
                println!("Extent not verified: {source}");
            }
            for source in report.unselected_sources {
                println!("No selected assets: {source}");
            }
            for assumption in report.assumptions {
                println!("Note: {assumption}");
            }
        }
        return Ok(0);
    }
    if kind.as_deref() != Some("bank") || arithmetic.is_some() {
        return Err(
            "extract requires --kind bank|xa_stream|xa_cue; --xa-arithmetic applies only to XA"
                .into(),
        );
    }
    let report = extraction::banks(&Options {
        disc_root,
        archives,
        executable: executable.ok_or("extract requires --executable")?,
        output: output.ok_or("extract requires --output")?,
        id,
        reference_rate: rate.unwrap_or(44100),
    })?;
    if json {
        println!("{}", serde_json::to_string_pretty(&report)?);
    } else {
        println!(
            "{}: {} banks, {} WAV samples, {} PCM frames at reference {} Hz",
            report.output, report.banks, report.samples, report.pcm_frames, report.reference_rate
        );
        println!(
            "{} fixed PCM loops approximate continuing ADPCM predictor history",
            report.approximate_loops
        );
        for assumption in report.assumptions {
            println!("Note: {assumption}");
        }
    }
    Ok(0)
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

fn string(args: &mut impl Iterator<Item = OsString>, flag: &str) -> Result<String> {
    value(args, flag)?
        .into_string()
        .map_err(|_| format!("{flag} requires UTF-8").into())
}
