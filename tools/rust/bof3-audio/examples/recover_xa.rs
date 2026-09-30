//! Reconstruct full US reference streams into a new directory, preserving inputs.
use bof3_audio::{archive::disc::DiscImage, digest::sha256_hex, Result};
use std::{fs::OpenOptions, io::Write, path::PathBuf};

fn main() -> Result<()> {
    let mut args = std::env::args_os().skip(1);
    let track = PathBuf::from(
        args.next()
            .ok_or("usage: recover_xa RAW_TRACK NEW_OUTPUT_DIRECTORY")?,
    );
    let output = PathBuf::from(args.next().ok_or("new output directory required")?);
    if args.next().is_some() {
        return Err("unexpected argument".into());
    }
    if output.exists() {
        return Err("output directory already exists; refusing to replace it".into());
    }
    let mut disc = DiscImage::open(&track)?;
    let executable =
        bof3_audio::machine::executable::Executable::from_bytes(disc.read_file("SLUS_004.22")?)?;
    bof3_audio::machine::profile::Profile::identify(&executable)?;
    let mut streams = Vec::new();
    let mut report = Vec::new();
    for path in [
        "BIN/BMAG_XA/MAGIC00.STR",
        "BIN/SCE_XA/S_XA00.STR",
        "BIN/SCE_XA/VOICE.STR",
        "LOGO/CAPCOM30.STR",
    ] {
        let entry = disc
            .files()
            .get(path)
            .ok_or("expected US stream is absent")?
            .clone();
        let image = disc.read_xa(path)?;
        report.push(serde_json::json!({"path":path,"lba":entry.lba,"iso_logical_bytes":entry.logical_bytes,
            "sectors":image.sectors().len(),"bytes":image.bytes().len(),"sha256":sha256_hex(image.bytes())}));
        streams.push((path, image));
    }
    // All extents and sector headers pass before any output is created.
    std::fs::create_dir(&output)?;
    for (relative, image) in streams {
        let path = output.join(relative);
        std::fs::create_dir_all(path.parent().unwrap())?;
        let mut file = OpenOptions::new().write(true).create_new(true).open(path)?;
        file.write_all(image.bytes())?;
        file.sync_all()?;
    }
    let mut manifest = OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(output.join("streams.json"))?;
    let report = serde_json::json!({"schema":"bof3.audio-disc-stream-recovery/v1","track":track,"streams":report});
    serde_json::to_writer_pretty(&mut manifest, &report)?;
    manifest.write_all(b"\n")?;
    manifest.sync_all()?;
    println!(
        "Recovered four complete XA/STR extents into {}",
        output.display()
    );
    Ok(())
}
