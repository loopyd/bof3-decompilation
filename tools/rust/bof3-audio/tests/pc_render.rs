use bof3_audio::{
    interchange::midi::{Event, Message, Midi, Track},
    interchange::wave::Wave,
    pc_render::{self, Options},
    soundfont::{Bank, Instrument, LoopMode, Preset, Sample, Zone},
};
use std::{fs, process::Command};

fn event(tick: u64, message: Message) -> Event {
    Event { tick, message }
}
fn channel(status: u8, data: &[u8]) -> Message {
    Message::Channel {
        status,
        data: data.to_vec(),
    }
}
fn tempo(value: u32) -> Message {
    Message::Meta {
        kind: 0x51,
        data: value.to_be_bytes()[1..].to_vec(),
    }
}
fn file(extra: Vec<Event>) -> Vec<u8> {
    Midi::new(
        1,
        96,
        vec![
            Track::new(vec![
                event(0, tempo(500000)),
                event(96, tempo(1000000)),
                event(192, Message::end()),
            ]),
            Track::new(extra),
        ],
    )
    .unwrap()
    .to_bytes()
    .unwrap()
}
fn song() -> Vec<u8> {
    file(vec![
        event(0, channel(0x90, &[69, 100])),
        event(96, channel(0x80, &[69, 0])),
        event(96, channel(0xc0, &[5])),
        event(96, channel(0x90, &[69, 100])),
        event(192, channel(0x80, &[69, 0])),
        event(192, Message::end()),
    ])
}
fn bank(layers: usize) -> Vec<u8> {
    let mut zone = Zone::new(0);
    zone.loop_mode = LoopMode::Continuous;
    zone.envelope.release = -3600;
    let mut octave = zone.clone();
    octave.coarse_tune = 12;
    Bank {
        name: "PC render fixture".into(),
        samples: vec![Sample {
            name: "100 Hz".into(),
            pcm: (0..2646)
                .map(|i| (24000.0 * (std::f64::consts::TAU * i as f64 / 441.0).sin()) as i16)
                .collect(),
            rate: 44100,
            root_key: 69,
            correction_cents: 0,
            loop_range: Some(441..2205),
        }],
        instruments: vec![
            Instrument {
                name: "base".into(),
                zones: vec![zone; layers],
            },
            Instrument {
                name: "octave".into(),
                zones: vec![octave],
            },
        ],
        presets: vec![
            Preset {
                name: "base".into(),
                bank: 0,
                program: 0,
                instruments: vec![0],
            },
            Preset {
                name: "octave".into(),
                bank: 0,
                program: 5,
                instruments: vec![1],
            },
        ],
    }
    .encode()
    .unwrap()
    .bytes
}

#[test]
fn tempo_map_order_and_fractional_frames_are_preserved() {
    let schedule = pc_render::schedule(&Midi::from_bytes(&song()).unwrap(), 48000).unwrap();
    assert_eq!(schedule.frames, 72000);
    assert_eq!(
        schedule
            .events
            .iter()
            .map(|e| (e.frame, e.status))
            .collect::<Vec<_>>(),
        [
            (0, 0x90),
            (24000, 0x80),
            (24000, 0xc0),
            (24000, 0x90),
            (72000, 0x80)
        ]
    );
    let file = Midi::new(
        0,
        3,
        vec![Track::new(vec![
            event(1, channel(0xc0, &[0])),
            event(2, channel(0xc0, &[1])),
            event(3, Message::end()),
        ])],
    )
    .unwrap();
    let fractional = pc_render::schedule(&file, 16000).unwrap();
    assert_eq!(
        fractional
            .events
            .iter()
            .map(|e| e.frame)
            .collect::<Vec<_>>(),
        [2666, 5333]
    );
    assert_eq!(fractional.frames, 8000); // No per-delta rounding drift.
    let consumer = rustysynth::MidiFile::new(&mut std::io::Cursor::new(song())).unwrap();
    assert!((consumer.get_length() - 1.5).abs() < 1e-9);
}

#[test]
fn unsupported_events_and_invalid_metadata_fail_before_playback() {
    for message in [
        channel(0xb0, &[99, 20]),
        channel(0xb0, &[6, 127]),
        channel(0xa0, &[69, 64]),
        channel(0xd0, &[64]),
        Message::SysEx {
            status: 0xf0,
            data: vec![0xf7],
        },
        Message::Meta {
            kind: 0x51,
            data: vec![0, 0],
        },
        tempo(0),
        Message::Meta {
            kind: 0x7f,
            data: vec![1],
        },
    ] {
        let bytes = file(vec![event(0, message), event(192, Message::end())]);
        let error = pc_render::render(&bytes, &[], &Options::default())
            .err()
            .unwrap()
            .to_string();
        assert!(error.contains("track 1 event 0"), "{error}");
    }
    let zero = Midi::new(0, 96, vec![Track::new(vec![event(0, Message::end())])]).unwrap();
    assert!(pc_render::schedule(&zero, 44100).is_err());
}

fn frequency(pcm: &[i16], start: usize, end: usize, rate: usize) -> f64 {
    let samples: Vec<_> = (start..end).map(|i| pcm[i * 2]).collect();
    let crossings = samples.windows(2).filter(|p| p[0] <= 0 && p[1] > 0).count();
    crossings as f64 * rate as f64 / (end - start) as f64
}

#[test]
fn render_is_audible_repeats_exactly_and_releases_with_reported_clipping() {
    let rendered = pc_render::render(
        &song(),
        &bank(1),
        &Options {
            repeats: 2,
            ..Options::default()
        },
    )
    .unwrap();
    assert_eq!(rendered.wave.frames(), 220500); // 1.5 s * 2 + 2 s.
    assert_eq!(rendered.report.started_repeats, 2);
    assert_eq!(
        &rendered.wave.pcm[..132300],
        &rendered.wave.pcm[132300..264600]
    );
    assert!((frequency(&rendered.wave.pcm, 4410, 17640, 44100) - 100.0).abs() < 4.0);
    assert!((frequency(&rendered.wave.pcm, 30870, 57330, 44100) - 200.0).abs() < 4.0);
    assert!(rendered.wave.pcm[rendered.wave.pcm.len() - 4410..]
        .iter()
        .all(|v| v.abs() <= 2));
    assert!(rendered.report.peak > 0.01);
    assert_eq!(rendered.report.clipped_samples, 0);
    assert_eq!(
        Wave::from_bytes(&rendered.wave.to_bytes().unwrap()).unwrap(),
        rendered.wave
    );
    let loud = pc_render::render(
        &song(),
        &bank(64),
        &Options {
            repeats: 1,
            duration_frames: Some(10000),
            release_frames: 0,
            ..Options::default()
        },
    )
    .unwrap();
    assert!(loud.report.peak > 1.0);
    assert!(loud.report.clipped_samples > 0);
    assert!(loud.wave.pcm.contains(&i16::MAX));
}

#[test]
fn missing_presets_and_unrepresentable_durations_are_rejected() {
    for events in [
        vec![
            event(0, channel(0xc0, &[3])),
            event(0, channel(0x90, &[69, 100])),
        ],
        vec![event(0, channel(0x99, &[69, 100]))],
        vec![
            event(0, channel(0xb0, &[0, 1])),
            event(0, channel(0x90, &[69, 100])),
        ],
    ] {
        let mut events = events;
        events.push(event(192, Message::end()));
        let error = pc_render::render(&file(events), &bank(1), &Options::default())
            .err()
            .unwrap()
            .to_string();
        assert!(error.contains("timbre substitution rejected"), "{error}");
    }
    for options in [
        Options {
            repeats: 0,
            ..Options::default()
        },
        Options {
            duration_frames: Some(0),
            ..Options::default()
        },
        Options {
            duration_frames: Some(u64::MAX),
            ..Options::default()
        },
        Options {
            release_frames: u64::MAX,
            ..Options::default()
        },
        Options {
            safety_frames: 1,
            ..Options::default()
        },
    ] {
        assert!(pc_render::render(&song(), &bank(1), &options).is_err());
    }
}

#[test]
fn default_plays_once_and_long_duration_preserves_final_state_without_restart() {
    let once = pc_render::render(&song(), &bank(1), &Options::default()).unwrap();
    assert_eq!(once.report.started_repeats, 1);
    assert_eq!(once.report.body_frames, 66150);
    assert_eq!(once.wave.frames(), 154350);
    let extended = pc_render::render(
        &song(),
        &bank(1),
        &Options {
            duration_frames: Some(132300),
            release_frames: 0,
            ..Options::default()
        },
    )
    .unwrap();
    assert_eq!(extended.report.started_repeats, 1);
    assert_eq!(extended.wave.frames(), 132300);
    assert_eq!(&extended.wave.pcm[..132300], &once.wave.pcm[..132300]);
    assert_eq!(extended.wave.pcm, once.wave.pcm[..extended.wave.pcm.len()]);
    assert_eq!(extended.report.post_sequence_frames, 66150);
    assert!(!extended.report.duration_cutoff);
}

#[test]
fn live_pan_and_pitch_bend_change_the_audible_output() {
    let midi = file(vec![
        event(0, channel(0xb0, &[10, 0])),
        event(0, channel(0x90, &[69, 100])),
        event(96, channel(0xb0, &[10, 127])),
        event(96, channel(0xe0, &[0, 96])), // Half of the default +2-semitone range.
        event(192, channel(0x80, &[69, 0])),
        event(192, Message::end()),
    ]);
    let rendered = pc_render::render(
        &midi,
        &bank(1),
        &Options {
            repeats: 1,
            release_frames: 0,
            ..Options::default()
        },
    )
    .unwrap();
    let energy = |start: usize, end: usize, channel: usize| -> f64 {
        (start..end)
            .map(|i| f64::from(rendered.wave.pcm[i * 2 + channel]).powi(2))
            .sum()
    };
    assert!(energy(4410, 17640, 0) > energy(4410, 17640, 1) * 10.0);
    assert!(energy(30870, 57330, 1) > energy(30870, 57330, 0) * 10.0);
    let right: Vec<i16> = rendered
        .wave
        .pcm
        .as_chunks::<2>()
        .0
        .iter()
        .flat_map(|v| [v[1], v[1]])
        .collect();
    assert!((frequency(&right, 30870, 57330, 44100) - 100.0 * 2f64.powf(1.0 / 12.0)).abs() < 3.0);
}

#[test]
fn cli_publishes_verified_wav_and_report_and_preserves_existing_output() {
    let root = std::env::temp_dir().join(format!("bof3-pc-render-{}", std::process::id()));
    fs::create_dir(&root).unwrap();
    let midi = root.join("song.mid");
    let sf2 = root.join("bank.sf2");
    let output = root.join("rendered");
    fs::write(&midi, song()).unwrap();
    fs::write(&sf2, bank(1)).unwrap();
    let command = || {
        let mut command = Command::new(env!("CARGO_BIN_EXE_bof3-audio"));
        command
            .args(["render", "--mode", "music", "--engine", "pc", "--midi"])
            .arg(&midi)
            .arg("--soundfont")
            .arg(&sf2)
            .arg("--output")
            .arg(&output)
            .args([
                "--duration",
                "0.25",
                "--tail",
                "0.125",
                "--sample-rate",
                "16000",
                "--json",
            ]);
        command
    };
    let result = command().output().unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&result.stdout).unwrap();
    assert_eq!(report["output_frames"], 6000);
    assert_eq!(report["duration_cutoff"], true);
    let wave = fs::read(output.join("render.wav")).unwrap();
    assert_eq!(Wave::from_bytes(&wave).unwrap().frames(), 6000);
    assert_eq!(
        serde_json::from_slice::<serde_json::Value>(&fs::read(output.join("render.json")).unwrap())
            .unwrap(),
        report
    );
    assert!(!command().output().unwrap().status.success());
    assert_eq!(fs::read(output.join("render.wav")).unwrap(), wave);
    fs::remove_dir_all(&output).unwrap();
    assert!(!command()
        .args(["--timeout", "0.1"])
        .output()
        .unwrap()
        .status
        .success());
    for (flag, replacement) in [("--tail", "NaN"), ("--engine", "psx")] {
        let mut args: Vec<_> = command().get_args().map(|a| a.to_owned()).collect();
        let index = args.iter().position(|a| a == flag).unwrap();
        args[index + 1] = replacement.into();
        assert!(!Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
            .args(args)
            .output()
            .unwrap()
            .status
            .success());
        assert!(!output.exists());
    }
    fs::write(
        &midi,
        file(vec![
            event(0, channel(0xb0, &[99, 20])),
            event(192, Message::end()),
        ]),
    )
    .unwrap();
    assert!(!command().output().unwrap().status.success());
    assert_eq!(fs::read_dir(&root).unwrap().count(), 2); // No published or staging output.
    fs::remove_dir_all(root).unwrap();
}

#[test]
#[ignore = "requires installed FFmpeg for independent WAV decoding; never a production renderer"]
fn rendered_wav_decodes_to_identical_pcm_in_ffmpeg() {
    use std::io::Write;
    use std::process::Stdio;
    let rendered = pc_render::render(
        &song(),
        &bank(1),
        &Options {
            repeats: 1,
            duration_frames: Some(11025),
            release_frames: 0,
            ..Options::default()
        },
    )
    .unwrap();
    let bytes = rendered.wave.to_bytes().unwrap();
    let mut child = Command::new("ffmpeg")
        .args([
            "-v",
            "error",
            "-i",
            "pipe:0",
            "-f",
            "s16le",
            "-acodec",
            "pcm_s16le",
            "pipe:1",
        ])
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .unwrap();
    let mut stdin = child.stdin.take().unwrap();
    let writer = std::thread::spawn(move || stdin.write_all(&bytes).unwrap());
    let output = child.wait_with_output().unwrap();
    writer.join().unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    let expected: Vec<u8> = rendered
        .wave
        .pcm
        .iter()
        .flat_map(|v| v.to_le_bytes())
        .collect();
    assert_eq!(output.stdout, expected);
}
