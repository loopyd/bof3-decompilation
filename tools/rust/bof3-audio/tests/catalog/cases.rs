use bof3_audio::{
    bank::Bank, catalog::model::AssetData, catalog::model::Catalog, sequence::SequenceSet,
};
use std::{
    path::{Path, PathBuf},
    process::Command,
    sync::atomic::{AtomicU64, Ordering},
};

struct Fixture(PathBuf);

impl Fixture {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-audio-catalog-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        std::fs::create_dir(&path).unwrap();
        Self(path)
    }

    fn write(&self, name: &str, bytes: &[u8]) -> PathBuf {
        let path = self.0.join(name);
        std::fs::create_dir_all(path.parent().unwrap()).unwrap();
        std::fs::write(&path, bytes).unwrap();
        path
    }
}

impl Drop for Fixture {
    fn drop(&mut self) {
        std::fs::remove_dir_all(&self.0).unwrap();
    }
}

fn bank() -> Vec<u8> {
    let mut bytes = vec![0; 0x820 + 2 * 512 + 512];
    bytes[..4].copy_from_slice(b"pBAV");
    bytes[4..8].copy_from_slice(&7u32.to_le_bytes());
    for (at, value) in [(0x12, 2u16), (0x14, 3), (0x16, 2)] {
        bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
    }
    bytes[0x20 + 3 * 16] = 2;
    bytes[0x20 + 88 * 16] = 1;
    for (block, program, count) in [(0, 3u16, 2), (1, 88, 1)] {
        for index in 0..count {
            let at = 0x820 + block * 512 + index * 32;
            bytes[at + 2] = 100;
            bytes[at + 4] = 60;
            bytes[at + 5] = 200;
            bytes[at + 7] = 127;
            bytes[at + 20..at + 22].copy_from_slice(&program.to_le_bytes());
            bytes[at + 22..at + 24].copy_from_slice(&(index as u16 + 1).to_le_bytes());
        }
    }
    let table = 0x820 + 2 * 512;
    // Index zero is the transferred prefix before the first sample, in 8-byte units.
    bytes[table..table + 2].copy_from_slice(&321u16.to_le_bytes());
    bytes[table + 2..table + 4].copy_from_slice(&2u16.to_le_bytes());
    bytes[table + 4..table + 6].copy_from_slice(&4u16.to_le_bytes());
    bytes
}

fn sequences() -> Vec<u8> {
    let mut bytes = b"pQES\0\0".to_vec();
    for id in [12u16, 12, 19, 20] {
        bytes.extend(id.to_be_bytes());
        bytes.extend(48u16.to_be_bytes());
        bytes.extend([0x07, 0xa1, 0x20, 4, 2]);
        bytes.extend(3u32.to_be_bytes());
        bytes.extend([0, 0xff, 0x2f]);
    }
    bytes.extend([0; 8]);
    bytes
}

fn archive() -> Vec<u8> {
    let entries = [
        (6u16, bank()),
        (0, vec![0x91; 17]),
        (10, sequences()),
        (7, vec![0; 321 * 8 + 48]),
    ];
    let mut bytes = vec![0; 2048];
    bytes[..4].copy_from_slice(&(entries.len() as u32).to_le_bytes());
    bytes[8..16].copy_from_slice(b"MATH_TBL");
    for (index, (kind, payload)) in entries.iter().enumerate() {
        let at = 16 + index * 16;
        bytes[at..at + 4].copy_from_slice(&(payload.len() as u32).to_le_bytes());
        bytes[at + 12..at + 14].copy_from_slice(&kind.to_le_bytes());
        bytes.extend(payload);
        bytes.resize(bytes.len().div_ceil(2048) * 2048, 0xa5);
    }
    bytes
}

#[test]
fn sparse_programs_layered_tones_and_sample_table_keep_separate_numbers() {
    let bank = Bank::parse(&bank()).unwrap();
    assert_eq!(bank.header_id, 0);
    assert_eq!(
        bank.programs.iter().map(|p| p.program).collect::<Vec<_>>(),
        [3, 88]
    );
    assert_eq!(bank.programs[1].tone_block, 1);
    assert_eq!(bank.programs[0].tones.len(), 2);
    assert_eq!(bank.programs[0].tones[0].shift, 200);
    assert_eq!(bank.samples[0].sample_id, 1);
    assert_eq!(bank.body_prefix_bytes, 321 * 8);
    assert_eq!(bank.samples[0].body_offset, 321 * 8);
    assert_eq!(bank.samples[1].body_offset, 321 * 8 + 16);
    assert_eq!(bank.samples[1].encoded_bytes, 32);
    assert!(bank.diagnostics.is_empty());
}

#[test]
fn malformed_banks_fail_while_unresolved_tone_references_remain_visible() {
    let mut bytes = bank();
    bytes[0x820 + 22..0x820 + 24].fill(0);
    let parsed = Bank::parse(&bytes).unwrap();
    assert_eq!(parsed.programs[0].tones.len(), 2);
    assert_eq!(parsed.programs[0].tones[0].sample_reference, 0);
    assert!(parsed.diagnostics[0].contains("sample reference 0"));
    for length in [0, 31, 0x81f, bytes.len() - 1] {
        assert!(Bank::parse(&bytes[..length]).is_err());
    }
    bytes[0x20 + 3 * 16] = 17;
    assert!(Bank::parse(&bytes).is_err());
}

#[test]
fn sep_sequences_stay_independent_even_with_duplicate_ids() {
    let bytes = sequences();
    let parsed = SequenceSet::parse(&bytes).unwrap();
    assert_eq!(parsed.sequences.len(), 4);
    assert_eq!(
        parsed.sequences[0].sequence_id,
        parsed.sequences[1].sequence_id
    );
    assert_ne!(
        parsed.sequences[0].sequence_index,
        parsed.sequences[1].sequence_index
    );
    assert_eq!(parsed.sequences[0].tempo_us, 500_000);
    assert_eq!(parsed.sequences[0].resolution, 48);
    assert_eq!(parsed.trailing_bytes, 8);
    for sequence in &parsed.sequences {
        assert_eq!(
            &bytes[sequence.data_offset..sequence.data_offset + sequence.data_bytes],
            &[0, 0xff, 0x2f]
        );
    }
    assert_eq!(parsed.event_validation, "not_checked");
    for length in [0, 6, 15, bytes.len() - 9] {
        assert!(SequenceSet::parse(&bytes[..length]).is_err());
    }
    let mut overflow = bytes.clone();
    overflow[15..19].fill(0xff);
    assert!(SequenceSet::parse(&overflow).is_err());
}

#[test]
fn catalogs_qualify_duplicate_basenames_and_reject_ambiguous_numeric_ids() {
    let fixture = Fixture::new();
    fixture.write("one/a.EMI", &archive());
    fixture.write("two/a.EMI", &archive());
    let catalog = Catalog::read(Some(&fixture.0), &[]).unwrap();
    assert_eq!(catalog.sources.len(), 2);
    assert_eq!(catalog.sources[0].entries.len(), 4);
    assert_eq!(catalog.sources[0].entries[1].file_type, 0);
    assert_eq!(catalog.assets.len(), 16);
    assert!(catalog
        .select("audio", Some("bank"), "0")
        .unwrap_err()
        .to_string()
        .contains("ambiguous"));
    assert!(catalog
        .select("music", Some("sequence"), "12")
        .unwrap_err()
        .to_string()
        .contains("ambiguous"));
    let bank = catalog
        .select("audio", Some("bank"), "one/a.EMI#entry=0")
        .unwrap();
    assert_eq!(bank.game_bank_id, None);
    assert!(
        matches!(&bank.data, AssetData::Bank { body_candidates, .. } if body_candidates == &["one/a.EMI#entry=3"])
    );
    assert_eq!(
        catalog
            .select("music", Some("sequence"), "two/a.EMI#entry=2/sequence=1")
            .unwrap()
            .source,
        "two/a.EMI"
    );
}

#[test]
fn duplicate_inputs_and_conflicting_sources_are_errors() {
    let fixture = Fixture::new();
    let path = fixture.write("a.EMI", &archive());
    assert!(Catalog::read(Some(&fixture.0), std::slice::from_ref(&path)).is_err());
    assert!(Catalog::read(None, &[]).is_err());
    assert!(Catalog::read(None, &[path.clone(), path])
        .unwrap_err()
        .to_string()
        .contains("duplicate"));
}

#[test]
fn xa_preserves_stream_file_channel_and_discontinuous_audio_ranges() {
    let fixture = Fixture::new();
    let headers = [
        [2, 3, 0x64, 1],
        [2, 9, 0x62, 1],
        [7, 3, 0x64, 0],
        [2, 3, 0xe4, 1],
    ];
    let mut bytes = vec![0; headers.len() * 2336];
    for (index, header) in headers.iter().enumerate() {
        bytes[index * 2336..index * 2336 + 4].copy_from_slice(header);
        bytes[index * 2336 + 4..index * 2336 + 8].copy_from_slice(header);
    }
    fixture.write("voice.STR", &bytes);
    let catalog = Catalog::read(Some(&fixture.0), &[]).unwrap();
    assert_eq!(catalog.assets.len(), 2);
    assert!(catalog.select("audio", Some("xa_stream"), "3").is_err());
    let asset = catalog
        .select(
            "audio",
            None,
            "voice.STR#stream=2/channel=3/coding=01/sectors=0..4",
        )
        .unwrap();
    let AssetData::XaStream(stream) = &asset.data else {
        panic!()
    };
    assert_eq!(stream.channels, Some(2));
    assert_eq!(stream.sample_rate, Some(37800));
    assert_eq!(
        stream
            .sector_ranges
            .iter()
            .map(|r| (r.start, r.end_exclusive))
            .collect::<Vec<_>>(),
        [(0, 1), (3, 4)]
    );
    assert_eq!(stream.eof_sectors, [3]);
    assert_eq!(stream.cue_boundaries, "unresolved");
    assert_eq!(asset.game_bank_id, None);
}

fn cli(root: &Path, args: &[&str]) -> std::process::Output {
    Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(args)
        .arg("--disc-root")
        .arg(root)
        .output()
        .unwrap()
}

#[test]
fn cli_disc_root_archive_list_json_human_and_query_have_consistent_identities() {
    let fixture = Fixture::new();
    let a = fixture.write("a%.EMI", &archive());
    let b = fixture.write("b#.EMI", &archive());
    let result = cli(
        &fixture.0,
        &["index", "--mode", "music", "--kind", "sequence", "--json"],
    );
    assert!(result.status.success(), "{:?}", result.stderr);
    let json: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(json["schema"], "bof3.audio-catalog/v1");
    assert_eq!(json["assets"].as_array().unwrap().len(), 8);
    let id = json["assets"][0]["id"].as_str().unwrap();
    assert!(id.contains("%25"));
    let query = cli(
        &fixture.0,
        &["query", "--mode", "music", "--id", id, "--json"],
    );
    assert!(query.status.success());
    let selected: serde_json::Value = serde_json::from_slice(&query.stdout).unwrap();
    assert_eq!(selected["asset"], json["assets"][0]);
    let human = cli(
        &fixture.0,
        &["index", "--mode", "music", "--kind", "sequence"],
    );
    assert!(human.status.success());
    assert!(String::from_utf8(human.stdout).unwrap().contains(id));
    let list = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["index", "--mode", "audio", "--kind", "bank", "--archive"])
        .arg(a)
        .arg("--archive")
        .arg(b)
        .arg("--json")
        .output()
        .unwrap();
    assert!(list.status.success());
    let listed: serde_json::Value = serde_json::from_slice(&list.stdout).unwrap();
    assert_eq!(listed["assets"].as_array().unwrap().len(), 2);
    for args in [
        vec!["query", "--mode", "music", "--id", "12"],
        vec!["index", "--mode", "music", "--kind", "xa_stream"],
        vec!["index", "--mode", "audio", "--type", "vocals"],
        vec!["query", "--mode", "audio"],
        vec!["index", "--mode", "music", "--json", "--json"],
    ] {
        let result = cli(&fixture.0, &args);
        assert_eq!(result.status.code(), Some(2), "{args:?}");
        assert!(result.stdout.is_empty());
    }
}

#[test]
#[ignore = "requires user-supplied BOF3_AUDIO_CORPUS"]
fn local_corpus_inventory_retains_all_banks_and_independent_sequences() {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").expect("set BOF3_AUDIO_CORPUS"));
    let catalog = Catalog::read(Some(&root), &[]).unwrap();
    assert_eq!(catalog.sources.len(), 884);
    let count = |kind| catalog.assets.iter().filter(|a| a.kind() == kind).count();
    assert_eq!(count("bank"), 1020);
    assert_eq!(count("song"), 119);
    assert_eq!(count("sequence"), 476);
    assert!(count("xa_stream") >= 30);
    let mut identities = std::collections::BTreeSet::new();
    for asset in &catalog.assets {
        assert!(identities.insert(&asset.id));
    }
    assert!(catalog
        .assets
        .iter()
        .all(|a| a.game_bank_id.is_none() && a.game_song_ids.is_empty()));
}
