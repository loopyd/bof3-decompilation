use bof3_audio::{
    archive::disc::DiscImage, xa::extraction::extract, xa::extraction::Kind,
    xa::extraction::Options, xa::Arithmetic,
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
            "bof3-xa-extract-{}-{}",
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

fn sector(file: u8, channel: u8, coding: u8, value: u8) -> Vec<u8> {
    let mut bytes = vec![0xa5; 2336];
    bytes[..8].copy_from_slice(&[file, channel, 0xe4, coding, file, channel, 0xe4, coding]);
    for group in 0..18 {
        let at = 8 + group * 128;
        bytes[at..at + 16].fill(12);
        bytes[at + 16..at + 128].fill(value);
    }
    bytes
}

fn options(directory: &Directory, archive: PathBuf) -> Options {
    Options {
        disc_root: None,
        archives: vec![archive],
        executable: None,
        output: directory.0.join("export"),
        id: None,
        kind: Kind::Stream,
        arithmetic: Arithmetic::CombinedRounded,
    }
}

#[test]
#[ignore = "requires independent Rust XML/WAV and FFmpeg consumers"]
fn multiplexed_streams_preserve_unknown_sectors_tails_and_file_channel_identities() {
    let directory = Directory::new();
    let input = directory.0.join("audio&'\".STR");
    let mut opaque = sector(255, 255, 0, 0xaa);
    opaque[2] = 0;
    opaque[6] = 0;
    fs::write(
        &input,
        [
            sector(1, 0, 0, 0x11),
            opaque,
            sector(1, 1, 1, 0x23),
            sector(2, 0, 0, 0x67),
            sector(1, 0, 0, 0x45),
        ]
        .concat(),
    )
    .unwrap();
    let mut options = options(&directory, input);
    options.id = Some("0".into());
    assert!(extract(&options)
        .unwrap_err()
        .to_string()
        .contains("ambiguous"));
    options.id = None;
    let report = extract(&options).unwrap();
    assert_eq!(
        (
            report.sources,
            report.assets,
            report.sectors,
            report.pcm_frames
        ),
        (1, 3, 4, 14112)
    );
    super::consumer::validate(&options.output, None, None);
    assert!(extract(&options)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    let output = directory.0.join("cli");
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args([
            "extract",
            "--mode",
            "audio",
            "--kind",
            "xa_stream",
            "--xa-arithmetic",
            "combined-rounded",
            "--archive",
        ])
        .arg(&options.archives[0])
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
    let json: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(json["assets"], 3);
    super::consumer::validate(&output, None, None);
}

#[test]
fn invalid_sectors_and_inapplicable_options_cannot_publish_output() {
    let directory = Directory::new();
    let input = directory.0.join("bad.STR");
    let mut data = sector(1, 0, 0, 0x11);
    data[8 + 17 * 128..24 + 17 * 128].fill(0xfc);
    fs::write(&input, data).unwrap();
    let options = options(&directory, input);
    assert!(extract(&options)
        .unwrap_err()
        .to_string()
        .contains("unsupported parameter"));
    assert!(!options.output.exists());
    assert_eq!(fs::read_dir(&directory.0).unwrap().count(), 1);
    for extra in [
        vec![],
        vec!["--xa-arithmetic", "bogus"],
        vec![
            "--xa-arithmetic",
            "combined-rounded",
            "--reference-rate",
            "44100",
        ],
    ] {
        let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args([
                "extract",
                "--mode",
                "audio",
                "--kind",
                "xa_stream",
                "--output",
            ])
            .arg(&options.output)
            .args(extra)
            .output()
            .unwrap();
        assert_eq!(result.status.code(), Some(2));
        assert!(!options.output.exists());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK, BOF3_AUDIO_EXE, BOF3_AUDIO_CORPUS and Rust/FFmpeg consumers"]
fn complete_original_streams_and_all_cues_export_with_independent_pcm_and_table_checks() {
    let directory = Directory::new();
    let media = directory.0.join("media");
    fs::create_dir(&media).unwrap();
    let exe = PathBuf::from(std::env::var_os("BOF3_AUDIO_EXE").unwrap());
    let mut disc = DiscImage::open(&PathBuf::from(
        std::env::var_os("BOF3_AUDIO_TRACK").unwrap(),
    ))
    .unwrap();
    for source in [
        "BIN/BMAG_XA/MAGIC00.STR",
        "BIN/SCE_XA/S_XA00.STR",
        "BIN/SCE_XA/VOICE.STR",
        "LOGO/CAPCOM30.STR",
    ] {
        fs::write(
            media.join(Path::new(source).file_name().unwrap()),
            disc.read_xa(source).unwrap().bytes(),
        )
        .unwrap();
    }
    let mut options = Options {
        disc_root: Some(media.clone()),
        archives: vec![],
        executable: Some(exe.clone()),
        output: directory.0.join("streams"),
        id: None,
        kind: Kind::Stream,
        arithmetic: Arithmetic::CombinedRounded,
    };
    let report = extract(&options).unwrap();
    assert_eq!(
        (
            report.sources,
            report.assets,
            report.sectors,
            report.pcm_frames
        ),
        (4, 31, 26599, 84174048)
    );
    super::consumer::validate(&options.output, Some(&media), Some(&exe));
    options.kind = Kind::Cue;
    options.output = directory.0.join("cues");
    let report = extract(&options).unwrap();
    assert_eq!(
        (
            report.sources,
            report.assets,
            report.sectors,
            report.pcm_frames
        ),
        (3, 896, 26115, 83196288)
    );
    super::consumer::validate(&options.output, Some(&media), Some(&exe));
    options.disc_root = None;
    options.archives = vec![
        PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()).join("BIN/SCE_XA/VOICE.STR"),
    ];
    options.kind = Kind::Stream;
    options.output = directory.0.join("truncated");
    assert!(extract(&options)
        .unwrap_err()
        .to_string()
        .contains("known truncated"));
    assert!(!options.output.exists());
}
