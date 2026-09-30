use bof3_audio::{
    document::manifest, interchange::wave::Wave, xa::extraction::Kind, xa::Arithmetic,
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
        let p = std::env::temp_dir().join(format!(
            "bof3-xa-pack-{}-{}",
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
fn sector(channel: u8, coding: u8, value: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, channel, 0x64, coding, 1, channel, 0x64, coding]);
    for group in 0..18 {
        let at = 8 + group * 128;
        bytes[at..at + 16].fill(if coding & 16 == 0 { 12 } else { 8 });
        bytes[at + 16..at + 128].fill(value);
    }
    bytes[2312..2332].fill(0xa5);
    bytes
}
fn extract(
    input: &Path,
    output: &Path,
    exe: Option<PathBuf>,
    kind: Kind,
) -> bof3_audio::xa::extraction::Report {
    bof3_audio::xa::extraction::extract(&bof3_audio::xa::extraction::Options {
        disc_root: None,
        archives: vec![input.into()],
        executable: exe,
        output: output.into(),
        id: None,
        kind,
        arithmetic: Arithmetic::CombinedRounded,
    })
    .unwrap()
}
fn assets(root: &Path) -> Vec<PathBuf> {
    manifest::read(&root.join("audio.xml"))
        .unwrap()
        .children
        .iter()
        .filter(|n| n.name == "asset")
        .map(|n| manifest::relative_file(root, root, n.attribute("path").unwrap()).unwrap())
        .collect()
}
fn wav_path(root: &Path, asset: &Path) -> PathBuf {
    let node = manifest::read(asset).unwrap();
    manifest::relative_file(
        root,
        asset.parent().unwrap(),
        node.child("wave").unwrap().attribute("path").unwrap(),
    )
    .unwrap()
}

#[test]
fn streams_pack_without_original_sources_and_preserve_interleaving_and_opaque_bytes() {
    for coding in [0, 1, 4, 5, 16, 17, 20, 21] {
        let directory = Directory::new();
        let source = directory.0.join("audio & 'é'.STR");
        let mut opaque = vec![0xd4; 2336];
        opaque[..8].copy_from_slice(&[7, 0, 0x28, 0, 7, 0, 0x28, 0]);
        let bytes = [
            sector(0, coding, 0x11),
            sector(1, coding, 0x22),
            opaque,
            sector(0, coding, 0x33),
        ]
        .concat();
        fs::write(&source, &bytes).unwrap();
        let export = directory.0.join("export");
        extract(&source, &export, None, Kind::Stream);
        fs::remove_file(source).unwrap();
        let mut options = bof3_audio::xa::packing::Options {
            input: export.clone(),
            executable: None,
            output: directory.0.join("unchanged"),
        };
        let report = bof3_audio::xa::packing::assets(&options).unwrap();
        assert_eq!(report.assets.len(), 2);
        assert!(report.sources[0].byte_equal);
        assert_eq!(
            fs::read(options.output.join(&report.sources[0].path)).unwrap(),
            bytes
        );
        let path = assets(&export)
            .into_iter()
            .find(|p| {
                manifest::read(p)
                    .unwrap()
                    .child("stream")
                    .unwrap()
                    .number::<u8>("channel")
                    .unwrap()
                    == 0
            })
            .unwrap();
        let wav = wav_path(&export, &path);
        let mut wave = Wave::from_bytes(&fs::read(&wav).unwrap()).unwrap();
        for p in wave.pcm.iter_mut().step_by(17) {
            *p = p.saturating_add(2048);
        }
        fs::write(wav, wave.to_bytes().unwrap()).unwrap();
        options.output = directory.0.join("edited");
        let report = bof3_audio::xa::packing::assets(&options).unwrap();
        assert_eq!(report.sources[0].changed_sectors, [0, 3]);
        let out = fs::read(options.output.join(&report.sources[0].path)).unwrap();
        assert_eq!(&out[2336..3 * 2336], &bytes[2336..3 * 2336]);
        for i in [0, 3] {
            assert_eq!(&out[i * 2336..i * 2336 + 8], &bytes[i * 2336..i * 2336 + 8]);
            assert_eq!(
                &out[i * 2336 + 2312..i * 2336 + 2332],
                &bytes[i * 2336 + 2312..i * 2336 + 2332]
            );
        }
    }
}

#[test]
fn manifest_conflicts_capacity_failures_and_bad_metadata_do_not_publish() {
    let directory = Directory::new();
    let source = directory.0.join("source.STR");
    fs::write(&source, sector(0, 0, 0x11)).unwrap();
    let export = directory.0.join("export");
    extract(&source, &export, None, Kind::Stream);
    let opts = bof3_audio::xa::packing::Options {
        input: export.clone(),
        executable: None,
        output: directory.0.join("failed"),
    };
    let asset = assets(&export).remove(0);
    let original = fs::read_to_string(&asset).unwrap();
    for (from, to, reason) in [
        ("submode=\"100\"", "submode=\"101\"", "submode"),
        (
            "initial_history=\"zero_per_exported_asset\"",
            "initial_history=\"guessed\"",
            "initial_history",
        ),
        ("frames=\"4032\"", "frames=\"4033\"", "frames"),
    ] {
        assert!(original.contains(from));
        fs::write(&asset, original.replace(from, to)).unwrap();
        let error = bof3_audio::xa::packing::assets(&opts)
            .unwrap_err()
            .to_string();
        assert!(error.contains(reason), "{error}");
        assert!(!opts.output.exists());
    }
    fs::write(&asset, original).unwrap();
    let wav = wav_path(&export, &asset);
    let mut wave = Wave::from_bytes(&fs::read(&wav).unwrap()).unwrap();
    wave.pcm.pop();
    fs::write(&wav, wave.to_bytes().unwrap()).unwrap();
    assert!(bof3_audio::xa::packing::assets(&opts)
        .unwrap_err()
        .to_string()
        .contains("capacity"));
    assert!(!opts.output.exists());
    wave.pcm.push(0);
    wave.sample_rate = 44100;
    fs::write(&wav, wave.to_bytes().unwrap()).unwrap();
    assert!(bof3_audio::xa::packing::assets(&opts)
        .unwrap_err()
        .to_string()
        .contains("rate/channel"));
    assert!(!opts.output.exists());
    assert_eq!(
        fs::read_dir(&directory.0)
            .unwrap()
            .filter(|e| e
                .as_ref()
                .unwrap()
                .file_name()
                .to_string_lossy()
                .starts_with(".bof3-extract"))
            .count(),
        0
    );
}

#[test]
fn cli_dispatches_xa_roots_without_an_executable_and_preserves_existing_output() {
    let directory = Directory::new();
    let source = directory.0.join("source.STR");
    fs::write(&source, sector(0, 0, 0)).unwrap();
    let export = directory.0.join("export");
    extract(&source, &export, None, Kind::Stream);
    let output = directory.0.join("packed");
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["pack", "--mode", "audio", "--input"])
        .arg(&export)
        .arg("--output")
        .arg(&output)
        .arg("--json")
        .output()
        .unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(report["sources"][0]["byte_equal"], true);
    assert!(
        bof3_audio::xa::packing::assets(&bof3_audio::xa::packing::Options {
            input: export,
            executable: None,
            output: output.clone()
        })
        .unwrap_err()
        .to_string()
        .contains("already exists")
    );
    assert!(output.join("pack.json").is_file());
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK and BOF3_AUDIO_EXE; complete stream and cue XML/WAV round trips"]
fn complete_original_xa_sources_pack_stream_and_cue_exports_byte_exactly() {
    use bof3_audio::archive::disc::DiscImage;
    let directory = Directory::new();
    let exe = PathBuf::from(std::env::var_os("BOF3_AUDIO_EXE").unwrap());
    let mut disc =
        DiscImage::open(Path::new(&std::env::var_os("BOF3_AUDIO_TRACK").unwrap())).unwrap();
    let mut streams = 0;
    let mut cues = 0;
    for name in [
        "BIN/BMAG_XA/MAGIC00.STR",
        "BIN/SCE_XA/S_XA00.STR",
        "BIN/SCE_XA/VOICE.STR",
        "LOGO/CAPCOM30.STR",
    ] {
        let image = disc.read_xa(name).unwrap();
        let source = directory.0.join("original.STR");
        fs::write(&source, image.bytes()).unwrap();
        for kind in [Kind::Stream, Kind::Cue] {
            if name.starts_with("LOGO/") && matches!(kind, Kind::Cue) {
                continue;
            }
            let export = directory.0.join("export");
            let packed = directory.0.join("packed");
            let extracted = extract(&source, &export, Some(exe.clone()), kind);
            match kind {
                Kind::Stream => streams += extracted.assets,
                Kind::Cue => cues += extracted.assets,
            }
            let report = bof3_audio::xa::packing::assets(&bof3_audio::xa::packing::Options {
                input: export.clone(),
                executable: Some(exe.clone()),
                output: packed.clone(),
            })
            .unwrap_or_else(|e| panic!("{name} {}: {e}", kind.name()));
            assert_eq!(report.sources.len(), 1);
            assert!(report.sources[0].byte_equal);
            assert_eq!(
                fs::read(packed.join(&report.sources[0].path)).unwrap(),
                image.bytes()
            );
            println!(
                "{name} {}: {} assets, exact {} bytes",
                kind.name(),
                extracted.assets,
                image.bytes().len()
            );
            fs::remove_dir_all(export).unwrap();
            fs::remove_dir_all(packed).unwrap();
        }
        fs::remove_file(source).unwrap();
    }
    assert_eq!((streams, cues), (31, 896));
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK and BOF3_AUDIO_EXE; edited original cue and runtime metadata validation"]
fn original_cue_edit_preserves_other_sectors_and_rejects_runtime_conflicts() {
    use bof3_audio::archive::disc::DiscImage;
    let directory = Directory::new();
    let executable = PathBuf::from(std::env::var_os("BOF3_AUDIO_EXE").unwrap());
    let mut disc =
        DiscImage::open(Path::new(&std::env::var_os("BOF3_AUDIO_TRACK").unwrap())).unwrap();
    let image = disc.read_xa("BIN/BMAG_XA/MAGIC00.STR").unwrap();
    let source = directory.0.join("original.STR");
    fs::write(&source, image.bytes()).unwrap();
    let export = directory.0.join("export");
    bof3_audio::xa::extraction::extract(&bof3_audio::xa::extraction::Options {
        disc_root: None,
        archives: vec![source],
        executable: Some(executable.clone()),
        output: export.clone(),
        id: Some("4096".into()),
        kind: Kind::Cue,
        arithmetic: Arithmetic::CombinedRounded,
    })
    .unwrap();
    let asset = assets(&export).remove(0);
    let node = manifest::read(&asset).unwrap();
    let indices = node
        .child("sectors")
        .unwrap()
        .children
        .iter()
        .map(|n| n.number::<usize>("index").unwrap())
        .collect::<Vec<_>>();
    assert_eq!(indices[0], 0);
    let wav = wav_path(&export, &asset);
    let mut wave = Wave::from_bytes(&fs::read(&wav).unwrap()).unwrap();
    for value in wave.pcm.iter_mut().take(112) {
        *value = value.saturating_add(257);
    }
    fs::write(wav, wave.to_bytes().unwrap()).unwrap();
    let mut options = bof3_audio::xa::packing::Options {
        input: export,
        executable: Some(executable),
        output: directory.0.join("edited"),
    };
    let report = bof3_audio::xa::packing::assets(&options).unwrap();
    assert!(!report.sources[0].byte_equal);
    assert!(!report.sources[0].changed_sectors.is_empty());
    assert!(report.sources[0]
        .changed_sectors
        .iter()
        .all(|i| indices.contains(i)));
    let bytes = fs::read(options.output.join(&report.sources[0].path)).unwrap();
    for sector in image.sectors() {
        if !indices.contains(&sector.index) {
            assert_eq!(
                &bytes[sector.index * 2336..(sector.index + 1) * 2336],
                image.sector(sector.index).unwrap()
            );
        }
    }
    options.output = directory.0.join("conflict");
    let text = fs::read_to_string(&asset).unwrap();
    let selector = node
        .child("runtime")
        .unwrap()
        .attribute("selector")
        .unwrap();
    fs::write(
        &asset,
        text.replace(&format!("selector=\"{selector}\""), "selector=\"0\""),
    )
    .unwrap();
    assert!(bof3_audio::xa::packing::assets(&options)
        .unwrap_err()
        .to_string()
        .contains("selector"));
    assert!(!options.output.exists());
}
