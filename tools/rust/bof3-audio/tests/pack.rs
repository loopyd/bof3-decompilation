use bof3_audio::{
    bank::extraction, catalog::model::Catalog, document::manifest, interchange::wave::Wave, pack,
};
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
            "bof3-pack-{}-{}",
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

fn options(input: &Path, output: &Path) -> pack::Options {
    pack::Options {
        input: input.into(),
        output: output.into(),
        executable: std::env::var_os("BOF3_AUDIO_EXE")
            .map(PathBuf::from)
            .unwrap_or_else(|| PathBuf::from("missing-executable")),
    }
}

fn bank_path(root: &Path) -> PathBuf {
    let node = manifest::read(&root.join("audio.xml")).unwrap();
    manifest::relative_file(root, root, node.children[0].attribute("path").unwrap()).unwrap()
}

#[test]
fn refuses_ambiguous_roots_empty_roots_escape_paths_and_existing_outputs() {
    let directory = Directory::new();
    let input = directory.0.join("input");
    fs::create_dir(&input).unwrap();
    let output = directory.0.join("result");
    let opts = options(&input, &output);
    fs::write(
        input.join("audio.xml"),
        "<audio schema=\"bof3.audio-extraction/v1\" mode=\"audio\" kind=\"bank\"/>",
    )
    .unwrap();
    assert!(pack::banks(&opts)
        .unwrap_err()
        .to_string()
        .contains("no banks"));
    fs::write(input.join("bank.xml"), "<bank/>").unwrap();
    assert!(pack::banks(&opts)
        .unwrap_err()
        .to_string()
        .contains("ambiguous"));
    fs::remove_file(input.join("bank.xml")).unwrap();
    fs::write(directory.0.join("outside.xml"), "<bank/>").unwrap();
    fs::write(input.join("audio.xml"), "<audio schema=\"bof3.audio-extraction/v1\" mode=\"audio\" kind=\"bank\"><bank id=\"x\" path=\"../outside.xml\" source_sha256=\"x\"/></audio>").unwrap();
    assert!(pack::banks(&opts)
        .unwrap_err()
        .to_string()
        .contains("escape"));
    assert!(!output.exists());
    fs::write(&output, "existing").unwrap();
    assert!(pack::banks(&opts)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    assert_eq!(fs::read(output).unwrap(), b"existing");
}

#[test]
fn cli_rejects_unsupported_modes_and_repeated_arguments() {
    for args in [
        vec!["pack", "--mode", "unknown"],
        vec!["pack", "--mode", "audio", "--input", "x", "--input", "y"],
    ] {
        let output = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(args)
            .output()
            .unwrap();
        assert!(!output.status.success());
        let error = String::from_utf8(output.stderr).unwrap();
        assert!(
            error.contains("unsupported") || error.contains("repeated"),
            "{error}"
        );
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS and BOF3_AUDIO_EXE; full folder packing and failure publication"]
fn original_bank_folders_pack_standalone_and_as_root_with_verified_edits() {
    let directory = Directory::new();
    let corpus = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let input = directory.0.join("input");
    fs::create_dir(&input).unwrap();
    // Delete the copied inputs after extraction to prove XML preservation is sufficient.
    let mut originals = std::collections::BTreeMap::new();
    for name in ["BGM000.EMI", "BGM002.EMI"] {
        let path = input.join(name);
        let bytes = fs::read(corpus.join("BIN/BGM").join(name)).unwrap();
        fs::write(&path, &bytes).unwrap();
        originals.insert(
            path.canonicalize().unwrap().to_str().unwrap().to_owned(),
            bytes,
        );
    }
    let export = directory.0.join("export");
    let mut opts = options(&export, &directory.0.join("unchanged"));
    extraction::banks(&extraction::Options {
        disc_root: Some(input.clone()),
        archives: vec![],
        executable: opts.executable.clone(),
        output: export.clone(),
        id: None,
        reference_rate: 44100,
    })
    .unwrap();
    // Disc-root identities are relative.
    let originals = originals
        .into_iter()
        .map(|(p, b)| {
            (
                Path::new(&p)
                    .file_name()
                    .unwrap()
                    .to_str()
                    .unwrap()
                    .to_owned(),
                b,
            )
        })
        .collect::<std::collections::BTreeMap<_, _>>();
    fs::remove_dir_all(input).unwrap();
    let report = pack::banks(&opts).unwrap();
    assert_eq!(report.archives.len(), 2);
    for archive in &report.archives {
        assert!(archive.byte_equal);
        assert_eq!(
            fs::read(opts.output.join(&archive.path)).unwrap(),
            originals[&archive.source]
        );
    }
    let path = bank_path(&export);
    let manifest_text = fs::read_to_string(&path).unwrap();
    let node = manifest::read(&path).unwrap();
    let sample = &node.child("samples").unwrap().children[0];
    let wav_path = manifest::relative_file(
        &export,
        path.parent().unwrap(),
        sample.attribute("path").unwrap(),
    )
    .unwrap();
    let original_wav = fs::read(&wav_path).unwrap();
    let mut wave = Wave::from_bytes(&original_wav).unwrap();
    for value in wave.pcm.iter_mut().step_by(17) {
        *value = value.saturating_add(511);
    }
    fs::write(&wav_path, wave.to_bytes().unwrap()).unwrap();
    opts.output = directory.0.join("edited");
    let edited = pack::banks(&opts).unwrap();
    assert_eq!(
        edited
            .banks
            .iter()
            .flat_map(|b| &b.samples)
            .filter(|s| s.changed)
            .count(),
        1
    );
    assert_eq!(edited.archives.iter().filter(|a| !a.byte_equal).count(), 1);
    let archive = edited.archives.iter().find(|a| !a.byte_equal).unwrap();
    let original =
        emi_ex_v2::image::ArchiveImage::from_bytes(originals[&archive.source].clone()).unwrap();
    let rebuilt = emi_ex_v2::image::ArchiveImage::from_bytes(
        fs::read(opts.output.join(&archive.path)).unwrap(),
    )
    .unwrap();
    assert_eq!(original.bytes().len(), rebuilt.bytes().len());
    for (entry, _) in original.entries().iter().enumerate() {
        if entry != node.number::<usize>("body_entry").unwrap() {
            assert_eq!(
                original.entry(entry).unwrap(),
                rebuilt.entry(entry).unwrap()
            );
        }
    }
    opts.input = path.parent().unwrap().into();
    opts.output = directory.0.join("standalone");
    let standalone = pack::banks(&opts).unwrap();
    assert_eq!(standalone.archives.len(), 1);
    assert_eq!(standalone.archives[0].output_sha256, archive.output_sha256);
    // Invalid XML edits and WAV capacity failures never publish output.
    opts.output = directory.0.join("failure");
    fs::write(
        &path,
        manifest_text.replacen("content_type=\"unresolved\"", "content_type=\"vocals\"", 1),
    )
    .unwrap();
    assert!(pack::banks(&opts)
        .unwrap_err()
        .to_string()
        .contains("content_type"));
    assert!(!opts.output.exists());
    fs::write(&path, &manifest_text).unwrap();
    wave.sampler.as_mut().unwrap().loops.clear();
    wave.pcm.resize(
        sample.number::<usize>("encoded_bytes").unwrap() / 16 * 28 + 28,
        0,
    );
    fs::write(&wav_path, wave.to_bytes().unwrap()).unwrap();
    assert!(pack::banks(&opts)
        .unwrap_err()
        .to_string()
        .contains("capacity"));
    assert!(!opts.output.exists());
    fs::write(wav_path, original_wav).unwrap();
    // A CLI pack performs the same verified publication.
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["pack", "--mode", "audio", "--input"])
        .arg(&opts.input)
        .arg("--executable")
        .arg(&opts.executable)
        .arg("--output")
        .arg(&opts.output)
        .arg("--json")
        .output()
        .unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    assert_eq!(
        serde_json::from_slice::<serde_json::Value>(&result.stdout).unwrap()["archives"][0]
            ["byte_equal"],
        true
    );
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS and BOF3_AUDIO_EXE; extracts and packs every original EMI with a bank"]
fn entire_bank_corpus_round_trips_whole_emi_files() {
    let directory = Directory::new();
    let corpus = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let catalog = Catalog::read(Some(&corpus), &[]).unwrap();
    let mut counts = (0, 0, 0);
    for source in catalog
        .sources
        .iter()
        .filter(|s| s.entries.iter().any(|e| e.file_type == 6))
    {
        let export = directory.0.join("export");
        let output = directory.0.join("packed");
        let opts = options(&export, &output);
        let extraction = extraction::banks(&extraction::Options {
            disc_root: None,
            archives: vec![corpus.join(&source.source)],
            executable: opts.executable.clone(),
            output: export.clone(),
            id: None,
            reference_rate: 44100,
        })
        .unwrap();
        let report = pack::banks(&opts).unwrap_or_else(|e| panic!("{}: {e}", source.source));
        assert_eq!(report.archives.len(), 1);
        assert!(report.archives[0].byte_equal);
        assert_eq!(
            fs::read(output.join(&report.archives[0].path)).unwrap(),
            fs::read(corpus.join(&source.source)).unwrap()
        );
        counts.0 += 1;
        counts.1 += extraction.banks;
        counts.2 += extraction.samples;
        fs::remove_dir_all(export).unwrap();
        fs::remove_dir_all(output).unwrap();
    }
    assert_eq!((counts.1, counts.2), (1020, 8385));
    println!(
        "whole-file bank corpus: {} EMI archives, {} banks, {} samples",
        counts.0, counts.1, counts.2
    );
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS and BOF3_AUDIO_EXE; shared-source edits and preservation conflicts"]
fn multiple_banks_merge_edits_but_reject_conflicting_preservation() {
    use bof3_audio::digest::sha256_hex;
    use emi_ex_v2::image::ArchiveImage;
    let directory = Directory::new();
    let corpus = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let original =
        ArchiveImage::from_bytes(fs::read(corpus.join("BIN/BGM/BGM000.EMI")).unwrap()).unwrap();
    let entries = [
        (6u16, original.entry(0).unwrap()),
        (7, original.entry(2).unwrap()),
        (6, original.entry(0).unwrap()),
        (7, original.entry(2).unwrap()),
        (0x55, b"opaque unrelated data".as_slice()),
    ];
    let mut emi = vec![0; 2048];
    emi[..4].copy_from_slice(&(entries.len() as u32).to_le_bytes());
    emi[8..16].copy_from_slice(b"MATH_TBL");
    for (index, (kind, bytes)) in entries.into_iter().enumerate() {
        let at = 16 + index * 16;
        emi[at..at + 4].copy_from_slice(&(bytes.len() as u32).to_le_bytes());
        emi[at + 4..at + 8].copy_from_slice(&(if index < 2 { 0u32 } else { 1u32 }).to_le_bytes());
        emi[at + 8..at + 12].copy_from_slice(b"OLD!"); // stale cached first word remains byte-exact if unchanged
        emi[at + 12..at + 14].copy_from_slice(&kind.to_le_bytes());
        emi[at + 14..at + 16].copy_from_slice(b"XY");
        emi.extend(bytes);
        emi.resize(emi.len().div_ceil(2048) * 2048, 0xa5);
    }
    emi.extend(b"unrelated trailer");
    let source = directory.0.join("shared.EMI");
    fs::write(&source, &emi).unwrap();
    let export = directory.0.join("export");
    let mut opts = options(&export, &directory.0.join("unchanged"));
    extraction::banks(&extraction::Options {
        disc_root: None,
        archives: vec![source],
        executable: opts.executable.clone(),
        output: export.clone(),
        id: None,
        reference_rate: 44100,
    })
    .unwrap();
    let root_path = export.join("audio.xml");
    let root_text = fs::read_to_string(&root_path).unwrap();
    let root = manifest::read(&root_path).unwrap();
    assert_eq!(root.children.len(), 2);
    let report = pack::banks(&opts).unwrap();
    assert_eq!(
        fs::read(opts.output.join(&report.archives[0].path)).unwrap(),
        emi
    );
    for link in &root.children {
        let path =
            manifest::relative_file(&export, &export, link.attribute("path").unwrap()).unwrap();
        let node = manifest::read(&path).unwrap();
        let sample = &node.child("samples").unwrap().children[0];
        let wav = manifest::relative_file(
            &export,
            path.parent().unwrap(),
            sample.attribute("path").unwrap(),
        )
        .unwrap();
        let mut wave = Wave::from_bytes(&fs::read(&wav).unwrap()).unwrap();
        for value in wave.pcm.iter_mut().step_by(7) {
            *value = value.saturating_add(2048);
        }
        fs::write(wav, wave.to_bytes().unwrap()).unwrap();
    }
    opts.output = directory.0.join("edited");
    let report = pack::banks(&opts).unwrap();
    assert_eq!(report.archives.len(), 1);
    assert_eq!(report.archives[0].changed_entries, [1, 3]);
    let rebuilt =
        ArchiveImage::from_bytes(fs::read(opts.output.join(&report.archives[0].path)).unwrap())
            .unwrap();
    let original = ArchiveImage::from_bytes(emi.clone()).unwrap();
    for index in [0, 2, 4] {
        assert_eq!(
            rebuilt.entry(index).unwrap(),
            original.entry(index).unwrap()
        );
    }
    // Every changed byte is in a selected payload or its cached TOC first word.
    for (offset, (a, b)) in emi.iter().zip(rebuilt.bytes()).enumerate() {
        if a != b {
            assert!(
                [1, 3].into_iter().any(|index| {
                    let e = &original.entries()[index];
                    (e.offset as usize..e.offset as usize + e.size as usize).contains(&offset)
                        || (24 + index * 16..28 + index * 16).contains(&offset)
                }),
                "unexpected changed byte {offset}"
            );
        }
    }
    let link = &root.children[1];
    let path = manifest::relative_file(&export, &export, link.attribute("path").unwrap()).unwrap();
    let mut text = fs::read_to_string(&path).unwrap();
    let old_hash = sha256_hex(&emi);
    *emi.last_mut().unwrap() ^= 1;
    let new_hash = sha256_hex(&emi);
    text = text.replace(&old_hash, &new_hash);
    let end = text.find("</preservation>").unwrap();
    text.replace_range(end - 2..end, &format!("{:02x}", emi.last().unwrap()));
    fs::write(path, text).unwrap();
    let changed_root = root_text
        .lines()
        .map(|line| {
            if line.contains(link.attribute("path").unwrap()) {
                line.replace(&old_hash, &new_hash)
            } else {
                line.to_owned()
            }
        })
        .collect::<Vec<_>>()
        .join("\n");
    fs::write(root_path, changed_root).unwrap();
    opts.output = directory.0.join("conflict");
    assert!(pack::banks(&opts)
        .unwrap_err()
        .to_string()
        .contains("conflicting preserved archives"));
    assert!(!opts.output.exists());
}
