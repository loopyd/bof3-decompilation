use bof3_audio::{
    archive::disc::DiscImage, catalog::model::AssetData, catalog::model::Catalog,
    machine::cd_position::CdPosition, machine::executable::Executable, xa::cue,
};
use std::{
    fs::{self, File},
    io::{Read, Seek, SeekFrom},
    path::PathBuf,
    process::Command,
    sync::atomic::{AtomicU64, Ordering},
};

struct Directory(PathBuf);

impl Directory {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-xa-cues-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        fs::create_dir(&path).unwrap();
        Self(path)
    }
}

impl Drop for Directory {
    fn drop(&mut self) {
        fs::remove_dir_all(&self.0).unwrap();
    }
}

fn exe() -> Executable {
    Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap()).unwrap()
}

#[test]
fn xa_cue_cli_requires_audio_mode_and_executable() {
    for args in [
        vec!["index", "--mode", "audio", "--kind", "xa_cue"],
        vec![
            "index",
            "--mode",
            "music",
            "--kind",
            "xa_cue",
            "--executable",
            "missing",
        ],
    ] {
        let output = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(args)
            .output()
            .unwrap();
        assert_eq!(output.status.code(), Some(2));
        assert!(output.stdout.is_empty());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK original media"]
fn complete_disc_cues_cover_only_matching_raw_audio_sectors() {
    let track = PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").unwrap());
    let mut disc = DiscImage::open(&track).unwrap();
    let directory = Directory::new();
    for path in [
        "BIN/SCE_XA/S_XA00.STR",
        "BIN/BMAG_XA/MAGIC00.STR",
        "BIN/SCE_XA/VOICE.STR",
    ] {
        let destination = directory.0.join(path);
        fs::create_dir_all(destination.parent().unwrap()).unwrap();
        fs::write(destination, disc.read_xa(path).unwrap().bytes()).unwrap();
    }
    let mut catalog = Catalog::read(Some(&directory.0), &[]).unwrap();
    let original_streams = catalog.assets.len();
    let report = cue::resolve(&mut catalog, &exe()).unwrap();
    assert_eq!(report.cues.len(), 896);
    assert_eq!(catalog.assets.len(), original_streams + 896);
    let mut raw = File::open(track).unwrap();
    let mut sector_counts = [0usize; 3];
    for record in report.cues {
        assert_eq!(record.bindings.len(), 1);
        let binding = &record.bindings[0];
        assert_eq!(binding.missing_selected_sectors, 0);
        assert_eq!(binding.source_completeness, "known_complete_disc_extent");
        let asset = catalog
            .select("audio", Some("xa_cue"), binding.asset.as_deref().unwrap())
            .unwrap();
        assert!(matches!(asset.data, AssetData::XaCue(_)));
        assert!(asset.game_bank_id.is_none());
        assert!(asset.game_song_ids.is_empty());
        assert!(asset.entry.is_none());
        for index in 0..record.cue.sector_count {
            // Read subheaders directly from the original 2352-byte track, independently
            // of the catalog's extracted-stream sector grouping and presence checks.
            let lba =
                u64::from(record.cue.runtime_start_lba) + (index * record.cue.sector_stride) as u64;
            raw.seek(SeekFrom::Start(lba * 2352 + 12)).unwrap();
            let mut header = [0; 4];
            raw.read_exact(&mut header).unwrap();
            assert_eq!(
                &header[..3],
                &CdPosition::from_lba(lba as i32).unwrap().bcd()
            );
            assert_eq!(header[3], 2);
            let mut subheaders = [0; 8];
            raw.read_exact(&mut subheaders).unwrap();
            assert_eq!(&subheaders[..4], &subheaders[4..]);
            assert_eq!(subheaders[0], record.cue.filter_file);
            assert_eq!(subheaders[1], record.cue.channel);
            assert_ne!(
                subheaders[2] & 4,
                0,
                "cue {:#06x}, LBA {lba}",
                record.cue.packed_id
            );
            sector_counts[usize::from(record.cue.stream)] += 1;
        }
    }
    assert_eq!(sector_counts, [10962, 14856, 297]);
    cue::resolve(&mut catalog, &exe()).unwrap();
    assert_eq!(
        catalog.assets.len(),
        original_streams + 896,
        "resolving twice must not duplicate identities"
    );

    let voice = directory.0.join("BIN/SCE_XA/VOICE.STR");
    let output = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args([
            "query",
            "--mode",
            "audio",
            "--kind",
            "xa_cue",
            "--id",
            "0x2004",
            "--archive",
        ])
        .arg(&voice)
        .arg("--executable")
        .arg(std::env::var_os("BOF3_AUDIO_EXE").unwrap())
        .arg("--json")
        .output()
        .unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    let json: serde_json::Value = serde_json::from_slice(&output.stdout).unwrap();
    assert_eq!(json["asset"]["metadata"]["packed_id"], 0x2004);
    assert_eq!(json["asset"]["metadata"]["channel"], 4);

    let duplicate = directory.0.join("renamed.STR");
    fs::copy(&voice, &duplicate).unwrap();
    let mut duplicated = Catalog::read(None, &[voice.clone(), duplicate.clone()]).unwrap();
    cue::resolve(&mut duplicated, &exe()).unwrap();
    assert!(duplicated
        .select("audio", Some("xa_cue"), "0x2004")
        .unwrap_err()
        .to_string()
        .contains("ambiguous"));
    let first_id = duplicated
        .assets
        .iter()
        .find(|a| a.kind() == "xa_cue")
        .unwrap()
        .id
        .clone();
    duplicated
        .select("audio", Some("xa_cue"), &first_id)
        .unwrap();

    let mut edited = fs::read(&voice).unwrap();
    edited[32] ^= 1;
    fs::write(&duplicate, edited).unwrap();
    let mut unknown = Catalog::read(None, &[duplicate]).unwrap();
    let unknown_map = cue::resolve(&mut unknown, &exe()).unwrap();
    assert!(unknown_map.cues.iter().all(|c| c.bindings.is_empty()));
    assert!(unknown.assets.iter().all(|a| a.kind() != "xa_cue"));
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS containing known truncated streams"]
fn truncated_sources_report_missing_sectors_without_inventing_complete_cues() {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let sources: Vec<_> = [
        "BIN/SCE_XA/S_XA00.STR",
        "BIN/BMAG_XA/MAGIC00.STR",
        "BIN/SCE_XA/VOICE.STR",
    ]
    .iter()
    .map(|path| root.join(path))
    .collect();
    let mut catalog = Catalog::read(None, &sources).unwrap();
    let report = cue::resolve(&mut catalog, &exe()).unwrap();
    let mut missing_by_stream = [0usize; 3];
    for record in report.cues {
        assert_eq!(record.bindings.len(), 1);
        let binding = &record.bindings[0];
        assert_eq!(binding.source_completeness, "known_truncated_disc_extent");
        assert_eq!(
            binding.asset.is_some(),
            binding.missing_selected_sectors == 0
        );
        missing_by_stream[usize::from(record.cue.stream)] += binding.missing_selected_sectors;
    }
    assert!(missing_by_stream[0] > 0);
    assert_eq!(missing_by_stream[1], 1347);
    assert_eq!(missing_by_stream[2], 0);
    assert_eq!(
        catalog
            .assets
            .iter()
            .filter(|a| matches!(&a.data, AssetData::XaCue(c) if c.stream == 2))
            .count(),
        5
    );
}
