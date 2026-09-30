use bof3_audio::bank::extraction::{banks, Options};
use std::{
    fs,
    path::PathBuf,
    process::Command,
    sync::atomic::{AtomicU64, Ordering},
};

struct Directory(PathBuf);

impl Directory {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-extraction-{}-{}",
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

fn archive(invalid: bool) -> Vec<u8> {
    let mut header = vec![0; 0xc20];
    header[..4].copy_from_slice(b"pBAV");
    header[4..8].copy_from_slice(&7u32.to_le_bytes());
    header[0x12..0x18].copy_from_slice(&[1, 0, 2, 0, 1, 0]);
    header[0x18] = 127;
    header[0x19] = 64;
    header[0x50..0x55].copy_from_slice(&[2, 126, 5, 1, 63]); // sparse program 3, layered tones
    for tone in 0..2 {
        let at = 0x820 + tone * 32;
        header[at..at + 14].copy_from_slice(&[4, 1, 125, 65, 60, 7, 12, 105, 1, 2, 3, 4, 2, 2]);
        header[at + 16..at + 24].copy_from_slice(&[0x80, 0x90, 0xa0, 0xb0, 3, 0, 1, 0]);
    }
    header[0xa22..0xa24].copy_from_slice(&4u16.to_le_bytes());
    let mut body = vec![0x11; 32];
    body[0] = if invalid { 0xf0 } else { 0x0c };
    body[1] = 7; // local start, repeat and end; following bytes remain opaque
    let entries = [(6u16, header), (7u16, body), (0x55u16, vec![1, 2, 3, 4, 5])];
    let mut emi = vec![0; 2048];
    emi[..4].copy_from_slice(&3u32.to_le_bytes());
    emi[8..16].copy_from_slice(b"MATH_TBL");
    for (index, (kind, data)) in entries.into_iter().enumerate() {
        let toc = 16 + index * 16;
        emi[toc..toc + 4].copy_from_slice(&(data.len() as u32).to_le_bytes());
        emi[toc + 4..toc + 8].copy_from_slice(&2u32.to_le_bytes());
        emi[toc + 12..toc + 14].copy_from_slice(&kind.to_le_bytes());
        emi.extend(data);
        emi.resize(emi.len().div_ceil(2048) * 2048, 0xa5);
    }
    emi.extend_from_slice(b"opaque archive trailer");
    emi
}

fn options(directory: &Directory, archive: PathBuf) -> Options {
    Options {
        disc_root: None,
        archives: vec![archive],
        executable: PathBuf::from(std::env::var_os("BOF3_AUDIO_EXE").unwrap()),
        output: directory.0.join("export"),
        id: None,
        reference_rate: 44100,
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE original US profile and Rust XML/RIFF consumer"]
fn bank_folder_preserves_archive_layers_loops_and_xml_path_characters() {
    let directory = Directory::new();
    let input = directory.0.join("a&<\"'\t\n.EMI");
    fs::write(&input, archive(false)).unwrap();
    let mut options = options(&directory, input);
    options.reference_rate = 22050;
    let report = banks(&options).unwrap();
    assert_eq!(
        (
            report.banks,
            report.samples,
            report.pcm_frames,
            report.approximate_loops
        ),
        (1, 1, 28, 0)
    );
    super::consumer::validate(&options.output, None);
    let original_manifest = fs::read(options.output.join("audio.xml")).unwrap();
    assert!(banks(&options)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    assert_eq!(
        fs::read(options.output.join("audio.xml")).unwrap(),
        original_manifest
    );
    // The supported CLI takes the same route and reports its explicit context.
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["extract", "--mode", "audio", "--kind", "bank", "--archive"])
        .arg(&options.archives[0])
        .arg("--executable")
        .arg(&options.executable)
        .arg("--output")
        .arg(directory.0.join("cli-export"))
        .arg("--json")
        .output()
        .unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(report["reference_rate"], 44100);
    super::consumer::validate(&directory.0.join("cli-export"), None);
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE original US profile"]
fn invalid_samples_xml_and_ambiguous_ids_do_not_publish_or_leave_staging() {
    let directory = Directory::new();
    let first = directory.0.join("first.EMI");
    let second = directory.0.join("second.EMI");
    fs::write(&first, archive(false)).unwrap();
    fs::write(&second, archive(true)).unwrap();
    let mut options = options(&directory, first.clone());
    options.archives.push(second.clone());
    options.id = Some("0".into());
    assert!(banks(&options)
        .unwrap_err()
        .to_string()
        .contains("ambiguous ID"));
    options.id = None;
    let error = banks(&options).unwrap_err().to_string();
    assert!(
        error.contains("sample 1") && error.contains("predictor"),
        "{error}"
    );
    assert!(!options.output.exists());
    assert_eq!(fs::read_dir(&directory.0).unwrap().count(), 2);
    let invalid_xml = directory.0.join("invalid\u{1}.EMI");
    fs::rename(&first, &invalid_xml).unwrap();
    options.archives = vec![invalid_xml];
    assert!(banks(&options).unwrap_err().to_string().contains("XML 1.0"));
    assert!(!options.output.exists());
    assert_eq!(fs::read_dir(&directory.0).unwrap().count(), 2);
}

#[test]
fn invalid_surface_rate_and_existing_output_fail_before_media_access() {
    let directory = Directory::new();
    let mut options = Options {
        disc_root: None,
        archives: vec![],
        executable: "missing".into(),
        output: directory.0.clone(),
        id: None,
        reference_rate: 48000,
    };
    assert!(banks(&options)
        .unwrap_err()
        .to_string()
        .contains("reference rate"));
    options.reference_rate = 44100;
    assert!(banks(&options)
        .unwrap_err()
        .to_string()
        .contains("already exists"));
    for args in [
        vec!["extract", "--mode", "music", "--kind", "bank"],
        vec!["extract", "--mode", "audio", "--kind", "xa_stream"],
        vec![
            "extract", "--mode", "audio", "--kind", "bank", "--type", "sfx",
        ],
        vec![
            "extract", "--mode", "audio", "--kind", "bank", "--output", "--json",
        ],
    ] {
        let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(args)
            .output()
            .unwrap();
        assert_eq!(result.status.code(), Some(2));
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_CORPUS and Rust XML/RIFF consumer"]
fn all_original_banks_export_exact_archive_snapshots_and_independent_pcm_reference() {
    let directory = Directory::new();
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let options = Options {
        disc_root: Some(root.clone()),
        archives: vec![],
        executable: std::env::var_os("BOF3_AUDIO_EXE").unwrap().into(),
        output: directory.0.join("corpus"),
        id: None,
        reference_rate: 44100,
    };
    let report = banks(&options).unwrap();
    assert_eq!(
        (
            report.banks,
            report.samples,
            report.pcm_frames,
            report.approximate_loops
        ),
        (1020, 8385, 138296704, 790)
    );
    super::consumer::validate(&options.output, Some(&root));
}
