#[path = "../support/music/assignment.rs"]
mod assignment_music;
use crate::fixture as music_fixture;
#[path = "../support/music/pitch.rs"]
mod pitch_music;
#[path = "../support/music/timing.rs"]
mod timing_music;
#[path = "../support/soundfont/controls.rs"]
mod tone_controls;
#[path = "../support/music/activation.rs"]
mod unmute_music;
use bof3_audio::{
    document::manifest, interchange::midi::Message, interchange::midi::Midi, pack,
    sequence::midi as translation, sequence::timeline::Limits, sequence::SequenceSet,
};
use emi_ex_v2::image::ArchiveImage;
use std::{
    fs,
    path::{Path, PathBuf},
    process::Command,
    sync::atomic::{AtomicU64, Ordering},
};

struct Directory(PathBuf);
impl Directory {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-music-pack-{}-{}",
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

fn options(root: &Path, name: &str) -> pack::Options {
    pack::Options {
        input: root.join("export"),
        output: root.join(name),
        executable: std::env::var_os("BOF3_AUDIO_EXE")
            .map(PathBuf::from)
            .unwrap_or_else(|| "absent-executable".into()),
    }
}
fn song_path(root: &Path) -> PathBuf {
    let music = manifest::read(&root.join("music.xml")).unwrap();
    manifest::relative_file(root, root, music.children[0].attribute("path").unwrap()).unwrap()
}
fn extract(root: &Path, archive: &[u8]) -> PathBuf {
    let source = root.join("song & é.EMI");
    fs::write(&source, archive).unwrap();
    let opts = options(root, "unused");
    bof3_audio::music::extraction::songs(&bof3_audio::music::extraction::Options {
        disc_root: None,
        archives: vec![source.clone()],
        executable: opts.executable,
        output: opts.input.clone(),
        id: None,
        loops: 2,
        allow_approximations: true,
    })
    .unwrap();
    fs::remove_file(source).unwrap();
    song_path(&opts.input)
}

#[test]
fn root_validation_and_cli_fail_without_publishing() {
    let d = Directory::new();
    let opts = options(&d.0, "result");
    fs::create_dir(&opts.input).unwrap();
    fs::write(
        opts.input.join("music.xml"),
        "<music schema=\"bof3.music-extraction/v1\"/>",
    )
    .unwrap();
    assert!(bof3_audio::music::packing::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("no songs"));
    fs::write(opts.input.join("song.xml"), "<song/>").unwrap();
    assert!(bof3_audio::music::packing::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("ambiguous"));
    fs::remove_file(opts.input.join("song.xml")).unwrap();
    fs::write(d.0.join("escape.xml"), "<song/>").unwrap();
    fs::write(opts.input.join("music.xml"), "<music schema=\"bof3.music-extraction/v1\"><song id=\"x\" bank=\"x\" path=\"../escape.xml\" source_sha256=\"x\"/></music>").unwrap();
    assert!(bof3_audio::music::packing::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("escape"));
    assert!(!opts.output.exists());
    fs::write(&opts.output, b"existing").unwrap();
    assert!(bof3_audio::music::packing::songs(&opts)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    assert_eq!(fs::read(&opts.output).unwrap(), b"existing");
    for args in [
        vec!["pack", "--mode", "music"],
        vec!["pack", "--mode", "music", "--input", "x", "--output", "y"],
    ] {
        let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(args)
            .output()
            .unwrap();
        assert!(!result.status.success());
        assert!(String::from_utf8_lossy(&result.stderr).contains("required"));
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original executable with synthetic layered bank and two independent sequences"]
fn song_root_and_standalone_folder_pack_original_and_edited_sequences() {
    let d = Directory::new();
    let original = music_fixture::synthetic_archive(false);
    let path = extract(&d.0, &original);
    let folder = path.parent().unwrap();
    let opts = options(&d.0, "unchanged");
    let report = bof3_audio::music::packing::songs(&opts).unwrap();
    assert_eq!(report.archives.len(), 1);
    assert!(report.archives[0].byte_equal);
    assert_eq!(
        fs::read(opts.output.join(&report.archives[0].path)).unwrap(),
        original
    );
    assert_eq!(report.songs[0].sequences.len(), 2);
    let mut standalone = options(&d.0, "standalone");
    standalone.input = folder.into();
    assert!(
        bof3_audio::music::packing::songs(&standalone)
            .unwrap()
            .archives[0]
            .byte_equal
    );
    let xml = fs::read_to_string(&path).unwrap();
    fs::rename(
        folder.join("sequence-000.mid"),
        folder.join("relocated.mid"),
    )
    .unwrap();
    fs::rename(folder.join("bank.sf2"), folder.join("relocated.sf2")).unwrap();
    fs::write(
        &path,
        xml.replace("path=\"sequence-000.mid\"", "path=\"relocated.mid\"")
            .replace("path=\"bank.sf2\"", "path=\"relocated.sf2\""),
    )
    .unwrap();
    standalone.output = d.0.join("relocated");
    assert!(
        bof3_audio::music::packing::songs(&standalone)
            .unwrap()
            .archives[0]
            .byte_equal
    );
    fs::rename(
        folder.join("relocated.mid"),
        folder.join("sequence-000.mid"),
    )
    .unwrap();
    fs::rename(folder.join("relocated.sf2"), folder.join("bank.sf2")).unwrap();
    fs::write(&path, &xml).unwrap();
    let midi_path = folder.join("sequence-000.mid");
    let mut midi = Midi::from_bytes(&fs::read(&midi_path).unwrap()).unwrap();
    for event in &mut midi.tracks[1].events {
        if let Message::Channel { status, data } = &mut event.message {
            if *status == 0x90 && data[1] != 0 {
                data[1] = 80;
            }
        }
    }
    fs::write(&midi_path, midi.to_bytes().unwrap()).unwrap();
    let opts = options(&d.0, "edited");
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["pack", "--mode", "music", "--input"])
        .arg(&opts.input)
        .arg("--output")
        .arg(&opts.output)
        .arg("--executable")
        .arg(&opts.executable)
        .arg("--json")
        .output()
        .unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(
        report["archives"][0]["changed_entries"],
        serde_json::json!([2])
    );
    assert_eq!(
        report["songs"][0]["sequences"][0]["changed_source_events"],
        1
    );
    let bytes = fs::read(
        opts.output
            .join(report["archives"][0]["path"].as_str().unwrap()),
    )
    .unwrap();
    let before = ArchiveImage::from_bytes(original.clone()).unwrap();
    let after = ArchiveImage::from_bytes(bytes.clone()).unwrap();
    for index in [0, 1, 3] {
        assert_eq!(before.entry(index).unwrap(), after.entry(index).unwrap());
    }
    let sep = after.entry(2).unwrap();
    let sequences = SequenceSet::parse(sep).unwrap();
    let first = &sequences.sequences[0];
    let mut translated = translation::translate(
        first,
        &sep[first.data_offset..first.data_offset + first.data_bytes],
        &Limits::default(),
    )
    .unwrap();
    translation::initialize_channels(&mut translated).unwrap();
    assert!(translated
        .midi
        .tracks
        .iter()
        .zip(&midi.tracks)
        .all(|(a, b)| a.events == b.events));
    let second = &sequences.sequences[1];
    assert_eq!(
        &sep[second.data_offset..],
        &before.entry(2).unwrap()[second.data_offset..]
    );
    let differing: Vec<_> = original
        .iter()
        .zip(&bytes)
        .enumerate()
        .filter(|(_, (a, b))| a != b)
        .map(|(i, _)| i)
        .collect();
    // A changed EMI entry refreshes its cached first word. This synthetic
    // archive deliberately started with a stale zero cache, which survives
    // unchanged packing but is repaired by the existing archive writer on edits.
    let cache = 0x10 + 2 * 0x10 + 8;
    assert_eq!(&bytes[cache..cache + 4], b"pQES");
    assert_eq!(
        differing
            .iter()
            .filter(|&&i| !(cache..cache + 4).contains(&i))
            .count(),
        1,
        "only the source velocity and selected entry cache change; padding/trailer survive"
    );
    let font_path = folder.join("bank.sf2");
    let mut sf2 = fs::read(&font_path).unwrap();
    let parsed = bof3_audio::soundfont::reader::Font::from_bytes(sf2.clone()).unwrap();
    let at = parsed
        .chunks
        .iter()
        .find(|c| c.id == *b"smpl")
        .unwrap()
        .data
        .start;
    sf2[at..at + 2].copy_from_slice(&10000i16.to_le_bytes());
    fs::write(&font_path, sf2).unwrap();
    let combined = options(&d.0, "combined");
    let report = bof3_audio::music::packing::songs(&combined).unwrap();
    assert_eq!(report.archives[0].changed_entries, [1, 2]);
    assert!(report.songs[0].soundfont.samples[0].content_changed);
    assert_eq!(report.songs[0].sequences[0].changed_source_events, 1);
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; conflicting manifests, unsupported edits and failed publication"]
fn conflicts_and_unsupported_edits_leave_no_output() {
    let d = Directory::new();
    let path = extract(&d.0, &music_fixture::synthetic_archive(false));
    let folder = path.parent().unwrap();
    let original = fs::read_to_string(&path).unwrap();
    let opts = options(&d.0, "failed");
    let reject = |reason: &str| {
        let error = bof3_audio::music::packing::songs(&opts)
            .unwrap_err()
            .to_string();
        assert!(error.contains(reason), "{reason}: {error}");
        assert!(!opts.output.exists());
        assert!(!fs::read_dir(&d.0).unwrap().any(|e| e
            .unwrap()
            .file_name()
            .to_string_lossy()
            .starts_with(".bof3-extract-")));
    };
    for (old, new, reason) in [
        (
            "complete_playback=\"unverified\"",
            "complete_playback=\"verified\"",
            "complete_playback",
        ),
        ("sequence_id=\"17\"", "sequence_id=\"18\"", "sequence_id"),
        (
            "generated_setup_events=\"11\"",
            "generated_setup_events=\"10\"",
            "generated_setup_events",
        ),
        (
            "path=\"sequence-000.mid\"",
            "path=\"../../../../escape.mid\"",
            "No such file",
        ),
        (
            "<sequences>",
            "<sequences unsupported=\"true\">",
            "unsupported",
        ),
    ] {
        assert!(original.contains(old));
        fs::write(&path, original.replacen(old, new, 1)).unwrap();
        reject(reason);
        fs::write(&path, &original).unwrap();
    }
    // Changing the preservation hash claim cannot authorize a changed SF2.
    let font_path = folder.join("bank.sf2");
    let font = fs::read(&font_path).unwrap();
    let mut edited_font = font.clone();
    let name = edited_font.windows(4).position(|x| x == b"INAM").unwrap() + 8;
    edited_font[name] ^= 1;
    fs::write(&font_path, edited_font).unwrap();
    reject("unsupported metadata or instrument edit");
    fs::write(&font_path, &font).unwrap();
    let midi_path = folder.join("sequence-000.mid");
    let bytes = fs::read(&midi_path).unwrap();
    let source = Midi::from_bytes(&bytes).unwrap();
    let mut midi = source.clone();
    midi.ppqn += 1;
    fs::write(&midi_path, midi.to_bytes().unwrap()).unwrap();
    reject("PPQN");
    fs::write(&midi_path, &bytes).unwrap();
    let mut missing_parent = options(&d.0, "absent/child");
    assert!(bof3_audio::music::packing::songs(&missing_parent)
        .unwrap_err()
        .to_string()
        .contains("parent directory"));
    missing_parent.output = d.0.join("existing");
    fs::create_dir(&missing_parent.output).unwrap();
    fs::write(missing_parent.output.join("sentinel"), "keep").unwrap();
    assert!(bof3_audio::music::packing::songs(&missing_parent)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    assert_eq!(
        fs::read(missing_parent.output.join("sentinel")).unwrap(),
        b"keep"
    );
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS and BOF3_AUDIO_EXE; original BGM004 song root round trip"]
fn original_song_archive_round_trips_without_source_media() {
    let d = Directory::new();
    let original = fs::read(
        PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()).join("BIN/BGM/BGM004.EMI"),
    )
    .unwrap();
    let path = extract(&d.0, &original);
    let opts = options(&d.0, "packed");
    let report = bof3_audio::music::packing::songs(&opts).unwrap();
    assert!(report.archives[0].byte_equal);
    assert_eq!(report.songs[0].sequences.len(), 4);
    assert_eq!(
        fs::read(opts.output.join(&report.archives[0].path)).unwrap(),
        original
    );
    let node = manifest::read(&path).unwrap();
    let bank = node.child("bank").unwrap();
    let body_entry = bank.number::<usize>("body_entry").unwrap();
    let font_path = path.parent().unwrap().join("bank.sf2");
    let mut sf2 = fs::read(&font_path).unwrap();
    let parsed = bof3_audio::soundfont::reader::Font::from_bytes(sf2.clone()).unwrap();
    let pcm = parsed
        .chunks
        .iter()
        .find(|c| c.id == *b"smpl")
        .unwrap()
        .data
        .start;
    // Change real sample content, leaving its metadata and every MIDI unchanged.
    let sample = &parsed.samples[0];
    let original_value = (parsed.sample_pcm24(0).unwrap()[0] / 256) as i16;
    let value = if original_value > 0 { -12000i16 } else { 12000 };
    let at = pcm + sample.start as usize * 2;
    sf2[at..at + 2].copy_from_slice(&value.to_le_bytes());
    fs::write(&font_path, sf2).unwrap();
    let opts = options(&d.0, "edited-sample");
    let report = bof3_audio::music::packing::songs(&opts).unwrap();
    assert_eq!(report.archives[0].changed_entries, [body_entry]);
    assert!(!report.songs[0].soundfont_byte_equal);
    let changed: Vec<_> = report.songs[0]
        .soundfont
        .samples
        .iter()
        .filter(|s| s.content_changed)
        .collect();
    assert_eq!(changed.len(), 1);
    assert!(changed[0].encoding.is_some());
    assert!(changed[0]
        .pcm24_total_loss
        .as_ref()
        .unwrap()
        .rms_error
        .is_finite());
    let before = ArchiveImage::from_bytes(original).unwrap();
    let after =
        ArchiveImage::from_bytes(fs::read(opts.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    for index in 0..before.entries().len() {
        if index != body_entry {
            assert_eq!(before.entry(index).unwrap(), after.entry(index).unwrap());
        }
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; two songs sharing one bank and source preservation conflicts"]
fn shared_bank_songs_merge_and_conflicting_archive_copies_fail() {
    let d = Directory::new();
    let (image, original) = assignment_music::fixture();
    extract(&d.0, &original);
    let opts = options(&d.0, "shared");
    let music_path = opts.input.join("music.xml");
    let music_text = fs::read_to_string(&music_path).unwrap();
    let music = manifest::read(&music_path).unwrap();
    assert_eq!(music.children.len(), 2);
    let paths: Vec<_> = music
        .children
        .iter()
        .map(|n| {
            manifest::relative_file(&opts.input, &opts.input, n.attribute("path").unwrap()).unwrap()
        })
        .collect();
    let report = bof3_audio::music::packing::songs(&opts).unwrap();
    assert_eq!(report.archives.len(), 1);
    assert_eq!(report.songs.len(), 2);
    assert_eq!(report.songs[0].bank, report.songs[1].bank);
    assert!(report.archives[0].byte_equal);
    let fonts: Vec<_> = paths
        .iter()
        .map(|p| p.parent().unwrap().join("bank.sf2"))
        .collect();
    let original_font = fs::read(&fonts[0]).unwrap();
    let mut changed_font = original_font.clone();
    let font = bof3_audio::soundfont::reader::Font::from_bytes(changed_font.clone()).unwrap();
    let pcm = font
        .chunks
        .iter()
        .find(|c| c.id == *b"smpl")
        .unwrap()
        .data
        .start;
    changed_font[pcm..pcm + 2].copy_from_slice(&10000i16.to_le_bytes());
    fs::write(&fonts[0], &changed_font).unwrap();
    let conflicting = options(&d.0, "conflicting-sample");
    let error = bof3_audio::music::packing::songs(&conflicting)
        .unwrap_err()
        .to_string();
    assert!(
        error.contains("selected songs sharing a bank must agree"),
        "{error}"
    );
    assert!(!conflicting.output.exists());
    fs::write(&fonts[1], &changed_font).unwrap();
    let agreed = options(&d.0, "agreed-sample");
    let report = bof3_audio::music::packing::songs(&agreed).unwrap();
    assert_eq!(report.archives[0].changed_entries, [1]);
    assert!(report
        .songs
        .iter()
        .all(|s| s.soundfont.samples.iter().any(|p| p.content_changed)));
    fs::write(&fonts[0], &original_font).unwrap();
    fs::write(&fonts[1], &original_font).unwrap();
    let bank = bof3_audio::bank::Bank::parse(image.entry(0).unwrap()).unwrap();
    let program = &bank.programs[0];
    let mut tone = program.tones[0].clone();
    tone.volume = 93;
    tone.pan = 44;
    let gain = bof3_audio::soundfont::bank::Gain::from_bank(
        &bank,
        program,
        &tone,
        bof3_audio::soundfont::gain::Model::RustySynth136,
    )
    .unwrap();
    let mut changed_gain = original_font.clone();
    tone_controls::set(
        &mut changed_gain,
        0,
        gain.fit.unwrap().parameters.unwrap(),
        true,
    );
    fs::write(&fonts[0], &changed_gain).unwrap();
    let conflicting = options(&d.0, "conflicting-tone");
    assert!(bof3_audio::music::packing::songs(&conflicting)
        .unwrap_err()
        .to_string()
        .contains("selected songs sharing a bank must agree"));
    assert!(!conflicting.output.exists());
    fs::write(&fonts[1], &changed_gain).unwrap();
    let agreed = options(&d.0, "agreed-tone");
    let report = bof3_audio::music::packing::songs(&agreed).unwrap();
    assert_eq!(report.archives[0].changed_entries, [0]);
    assert!(report
        .songs
        .iter()
        .all(|s| s.tone_controls.tones[0].changed));
    let rebuilt =
        ArchiveImage::from_bytes(fs::read(agreed.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    let source = ArchiveImage::from_bytes(original.clone()).unwrap();
    for i in 1..rebuilt.entries().len() {
        assert_eq!(rebuilt.entry(i).unwrap(), source.entry(i).unwrap());
    }
    pitch_music::check(
        options(&d.0, "conflicting-pitch"),
        &fonts,
        &original_font,
        &original,
    );
    assignment_music::check(
        options(&d.0, "conflicting-assignment"),
        &fonts,
        &original_font,
        &original,
    );
    let midi_path = paths[1].parent().unwrap().join("sequence-000.mid");
    let mut midi = Midi::from_bytes(&fs::read(&midi_path).unwrap()).unwrap();
    for e in &mut midi.tracks[1].events {
        if let Message::Channel { status: 0x90, data } = &mut e.message {
            if data[1] > 0 {
                data[1] -= 1;
            }
        }
    }
    fs::write(&midi_path, midi.to_bytes().unwrap()).unwrap();
    let edited_opts = options(&d.0, "shared-edited");
    let edited = bof3_audio::music::packing::songs(&edited_opts).unwrap();
    assert_eq!(edited.archives[0].changed_entries, [4]);
    let image = ArchiveImage::from_bytes(
        fs::read(edited_opts.output.join(&edited.archives[0].path)).unwrap(),
    )
    .unwrap();
    assert_eq!(
        image.entry(2).unwrap(),
        ArchiveImage::from_bytes(original.clone())
            .unwrap()
            .entry(2)
            .unwrap()
    );

    // Both copies individually hash correctly but cannot describe different
    // original bytes for the same qualified archive identity.
    let document = fs::read_to_string(&paths[1]).unwrap();
    let mut conflicting = original.clone();
    *conflicting.last_mut().unwrap() ^= 1;
    let hash = bof3_audio::digest::sha256_hex(&conflicting);
    let old_hash = bof3_audio::digest::sha256_hex(&original);
    let hex = |bytes: &[u8]| bytes.iter().map(|b| format!("{b:02x}")).collect::<String>();
    let changed = document
        .replace(&hex(&original), &hex(&conflicting))
        .replace(&old_hash, &hash);
    fs::write(&paths[1], changed).unwrap();
    let second = music_text.rfind(&old_hash).unwrap();
    let mut changed_root = music_text.clone();
    changed_root.replace_range(second..second + old_hash.len(), &hash);
    fs::write(&music_path, changed_root).unwrap();
    let failed = options(&d.0, "conflict");
    let error = bof3_audio::music::packing::songs(&failed)
        .unwrap_err()
        .to_string();
    assert!(error.contains("conflicting preserved archives"), "{error}");
    assert!(!failed.output.exists());
    fs::write(&paths[1], &document).unwrap();
    fs::write(&music_path, &music_text).unwrap();

    let alias = opts.input.join("alias.xml");
    fs::copy(&paths[0], &alias).unwrap();
    let start = music_text.find("<song ").unwrap();
    let end = start + music_text[start..].find("/>").unwrap() + 2;
    let duplicate =
        music_text[start..end].replace(music.children[0].attribute("path").unwrap(), "alias.xml");
    fs::write(
        &music_path,
        music_text.replace("</music>", &(duplicate + "</music>")),
    )
    .unwrap();
    let error = bof3_audio::music::packing::songs(&failed)
        .unwrap_err()
        .to_string();
    assert!(error.contains("duplicate song identity"), "{error}");
    assert!(!failed.output.exists());
}
