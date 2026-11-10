//! Qualify the original raw disc and the confirmed Japanese vocal cue.

use crate::{
    archive::disc::{DiscImage, RAW_SECTOR_BYTES},
    catalog::{model::XaCue, music},
    digest::{sha256_file, sha256_hex},
    machine::{image::executable::Executable, reference},
    xa, Result,
};
use serde::Serialize;
use std::{
    fs::File,
    io::Read,
    path::{Component, Path, PathBuf},
};

const JAPANESE_VOCAL: u16 = 0x2000;
const CDDA_SHA256: &str = "ce5509fad13f6210656c9d29fb536b47abe5f824467177652c91b5c500470c77";

#[derive(Serialize)]
pub(super) struct Media {
    cue_sheet: PathBuf,
    cue_sheet_sha256: String,
    tracks: [Track; 2],
    lead_out_lba: u64,
    source_lba: u32,
    source_sectors: usize,
    source_sha256: String,
    pub cue: XaCue,
    pub coding: u8,
}

#[derive(Serialize)]
struct Track {
    number: u8,
    file: PathBuf,
    mode: &'static str,
    sha256: String,
    index_01: u64,
    pregap_lba: u64,
    start_lba: u64,
    end_lba: u64,
}

pub(super) fn prepare(exe: &Executable, sheet: &Path) -> Result<Media> {
    let mut bytes = Vec::new();
    File::open(sheet)?.take(8193).read_to_end(&mut bytes)?;
    if bytes.len() > 8192 {
        return Err("vocal compile: CUE sheet exceeds 8 KiB".into());
    }
    let lines: Vec<_> = std::str::from_utf8(&bytes)?
        .lines()
        .map(str::trim)
        .filter(|line| !line.is_empty())
        .collect();
    let [data, "TRACK 01 MODE2/2352", "INDEX 01 00:00:00", cdda, "TRACK 02 AUDIO", "INDEX 00 00:00:00", "INDEX 01 00:02:00"] =
        lines.as_slice()
    else {
        return Err("vocal compile: expected the original two-file US raw CUE layout".into());
    };
    let root = sheet.parent().unwrap_or(Path::new("."));
    let data = root.join(track_name(data)?);
    let cdda = root.join(track_name(cdda)?);
    let (data_sha256, data_sectors) =
        qualify_track(&data, 442_938_048, &music::disc_evidence()?.track_sha256)?;
    let (cdda_sha256, cdda_sectors) = qualify_track(&cdda, 37_396_800, CDDA_SHA256)?;
    let cue = reference::mapping::read_cues(exe)?
        .into_iter()
        .find(|cue| cue.packed_id == JAPANESE_VOCAL)
        .ok_or("vocal compile: original selector lacks the qualified Japanese cue")?;
    let mut disc = DiscImage::open(&data)?;
    let entry = disc
        .files()
        .get(cue.disc_path)
        .ok_or("vocal compile: missing VOICE.STR")?;
    let source_lba = entry.lba;
    let source_sectors = entry.sector_count();
    if u64::from(source_lba) + cue.sector_start as u64 != u64::from(cue.runtime_start_lba) {
        return Err("vocal compile: mounted source LBA disagrees with original selector".into());
    }
    let source = disc.read_xa(cue.disc_path)?;
    let source_sha256 = sha256_hex(source.bytes());
    xa::reference::identify(&source_sha256)
        .filter(|record| !record.is_truncated() && record.reference_source == cue.disc_path)
        .ok_or("vocal compile: expected the complete original VOICE.STR extent")?;
    let coding = source
        .sectors()
        .get(cue.sector_start)
        .ok_or("vocal compile: cue starts outside the source")?
        .coding;
    for index in 0..cue.sector_count {
        let sector = source
            .sectors()
            .get(cue.sector_start + index * cue.sector_stride)
            .ok_or("vocal compile: cue extends outside the source")?;
        if [sector.file, sector.channel, sector.coding] != [cue.filter_file, cue.channel, coding]
            || sector.submode & 0x64 != 0x64
        {
            return Err("vocal compile: selected sectors disagree with the original cue".into());
        }
    }
    let lead_out_lba = data_sectors + cdda_sectors;
    Ok(Media {
        cue_sheet: sheet.into(),
        cue_sheet_sha256: sha256_hex(&bytes),
        tracks: [
            Track {
                number: 1,
                file: data,
                mode: "MODE2/2352",
                sha256: data_sha256,
                index_01: 0,
                pregap_lba: 0,
                start_lba: 0,
                end_lba: data_sectors,
            },
            Track {
                number: 2,
                file: cdda,
                mode: "AUDIO",
                sha256: cdda_sha256,
                index_01: 150,
                pregap_lba: data_sectors,
                start_lba: data_sectors + 150,
                end_lba: lead_out_lba,
            },
        ],
        lead_out_lba,
        source_lba,
        source_sectors,
        source_sha256,
        cue,
        coding,
    })
}

fn track_name(line: &str) -> Result<&str> {
    let name = line
        .strip_prefix("FILE \"")
        .and_then(|rest| rest.strip_suffix("\" BINARY"))
        .ok_or("vocal compile: expected a quoted BINARY track file")?;
    let mut parts = Path::new(name).components();
    if !matches!(parts.next(), Some(Component::Normal(part)) if part == name)
        || parts.next().is_some()
        || name.contains(['"', '\\', '\0'])
    {
        return Err("vocal compile: track file must be a name beside the CUE sheet".into());
    }
    Ok(name)
}

fn qualify_track(path: &Path, bytes: u64, expected: &str) -> Result<(String, u64)> {
    let file = File::open(path)?;
    let metadata = file.metadata()?;
    if !metadata.is_file() || metadata.len() != bytes {
        return Err(format!("vocal compile: wrong raw track size for {}", path.display()).into());
    }
    let hash = sha256_file(file, bytes)?;
    if hash != expected {
        return Err(format!(
            "vocal compile: wrong US track SHA-256 for {}",
            path.display()
        )
        .into());
    }
    Ok((hash, bytes / RAW_SECTOR_BYTES))
}
