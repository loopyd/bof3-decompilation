use bof3_audio::{
    catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
    catalog::music::disc_evidence, catalog::music::SUPPORTED_CUES, machine::bus::Bus,
    machine::bus::Ram, machine::bus::Width, machine::cpu::Cpu, machine::executable::Executable,
};
use std::{
    collections::BTreeMap,
    fs::File,
    io::{Read, Seek, SeekFrom},
    path::PathBuf,
    process::Command,
};

fn executable() -> Executable {
    Executable::from_bytes(
        std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE")).unwrap(),
    )
    .unwrap()
}

fn corpus() -> PathBuf {
    PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").expect("set BOF3_AUDIO_CORPUS"))
}

#[test]
fn compiled_music_metadata_is_complete_and_contains_no_payloads() {
    let evidence = disc_evidence().unwrap();
    assert_eq!(evidence.files.len(), 81);
    assert_eq!(evidence.files[0].slot, 209);
    assert_eq!(evidence.files[0].path, "BIN/BGM/BGM000.EMI");
    assert_eq!(evidence.files[80].slot, 289);
}

#[test]
fn cue_cli_requires_a_single_selector_music_mode_and_executable_evidence() {
    for options in [
        vec!["--mode", "music", "--cue", "0"],
        vec![
            "--mode",
            "audio",
            "--cue",
            "0",
            "--executable",
            "missing.exe",
        ],
        vec![
            "--mode",
            "music",
            "--id",
            "0",
            "--cue",
            "0",
            "--executable",
            "missing.exe",
        ],
        vec![
            "--mode",
            "music",
            "--cue",
            "0",
            "--kind",
            "bank",
            "--executable",
            "missing.exe",
        ],
    ] {
        let output = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .arg("query")
            .args(options)
            .output()
            .unwrap();
        assert_eq!(output.status.code(), Some(2));
        assert!(output.stdout.is_empty());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK original 2352-byte data track"]
fn compiled_file_paths_and_lbas_match_independent_iso_directory_records() {
    // ECMA-119 directory extents, with the PSX Mode-2 sector header removed.
    // This verifier reads the disc directory, not the extraction XML or profile tables.
    fn read_extent(file: &mut File, lba: u32, size: usize) -> Vec<u8> {
        let mut result = vec![0; size.div_ceil(2048) * 2048];
        for (index, sector) in result.as_chunks_mut::<2048>().0.iter_mut().enumerate() {
            file.seek(SeekFrom::Start((u64::from(lba) + index as u64) * 2352 + 24))
                .unwrap();
            file.read_exact(sector).unwrap();
        }
        result.truncate(size);
        result
    }
    fn record_extent(record: &[u8]) -> (u32, usize) {
        let lba = u32::from_le_bytes(record[2..6].try_into().unwrap());
        let size = u32::from_le_bytes(record[10..14].try_into().unwrap());
        assert_eq!(lba, u32::from_be_bytes(record[6..10].try_into().unwrap()));
        assert_eq!(size, u32::from_be_bytes(record[14..18].try_into().unwrap()));
        (lba, size as usize)
    }
    fn directory(
        file: &mut File,
        lba: u32,
        size: usize,
        prefix: &str,
        files: &mut BTreeMap<String, (u32, usize)>,
    ) {
        let bytes = read_extent(file, lba, size);
        let mut pos = 0;
        while pos < bytes.len() {
            let length = usize::from(bytes[pos]);
            if length == 0 {
                pos = (pos / 2048 + 1) * 2048;
                continue;
            }
            let record = &bytes[pos..pos + length];
            pos += length;
            let name = &record[33..33 + usize::from(record[32])];
            if name == [0] || name == [1] {
                continue;
            }
            let name = std::str::from_utf8(name)
                .unwrap()
                .split(';')
                .next()
                .unwrap();
            let path = if prefix.is_empty() {
                name.to_owned()
            } else {
                format!("{prefix}/{name}")
            };
            let (extent, size) = record_extent(record);
            if record[25] & 2 != 0 {
                directory(file, extent, size, &path, files);
            } else {
                assert!(files.insert(path, (extent, size)).is_none());
            }
        }
    }
    let mut track =
        File::open(std::env::var_os("BOF3_AUDIO_TRACK").expect("set BOF3_AUDIO_TRACK")).unwrap();
    let pvd = read_extent(&mut track, 16, 2048);
    assert_eq!(&pvd[..7], b"\x01CD001\x01");
    let (lba, size) = record_extent(&pvd[156..190]);
    let mut files = BTreeMap::new();
    directory(&mut track, lba, size, "", &mut files);
    assert_eq!(files.len(), 887);
    for expected in disc_evidence().unwrap().files {
        assert_eq!(files[&expected.path], (expected.lba, expected.bytes));
        assert_eq!(
            bof3_audio::digest::sha256_hex(&read_extent(&mut track, expected.lba, expected.bytes)),
            expected.sha256
        );
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn original_cue_file_selection_and_lba_lookup_match_all_supported_media_records() {
    let executable = executable();
    let mut catalog = Catalog::read(Some(&corpus()), &[]).unwrap();
    let report = loader::resolve(&mut catalog, &executable).unwrap();
    assert_eq!(report.music.cues.len(), usize::from(SUPPORTED_CUES));
    for cue in &report.music.cues {
        assert_eq!(cue.resolution, "disc_path_and_whole_file_identity_match");
        assert_eq!(cue.sequence_assets.len(), 1);
        let mut ram = Ram::from_executable(&executable);
        ram.write(0x8014_5029, Width::Byte, 0xff).unwrap();
        let mut cpu = Cpu::new(0x8016_1bbc);
        cpu.set_register(4, 0x1200 | u32::from(cue.game_song_id));
        cpu.set_register(29, 0x801f_fff0);
        cpu.run_until(&mut ram, 0x8016_1fdc, 100).unwrap();
        assert_eq!(cpu.register(4), u32::from(cue.file_slot));
        let mut cpu = Cpu::new(0x8016_2160);
        cpu.set_register(4, u32::from(cue.file_slot));
        cpu.set_register(31, 0x8000_1000);
        cpu.run_until(&mut ram, 0x8000_1000, 30).unwrap();
        assert_eq!(cpu.register(2), cue.disc_lba);
        // A request already using the same cue's file bypasses the CD loader.
        ram.write(0x8014_5029, Width::Byte, u32::from(cue.game_song_id))
            .unwrap();
        let mut cpu = Cpu::new(0x8016_1bbc);
        cpu.set_register(4, u32::from(cue.game_song_id));
        cpu.set_register(29, 0x801f_fff0);
        cpu.set_register(31, 0x8000_1000);
        cpu.run_until(&mut ram, 0x8000_1000, 100).unwrap();
        assert_eq!(cpu.register(2), 0);
    }
    catalog.runtime_mapping = Some(report);
    for cue_id in 0..SUPPORTED_CUES {
        let asset = catalog.select_cue(cue_id).unwrap();
        assert_eq!(asset.game_song_ids, [cue_id]);
    }
    let song = catalog
        .assets
        .iter()
        .find(|a| a.id == "BIN/BGM/BGM000.EMI#entry=1")
        .unwrap();
    assert!(song.game_song_ids.len() > 1);
    assert!(catalog.select_cue(165).is_err());
    assert!(catalog.select_cue(255).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn cue_queries_accept_renamed_originals_reject_edits_and_require_ambiguous_sources_to_be_qualified()
{
    struct Temp(PathBuf);
    impl Drop for Temp {
        fn drop(&mut self) {
            std::fs::remove_dir_all(&self.0).unwrap();
        }
    }
    let dir = Temp(std::env::temp_dir().join(format!("bof3-cue-{}", std::process::id())));
    std::fs::create_dir(&dir.0).unwrap();
    let renamed = dir.0.join("renamed.EMI");
    let bytes = std::fs::read(corpus().join("BIN/BGM/BGM000.EMI")).unwrap();
    std::fs::write(&renamed, &bytes).unwrap();
    let mut catalog = Catalog::read(None, std::slice::from_ref(&renamed)).unwrap();
    let mapping = loader::resolve(&mut catalog, &executable()).unwrap();
    catalog.runtime_mapping = Some(mapping);
    let asset = catalog.select_cue(0).unwrap();
    assert!(
        matches!(&asset.data, AssetData::Sequence { metadata, .. } if metadata.sequence_index == 0)
    );
    let exe_path = std::env::var_os("BOF3_AUDIO_EXE").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["query", "--mode", "music", "--cue", "0", "--archive"])
        .arg(&renamed)
        .arg("--executable")
        .arg(&exe_path)
        .arg("--json")
        .output()
        .unwrap();
    assert!(output.status.success(), "{:?}", output.stderr);
    let json: serde_json::Value = serde_json::from_slice(&output.stdout).unwrap();
    assert_eq!(json["asset"]["game_song_ids"], serde_json::json!([0]));
    let duplicate = dir.0.join("duplicate.EMI");
    std::fs::write(&duplicate, &bytes).unwrap();
    let mut catalog = Catalog::read(None, &[renamed.clone(), duplicate]).unwrap();
    let mapping = loader::resolve(&mut catalog, &executable()).unwrap();
    catalog.runtime_mapping = Some(mapping);
    assert!(catalog
        .select_cue(0)
        .unwrap_err()
        .to_string()
        .contains("ambiguous"));
    let mut changed = bytes;
    *changed.last_mut().unwrap() ^= 1;
    std::fs::write(&renamed, changed).unwrap();
    let mut catalog = Catalog::read(None, &[renamed]).unwrap();
    let mapping = loader::resolve(&mut catalog, &executable()).unwrap();
    catalog.runtime_mapping = Some(mapping);
    assert!(catalog
        .select_cue(0)
        .unwrap_err()
        .to_string()
        .contains("original_source_not_supplied"));
    assert!(catalog.assets.iter().all(|a| a.game_song_ids.is_empty()));
}
