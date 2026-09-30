use bof3_audio::{
    interchange::wave::Wave,
    psx_render::{Body, Options},
};
use std::{fs, process::Command};

#[test]
fn timing_and_selection_limits_fail_before_runtime_execution() {
    let valid = Options {
        sequence: 0,
        layout: None,
        body: Body::Duration(44100),
        release_frames: 88200,
        safety_frames: 44100 * 600,
    };
    valid.validate().unwrap();
    for invalid in [
        Options {
            sequence: 4,
            ..valid.clone()
        },
        Options {
            layout: Some(3),
            ..valid.clone()
        },
        Options {
            body: Body::Duration(0),
            ..valid.clone()
        },
        Options {
            body: Body::Duration(u64::MAX),
            ..valid.clone()
        },
        Options {
            safety_frames: 44100,
            ..valid.clone()
        },
        Options {
            safety_frames: 44100 * 601,
            ..valid.clone()
        },
    ] {
        assert!(invalid.validate().is_err());
    }
}

#[test]
fn cli_rejects_incomplete_or_mixed_engine_requests_without_publication() {
    let output = std::env::temp_dir().join(format!("bof3-psx-invalid-{}", std::process::id()));
    assert!(!output.exists());
    for (extra, diagnostic) in [
        (vec!["--sequence", "0", "--loops", "0"], "must be positive"),
        (
            vec!["--sequence", "0", "--loops", "1", "--duration", "1"],
            "mutually exclusive",
        ),
        (vec!["--duration", "1"], "requires --sequence"),
        (
            vec![
                "--sequence",
                "0",
                "--duration",
                "1",
                "--sample-rate",
                "48000",
            ],
            "44100 Hz",
        ),
        (
            vec!["--sequence", "0", "--duration", "1", "--repeats", "2"],
            "PC options",
        ),
        (
            vec!["--sequence", "0", "--duration", "1", "--timeout", "0.5"],
            "safety limit",
        ),
        (vec!["--sequence", "4", "--duration", "1"], "sequence must"),
    ] {
        let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(["render", "--mode", "music", "--engine", "psx", "--output"])
            .arg(&output)
            .args(extra)
            .output()
            .unwrap();
        assert_eq!(result.status.code(), Some(2));
        assert!(
            String::from_utf8_lossy(&result.stderr).contains(diagnostic),
            "{}",
            String::from_utf8_lossy(&result.stderr)
        );
        assert!(!output.exists());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS, BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn original_runtime_cli_publishes_exact_duration_tail_and_qualified_evidence() {
    let root = std::env::temp_dir().join(format!("bof3-psx-render-{}", std::process::id()));
    fs::create_dir(&root).unwrap();
    let output = root.join("rendered");
    let archive = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap())
        .join("BIN/BGM/BGM000.EMI");
    let command = || {
        let mut c = Command::new(env!("CARGO_BIN_EXE_bof3-audio"));
        c.args(["render", "--mode", "music", "--engine", "psx", "--archive"])
            .arg(&archive)
            .arg("--executable")
            .arg(std::env::var_os("BOF3_AUDIO_EXE").unwrap())
            .arg("--bios")
            .arg(std::env::var_os("BOF3_AUDIO_BIOS").unwrap())
            .args([
                "--sequence",
                "0",
                "--duration",
                "2",
                "--tail",
                "2",
                "--json",
                "--output",
            ])
            .arg(&output);
        c
    };
    let result = command().output().unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(report["schema"], "bof3.psx-render/v1");
    assert_eq!(
        report["identity"]["archive_sha256"],
        "e1caf8633ce6be70524bd288b5af1c4b6042e4ca5cb3b02600affd68b8013a91"
    );
    assert_eq!(report["identity"]["layout"], 0);
    assert!(report["identity"]["requested_layout"].is_null());
    assert_eq!(report["identity"]["sequence_entry"], 1);
    assert_eq!(report["identity"]["sequence_index"], 0);
    assert_eq!(report["body_frames"], 88200);
    assert_eq!(report["release_frames"], 88200);
    assert_eq!(report["output_frames"], 176400);
    assert_eq!(report["independent_pcm_validated"], false);
    let original = fs::read(output.join("render.wav")).unwrap();
    let wave = Wave::from_bytes(&original).unwrap();
    assert_eq!(wave.frames(), 176400);
    assert_eq!(wave.sample_rate, 44100);
    assert_eq!(wave.channels, 2);
    assert!(wave.pcm[..88200 * 2].iter().any(|&s| s != 0));
    let energy = |samples: &[i16]| {
        samples
            .iter()
            .map(|&s| i64::from(s).pow(2) as u64)
            .sum::<u64>()
    };
    assert!(energy(&wave.pcm[88200 * 2..92610 * 2]) > energy(&wave.pcm[171990 * 2..]));
    let persisted: serde_json::Value =
        serde_json::from_slice(&fs::read(output.join("render.json")).unwrap()).unwrap();
    assert_eq!(report, persisted);
    assert!(!command().output().unwrap().status.success());
    assert_eq!(fs::read(output.join("render.wav")).unwrap(), original);
    fs::remove_dir_all(root).unwrap();
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn ambiguous_entries_and_manifest_body_conflicts_fail_before_bios_boot() {
    use bof3_audio::machine::{
        executable::Executable,
        firmware::{Image, ROM_BYTES},
        music,
    };
    use emi_ex_v2::image::ArchiveImage;
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let path = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap())
        .join("BIN/BGM/BGM000.EMI");
    let bytes = fs::read(path).unwrap();
    let archive = ArchiveImage::from_bytes(bytes.clone()).unwrap();
    let error = |archive: &ArchiveImage, sequence| {
        music::prepare(
            &exe,
            Image::from_bytes(vec![0; ROM_BYTES]).unwrap(),
            archive,
            None,
            sequence,
        )
        .err()
        .unwrap()
        .to_string()
    };
    assert!(error(&archive, 4).contains("has no sequence index"));
    let mut duplicate = bytes;
    duplicate[0x3c..0x3e].copy_from_slice(&6u16.to_le_bytes());
    assert!(
        error(&ArchiveImage::from_bytes(duplicate).unwrap(), 0).contains("found entries [0, 2]")
    );
    let body = archive.entry(2).unwrap();
    let shortened = archive
        .replace_entries(&[(2, &body[..body.len() - 16])])
        .unwrap();
    assert!(error(&shortened, 0).contains("VH describes"));
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS, BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn automatic_layout_fits_large_archives_without_cue_or_filename_rules() {
    use bof3_audio::machine::{executable::Executable, firmware::Image, music};
    use emi_ex_v2::image::ArchiveImage;
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let firmware = fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    let corpus = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    // Preparation accepts archive bytes, never a path, song name or cue ID.
    for name in ["BGMEND.EMI", "BGMOPN.EMI"] {
        let archive =
            ArchiveImage::from_bytes(fs::read(corpus.join("BIN/BGM").join(name)).unwrap()).unwrap();
        let prepare = |layout| {
            music::prepare(
                &exe,
                Image::from_bytes(firmware.clone()).unwrap(),
                &archive,
                layout,
                0,
            )
        };
        let automatic = prepare(None).unwrap();
        assert_eq!(automatic.identity.layout, 2);
        assert_eq!(automatic.identity.requested_layout, None);
        let explicit = prepare(Some(2)).unwrap();
        assert_eq!(explicit.identity.layout, 2);
        assert_eq!(explicit.identity.requested_layout, Some(2));
        assert_eq!(
            automatic.identity.slot.spu_base,
            explicit.identity.slot.spu_base
        );
        assert_eq!(automatic.identity.dma_bytes, explicit.identity.dma_bytes);
        for layout in [0, 1] {
            let error = prepare(Some(layout)).err().unwrap().to_string();
            assert!(
                error.contains(&format!("layout {layout} slot 0 capacity")),
                "{error}"
            );
        }
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS, BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn original_runtime_loop_limits_preserve_finite_counts_end_markers_and_pcm_prefix() {
    use bof3_audio::{
        machine::music_progress::Stop,
        machine::{executable::Executable, firmware::Image},
        psx_render,
    };
    use emi_ex_v2::image::ArchiveImage;
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let bios = fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    let corpus = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let archive =
        ArchiveImage::from_bytes(fs::read(corpus.join("BIN/BGM/BGM000.EMI")).unwrap()).unwrap();
    let sequence = |count: u8, delta: u8| {
        // Explicit status at the note and end events; loop target has B0 running status.
        let events = [
            0, 0xc0, 1, 0, 0xb0, 99, 20, 0, 98, count, 0, 0x90, 60, 100, delta, 0x90, 60, 0, 0,
            0xb0, 99, 30, 0, 0xff, 0x2f,
        ];
        let mut sep = archive.entry(1).unwrap()[..19].to_vec();
        sep[15..19].copy_from_slice(&(events.len() as u32).to_be_bytes());
        sep.extend(events);
        sep.resize(archive.entry(1).unwrap().len(), 0);
        archive.replace_entries(&[(1, sep.as_slice())]).unwrap()
    };
    let options = Options {
        sequence: 0,
        layout: None,
        body: Body::Loops(1),
        release_frames: 4410,
        safety_frames: 44100 * 10,
    };
    let render = |archive: &ArchiveImage, options: &Options| {
        psx_render::render(
            &exe,
            Image::from_bytes(bios.clone()).unwrap(),
            archive,
            options,
        )
    };
    let infinite = sequence(127, 48);
    let one = render(&infinite, &options).unwrap();
    let two = render(
        &infinite,
        &Options {
            body: Body::Loops(2),
            ..options.clone()
        },
    )
    .unwrap();
    assert_eq!(one.report.progress.total_infinite_traversals, 1);
    assert_eq!(two.report.progress.total_infinite_traversals, 2);
    assert_eq!(two.report.progress.loop_starts, 1);
    assert_eq!(two.report.progress.end_markers, 0);
    assert_eq!(
        two.report.progress.boundary.as_ref().unwrap().reason,
        Stop::InfiniteLoopLimit
    );
    assert!(two.report.body_frames > one.report.body_frames);
    let boundary = one.report.body_frames as usize * 2;
    assert_eq!(&one.wave.pcm[..boundary], &two.wave.pcm[..boundary]);
    assert!(one.wave.pcm[..boundary].iter().any(|&s| s != 0));
    assert_eq!(
        two.wave.frames() as u64,
        two.report.body_frames + options.release_frames
    );
    assert!(two.report.stop_call_started_at_frame >= two.report.body_frames);
    let root = std::env::temp_dir().join(format!("bof3-loop-default-{}", std::process::id()));
    fs::create_dir(&root).unwrap();
    let input = root.join("anonymous.emi");
    fs::write(&input, infinite.bytes()).unwrap();
    let output = root.join("render");
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["render", "--mode", "music", "--engine", "psx", "--archive"])
        .arg(&input)
        .arg("--executable")
        .arg(std::env::var_os("BOF3_AUDIO_EXE").unwrap())
        .arg("--bios")
        .arg(std::env::var_os("BOF3_AUDIO_BIOS").unwrap())
        .args(["--sequence", "0", "--tail", "0.1", "--json", "--output"])
        .arg(&output)
        .output()
        .unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(
        report["requested_body"],
        serde_json::json!({"kind":"loops", "value":2})
    );
    assert_eq!(report["progress"]["infinite_traversals"], 2);
    assert_eq!(
        Wave::from_bytes(&fs::read(output.join("render.wav")).unwrap()).unwrap(),
        two.wave
    );
    let opened = two
        .report
        .initialization_calls
        .iter()
        .find(|call| call.entry == 0x8016b38c)
        .unwrap();
    assert_eq!(opened.arguments[2], 1);
    fs::remove_dir_all(root).unwrap();
    let finite = sequence(2, 48);
    let finite_one = render(&finite, &options).unwrap();
    let finite_three = render(
        &finite,
        &Options {
            body: Body::Loops(3),
            ..options.clone()
        },
    )
    .unwrap();
    assert_eq!(finite_one.wave, finite_three.wave);
    assert_eq!(finite_one.report.progress.total_infinite_traversals, 0);
    assert_eq!(finite_one.report.progress.end_markers, 1);
    assert_eq!(
        finite_one.report.progress.boundary.as_ref().unwrap().reason,
        Stop::EndMarker
    );
    // A callback that cannot return must fail, even after its observer sees the loop limit.
    let error = render(&sequence(127, 0), &options)
        .err()
        .unwrap()
        .to_string();
    assert!(error.contains("instruction"), "{error}");
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS, BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn default_loop_limit_times_out_without_publishing_a_truncated_song() {
    let output = std::env::temp_dir().join(format!("bof3-loop-timeout-{}", std::process::id()));
    assert!(!output.exists());
    let corpus = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["render", "--mode", "music", "--engine", "psx", "--archive"])
        .arg(corpus.join("BIN/BGM/BGM000.EMI"))
        .arg("--executable")
        .arg(std::env::var_os("BOF3_AUDIO_EXE").unwrap())
        .arg("--bios")
        .arg(std::env::var_os("BOF3_AUDIO_BIOS").unwrap())
        .args([
            "--sequence",
            "0",
            "--timeout",
            "0.2",
            "--tail",
            "0.05",
            "--output",
        ])
        .arg(&output)
        .output()
        .unwrap();
    assert_eq!(result.status.code(), Some(2));
    let error = String::from_utf8_lossy(&result.stderr);
    assert!(
        error.contains("audio safety limit reached before requested loop/end boundary"),
        "{error}"
    );
    assert!(!output.exists());
}
