#[path = "support/music/mod.rs"]
mod fixture;
use bof3_audio::{
    interchange::wave::Wave, machine::executable::Executable, pc_archive, pc_archive::Options,
    pc_render, sequence::SequenceSet,
};
use emi_ex_v2::image::ArchiveImage;
use std::{
    fs,
    path::PathBuf,
    process::Command,
    sync::atomic::{AtomicU64, Ordering},
};

fn options(sequence: usize) -> Options {
    Options {
        sequence,
        loops: None,
        allow_approximations: true,
        playback: pc_render::Options {
            release_frames: 0,
            ..Default::default()
        },
    }
}
fn executable() -> Executable {
    Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap()).unwrap()
}
struct Directory(PathBuf);
impl Directory {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let p = std::env::temp_dir().join(format!(
            "bof3-pc-archive-{}-{}",
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

#[test]
fn invalid_controls_fail_before_media_reads_or_publication() {
    let d = Directory::new();
    for (args, reason) in [
        (vec![], "--allow-approximations"),
        (vec!["--allow-approximations", "--loops", "0"], "positive"),
        (
            vec!["--allow-approximations", "--loops", "2", "--duration", "1"],
            "mutually exclusive",
        ),
        (
            vec!["--allow-approximations", "--repeats", "2"],
            "separate input",
        ),
        (
            vec![
                "--allow-approximations",
                "--duration",
                "3",
                "--timeout",
                "1",
            ],
            "safety limit",
        ),
        (
            vec!["--allow-approximations", "--bios", "absent"],
            "PSX-only",
        ),
    ] {
        let r = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args([
                "render",
                "--mode",
                "music",
                "--engine",
                "pc",
                "--archive",
                "absent",
                "--sequence",
                "0",
                "--executable",
                "absent",
                "--output",
            ])
            .arg(d.0.join("output"))
            .args(args)
            .output()
            .unwrap();
        assert!(!r.status.success());
        assert!(
            String::from_utf8_lossy(&r.stderr).contains(reason),
            "{}",
            String::from_utf8_lossy(&r.stderr)
        );
        assert!(!d.0.join("output").exists());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE"]
fn archive_defaults_match_export_once_and_sequences_are_independent() {
    let exe = executable();
    let d = Directory::new();
    let image = ArchiveImage::from_bytes(fixture::synthetic_archive(false)).unwrap();
    let archive = d.0.join("source.emi");
    fs::write(&archive, image.bytes()).unwrap();
    let exported = bof3_audio::music::extraction::songs(&bof3_audio::music::extraction::Options {
        disc_root: None,
        archives: vec![archive.clone()],
        executable: std::env::var_os("BOF3_AUDIO_EXE").unwrap().into(),
        output: d.0.join("export"),
        id: None,
        loops: 2,
        allow_approximations: true,
    })
    .unwrap();
    let folder =
        d.0.join("export")
            .join(&exported.songs[0].path)
            .parent()
            .unwrap()
            .to_path_buf();
    for sequence in 0..2 {
        let rendered =
            pc_archive::render(&exe, &archive.to_string_lossy(), &image, &options(sequence))
                .unwrap();
        let direct = pc_render::render(
            &fs::read(folder.join(format!("sequence-{sequence:03}.mid"))).unwrap(),
            &fs::read(folder.join("bank.sf2")).unwrap(),
            &options(sequence).playback,
        )
        .unwrap();
        assert_eq!(rendered.wave, direct.wave);
        assert_eq!(rendered.report.playback.started_repeats, 1);
        assert_eq!(rendered.report.song.sequences.len(), 1);
        assert_eq!(rendered.report.song.sequences[0].sequence_index, sequence);
        assert_eq!(
            rendered.report.playback.midi_sha256,
            exported.songs[0].sequences[sequence].sha256
        );
    }
    let bad = ArchiveImage::from_bytes(fixture::synthetic_archive(true)).unwrap();
    assert!(pc_archive::render(&exe, "bad.emi", &bad, &options(0)).is_ok());
    let error = pc_archive::render(&exe, "bad.emi", &bad, &options(1))
        .err()
        .unwrap()
        .to_string();
    assert!(error.contains("unsupported controller"), "{error}");
    assert!(pc_archive::render(&exe, "source.emi", &image, &options(2)).is_err());
}

fn with_intro(image: &ArchiveImage) -> ArchiveImage {
    let mut sep = image.entry(2).unwrap().to_vec();
    let set = SequenceSet::parse(&sep).unwrap();
    let seq = &set.sequences[1];
    // Loop-start occurs after a 24-tick intro; the saved cursor still starts
    // the same 12-tick body on subsequent traversals.
    sep[seq.data_offset] = 24;
    image.replace_entries(&[(2, &sep)]).unwrap()
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE"]
fn loops_keep_intro_once_and_fixed_duration_continues_without_file_restarts() {
    let exe = executable();
    let image = with_intro(&ArchiveImage::from_bytes(fixture::synthetic_archive(false)).unwrap());
    let mut outputs = Vec::new();
    for loops in [1, 2, 3] {
        let mut opts = options(1);
        opts.loops = Some(loops);
        let result = pc_archive::render(&exe, "intro.emi", &image, &opts).unwrap();
        let ticks = 24 + 12 * u64::from(loops);
        assert_eq!(result.report.playback.body_frames, ticks * 44100 / 192);
        assert_eq!(result.report.playback.started_repeats, 1);
        outputs.push(result.wave);
    }
    for pair in outputs.windows(2) {
        assert_eq!(pair[0].pcm, pair[1].pcm[..pair[0].pcm.len()]);
    }
    let mut fixed = options(1);
    fixed.playback.duration_frames = Some(20000);
    let rendered = pc_archive::render(&exe, "intro.emi", &image, &fixed).unwrap();
    assert_eq!(rendered.wave.frames(), 20000);
    assert!(rendered.report.expanded_loop_limit > 2);
    assert_eq!(rendered.report.playback.started_repeats, 1);
    assert!(rendered.report.playback.duration_cutoff);
    let mut ended = options(0);
    ended.playback.duration_frames = Some(44100);
    let result = pc_archive::render(&exe, "intro.emi", &image, &ended).unwrap();
    assert_eq!(result.wave.frames(), 44100);
    assert!(result.report.playback.post_sequence_frames > 0);
    assert_eq!(result.report.playback.started_repeats, 1);
    let mut finite_sep = image.entry(2).unwrap().to_vec();
    let offset = SequenceSet::parse(&finite_sep).unwrap().sequences[1].data_offset;
    assert_eq!(&finite_sep[offset + 5..offset + 7], &[98, 127]);
    finite_sep[offset + 6] = 2;
    let finite = image.replace_entries(&[(2, &finite_sep)]).unwrap();
    let mut one = options(1);
    one.loops = Some(1);
    let mut three = options(1);
    three.loops = Some(3);
    let first = pc_archive::render(&exe, "finite.emi", &finite, &one).unwrap();
    let last = pc_archive::render(&exe, "finite.emi", &finite, &three).unwrap();
    assert_eq!(first.wave, last.wave);
    assert_eq!(
        first.report.song.sequences[0].translation.stop,
        bof3_audio::sequence::timeline::Stop::EndMarker
    );
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE"]
fn cli_publishes_verified_wave_and_preserves_existing_output() {
    let d = Directory::new();
    let archive = d.0.join("source.emi");
    fs::write(&archive, fixture::synthetic_archive(false)).unwrap();
    let output = d.0.join("rendered");
    let run = || {
        Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(["render", "--mode", "music", "--engine", "pc", "--archive"])
            .arg(&archive)
            .arg("--executable")
            .arg(std::env::var_os("BOF3_AUDIO_EXE").unwrap())
            .args([
                "--sequence",
                "1",
                "--allow-approximations",
                "--loops",
                "3",
                "--tail",
                "0.125",
                "--sample-rate",
                "16000",
                "--json",
                "--output",
            ])
            .arg(&output)
            .output()
            .unwrap()
    };
    let first = run();
    assert!(
        first.status.success(),
        "{}",
        String::from_utf8_lossy(&first.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&first.stdout).unwrap();
    assert_eq!(report["expanded_loop_limit"], 3);
    assert_eq!(report["playback"]["started_repeats"], 1);
    let bytes = fs::read(output.join("render.wav")).unwrap();
    let wave = Wave::from_bytes(&bytes).unwrap();
    assert_eq!(wave.sample_rate, 16000);
    assert_eq!(wave.frames(), 5000);
    assert!(!run().status.success());
    assert_eq!(fs::read(output.join("render.wav")).unwrap(), bytes);
    assert_eq!(fs::read_dir(&d.0).unwrap().count(), 2);
}
