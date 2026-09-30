use crate::fixture as music_fixture;
use bof3_audio::{
    digest::sha256_hex, document::manifest, interchange::midi::Message, interchange::midi::Midi,
    music::extraction::Options, pc_render,
};
use music_fixture::synthetic_archive;
use std::{
    fs,
    io::Cursor,
    path::{Path, PathBuf},
    process::Command,
    sync::atomic::{AtomicU64, Ordering},
};
struct Directory(PathBuf);
impl Directory {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let p = std::env::temp_dir().join(format!(
            "bof3-music-extract-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        fs::create_dir(&p).unwrap();
        Self(p)
    }
}
impl Drop for Directory {
    fn drop(&mut self) {
        fs::remove_dir_all(&self.0).unwrap();
    }
}
fn options(root: &Path, archive: PathBuf) -> Options {
    Options {
        disc_root: None,
        archives: vec![archive],
        executable: std::env::var_os("BOF3_AUDIO_EXE")
            .map(PathBuf::from)
            .unwrap_or_else(|| "missing-executable".into()),
        output: root.join("export"),
        id: None,
        loops: 2,
        allow_approximations: true,
    }
}

#[test]
fn approximation_consent_and_loop_limits_fail_before_reading_or_publishing() {
    let d = Directory::new();
    let mut opts = options(&d.0, d.0.join("absent.EMI"));
    opts.allow_approximations = false;
    assert!(bof3_audio::music::extraction::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("--allow-approximations"));
    assert!(!opts.output.exists());
    opts.allow_approximations = true;
    opts.loops = 0;
    assert!(bof3_audio::music::extraction::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("positive"));
    assert!(!opts.output.exists());
}
#[test]
fn cli_rejects_music_options_in_audio_mode_and_wrong_media_flags() {
    for args in [
        vec!["extract", "--mode", "audio", "--allow-approximations"],
        vec!["extract", "--mode", "music", "--reference-rate", "44100"],
        vec!["extract", "--mode", "music", "--kind", "xa_cue"],
    ] {
        let output = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(args)
            .output()
            .unwrap();
        assert!(!output.status.success());
    }
}

fn check_export(root: &Path, expected: &[u8]) -> usize {
    let index = manifest::read(&root.join("music.xml")).unwrap();
    let song_link = index.child("song").unwrap();
    let path = manifest::relative_file(root, root, song_link.attribute("path").unwrap()).unwrap();
    let node = manifest::read(&path).unwrap();
    let base = path.parent().unwrap();
    assert_eq!(
        manifest::preservation(
            node.child("preservation").unwrap(),
            Some("entire_source_archive")
        )
        .unwrap(),
        expected
    );
    let bank = node.child("bank").unwrap();
    assert_eq!(
        bank.attribute("id").unwrap(),
        song_link.attribute("bank").unwrap()
    );
    let sf2 = bank.child("soundfont").unwrap();
    let font_bytes =
        fs::read(manifest::relative_file(root, base, sf2.attribute("path").unwrap()).unwrap())
            .unwrap();
    assert_eq!(sha256_hex(&font_bytes), sf2.attribute("sha256").unwrap());
    rustysynth::SoundFont::new(&mut Cursor::new(&font_bytes)).unwrap();
    let binding: serde_json::Value =
        serde_json::from_str(&bank.child("binding").unwrap().text).unwrap();
    assert_eq!(
        binding["identity"]["game_bank_id"].as_u64().unwrap(),
        bank.number::<u64>("game_bank_id").unwrap()
    );
    let sequences = &node.child("sequences").unwrap().children;
    for sequence in sequences {
        let bytes = fs::read(
            manifest::relative_file(root, base, sequence.attribute("path").unwrap()).unwrap(),
        )
        .unwrap();
        assert_eq!(sha256_hex(&bytes), sequence.attribute("sha256").unwrap());
        let midi = Midi::from_bytes(&bytes).unwrap();
        assert_eq!((midi.format, midi.tracks.len()), (1, 2));
        rustysynth::MidiFile::new(&mut Cursor::new(&bytes)).unwrap();
        let report: serde_json::Value =
            serde_json::from_str(&sequence.child("translation").unwrap().text).unwrap();
        let timeline: serde_json::Value =
            serde_json::from_str(&sequence.child("timeline").unwrap().text).unwrap();
        for mapping in report["mappings"].as_array().unwrap() {
            let step = &timeline["steps"][mapping["step"].as_u64().unwrap() as usize];
            let event = &midi.tracks[mapping["track"].as_u64().unwrap() as usize].events
                [mapping["event"].as_u64().unwrap() as usize];
            assert_eq!(event.tick, step["tick"].as_u64().unwrap());
            assert_eq!(mapping["source_cursor"], step["source_cursor"]);
        }
        let rendered = pc_render::render(
            &bytes,
            &font_bytes,
            &pc_render::Options {
                sample_rate: 44100,
                repeats: 1,
                duration_frames: Some(pc_render::schedule(&midi, 44100).unwrap().frames.min(4410)),
                release_frames: 4410,
                safety_frames: 44100,
            },
        )
        .unwrap();
        assert!(rendered.report.output_frames > 4410);
        assert_eq!(
            rendered.wave.pcm.len(),
            rendered.report.output_frames as usize * 2
        );
    }
    sequences.len()
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; independent SEP files, layering/mappings and RustySynth consumption"]
fn separate_sequences_and_preserved_archive_survive_music_extraction() {
    let d = Directory::new();
    let source = d.0.join("song & 'é'.EMI");
    let original = synthetic_archive(false);
    fs::write(&source, &original).unwrap();
    let opts = options(&d.0, source);
    let report = bof3_audio::music::extraction::songs(&opts).unwrap();
    assert_eq!(report.songs.len(), 1);
    assert_eq!(report.songs[0].sequences.len(), 2);
    assert_eq!(check_export(&opts.output, &original), 2);
    assert_eq!(
        report.songs[0]
            .sequences
            .iter()
            .map(|s| s.sequence_id)
            .collect::<Vec<_>>(),
        [17, 91]
    );
    assert_eq!(report.songs[0].sequences[1].translation.final_tick, 24);
    assert_eq!(
        report.songs[0].sequences[0]
            .translation
            .ignored_bend_low_bytes,
        1
    );
    let folder = opts
        .output
        .join(&report.songs[0].path)
        .parent()
        .unwrap()
        .to_owned();
    let first = Midi::from_bytes(&fs::read(folder.join("sequence-000.mid")).unwrap()).unwrap();
    let font =
        rustysynth::SoundFont::new(&mut Cursor::new(fs::read(folder.join("bank.sf2")).unwrap()))
            .unwrap();
    assert_eq!(font.get_instruments().len(), 4); // Three tones plus explicit empty programs.
    let second = Midi::from_bytes(&fs::read(folder.join("sequence-001.mid")).unwrap()).unwrap();
    assert!(first.tracks[1]
        .events
        .iter()
        .filter_map(|e| match e.message {
            Message::Channel { status, .. } if status & 0xf0 == 0x90 => Some(status),
            _ => None,
        })
        .all(|s| s & 15 == 0));
    assert!(second.tracks[1]
        .events
        .iter()
        .filter_map(|e| match e.message {
            Message::Channel { status, .. } if status & 0xf0 == 0x90 => Some(status),
            _ => None,
        })
        .all(|s| s & 15 == 9));
    assert!(bof3_audio::music::extraction::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    let bad = d.0.join("bad.EMI");
    fs::write(&bad, synthetic_archive(true)).unwrap();
    let mut opts = options(&d.0, bad);
    opts.output = d.0.join("failed");
    assert!(bof3_audio::music::extraction::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("controller"));
    assert!(!opts.output.exists());
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS and BOF3_AUDIO_EXE; original supported song and rejected pitch contexts"]
fn original_song_cli_exports_playable_interchange_and_rejects_unverified_ranges() {
    let d = Directory::new();
    let corpus = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let source = corpus.join("BIN/BGM/BGM004.EMI");
    let opts = options(&d.0, source.clone());
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["extract", "--mode", "music", "--archive"])
        .arg(&source)
        .arg("--executable")
        .arg(&opts.executable)
        .arg("--output")
        .arg(&opts.output)
        .args(["--allow-approximations", "--json"])
        .output()
        .unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    let count = check_export(&opts.output, &fs::read(source).unwrap());
    assert_eq!(
        count,
        report["songs"][0]["sequences"].as_array().unwrap().len()
    );
    for name in ["BGM000.EMI", "BGM002.EMI"] {
        let mut opts = options(&d.0, corpus.join("BIN/BGM").join(name));
        opts.output = d.0.join(name);
        let error = bof3_audio::music::extraction::songs(&opts)
            .unwrap_err()
            .to_string();
        assert!(error.contains("outside verified pitch table"), "{error}");
        assert!(error.contains("lookup 0x8018"), "{error}");
        assert!(error.contains("isolated_executable"), "{error}");
        if name == "BGM000.EMI" {
            assert!(error.contains("0x8018440C"), "{error}");
            assert!(error.contains("AdjacentData"), "{error}");
        }
        assert!(!opts.output.exists());
    }
}
