use bof3_audio::{
    archive::MediaImage,
    interchange::midi::{Message, Midi},
    pc_render,
    sequence::midi as translation,
    sequence::timeline::{Limits, Stop},
    sequence::{Sequence, SequenceSet},
};
use std::{io::Cursor, path::Path};

#[test]
fn initial_programs_follow_channels_and_generated_setup_edits_are_rejected() {
    use bof3_audio::sequence::editing;
    let mut bytes = Vec::new();
    for channel in 0..16 {
        bytes.extend([0, 0x90 | channel, 60, 100]);
    }
    bytes.extend([96, 0xff, 0x2f]);
    let sequence = header(&bytes);
    let mut translated = translation::translate(&sequence, &bytes, &Limits::default()).unwrap();
    translation::initialize_channels(&mut translated).unwrap();
    let programs: Vec<_> = translated.midi.tracks[1]
        .events
        .iter()
        .filter_map(|event| match &event.message {
            Message::Channel { status, data } if status & 0xf0 == 0xc0 => {
                Some((status & 15, data[0]))
            }
            _ => None,
        })
        .collect();
    assert_eq!(
        programs,
        (0..16)
            .map(|channel| (channel, channel))
            .collect::<Vec<_>>()
    );
    let rebuilt =
        editing::rebuild(&sequence, &bytes, &translated.midi, &Default::default()).unwrap();
    assert_eq!(rebuilt.bytes, bytes);
    let event = translated.midi.tracks[1]
        .events
        .iter_mut()
        .find(|event| matches!(&event.message, Message::Channel { status: 0xc1, .. }))
        .unwrap();
    if let Message::Channel { data, .. } = &mut event.message {
        data[0] = 0;
    }
    assert!(editing::rebuild(&sequence, &bytes, &translated.midi, &Default::default()).is_err());
}

fn header(bytes: &[u8]) -> Sequence {
    Sequence {
        sequence_index: 2,
        sequence_id: 17,
        resolution: 96,
        tempo_us: 500000,
        time_numerator: 4,
        time_denominator_power: 2,
        data_offset: 19,
        data_bytes: bytes.len(),
    }
}

#[test]
fn conductor_performance_and_source_mapping_retain_order_and_normalize_bend() {
    let bytes = [
        0, 0xc3, 4, 0, 0x93, 60, 100, 3, 0xe3, 17, 65, 0, 0xb3, 7, 90, 0, 0xb3, 10, 30, 4, 0xff,
        0x51, 6, 0x1a, 0x80, 5, 0x93, 60, 0, 0, 0xff, 0x2f, 0,
    ];
    let translation = translation::translate(&header(&bytes), &bytes, &Limits::default()).unwrap();
    assert_eq!(translation.midi.format, 1);
    assert_eq!(translation.midi.tracks.len(), 2);
    assert_eq!(translation.report.sequence_index, 2);
    assert_eq!(translation.report.sequence_id, 17);
    assert_eq!(translation.report.final_tick, 12);
    assert_eq!(translation.report.ignored_bend_low_bytes, 1);
    assert_eq!(translation.report.tempos.len(), 2);
    assert_eq!(translation.report.tempos[1].linked_integer_bpm, 150);
    let performance = &translation.midi.tracks[1].events;
    assert_eq!(
        performance[0].message,
        Message::Channel {
            status: 0xc3,
            data: vec![4]
        }
    );
    assert_eq!(performance[2].tick, 3);
    assert_eq!(
        performance[2].message,
        Message::Channel {
            status: 0xe3,
            data: vec![0, 65]
        }
    );
    assert_eq!(
        performance[7].message,
        Message::Channel {
            status: 0xb3,
            data: vec![123, 0]
        }
    );
    for mapping in &translation.report.mappings {
        let step = &translation.timeline.steps[mapping.step];
        assert_eq!(step.source_cursor, mapping.source_cursor);
        assert_eq!(
            translation.midi.tracks[mapping.track].events[mapping.event].tick,
            step.tick
        );
    }
    let encoded = translation.midi.to_bytes().unwrap();
    let parsed = Midi::from_bytes(&encoded).unwrap();
    assert_eq!(parsed.to_bytes().unwrap(), encoded);
    let scheduled = pc_render::schedule(&parsed, 44100).unwrap();
    assert_eq!(scheduled.frames, 2526);
    let independent = rustysynth::MidiFile::new(&mut Cursor::new(encoded)).unwrap();
    assert!((independent.get_length() - 5.5 / 96.0).abs() < 1e-6);
}

#[test]
fn loop_expansion_is_finite_and_never_sends_game_nrpn_to_standard_synthesis() {
    for count in [2, 127] {
        let bytes = [
            0, 0xb0, 99, 20, 0, 98, count, 0, 0xc0, 1, 0, 0x90, 60, 100, 12, 0x90, 60, 0, 0, 0xb0,
            99, 30, 7, 0xff, 0x2f, 0,
        ];
        let translated =
            translation::translate(&header(&bytes), &bytes, &Limits::default()).unwrap();
        assert_eq!(
            translated.report.final_tick,
            if count == 127 { 24 } else { 38 }
        );
        assert_eq!(
            translated
                .timeline
                .steps
                .iter()
                .filter(|s| matches!(s.kind, bof3_audio::sequence::events::Kind::Program { .. }))
                .count(),
            2
        );
        let scheduled = pc_render::schedule(&translated.midi, 44100).unwrap();
        assert!(scheduled
            .events
            .iter()
            .filter(|e| e.status & 0xf0 == 0xb0)
            .all(|e| e.data[0] == 123));
        assert_eq!(
            scheduled.events.iter().filter(|e| e.status == 0x90).count(),
            4
        );
        assert!(translated.midi.tracks[1]
            .events
            .iter()
            .any(|e| matches!(&e.message, Message::Meta { kind: 6, .. })));
    }
}

#[test]
fn header_limits_and_unsupported_events_cannot_produce_a_partial_translation() {
    let bytes = [0, 0xff, 0x2f, 0];
    for (resolution, tempo, numerator, power, length) in [
        (0, 500000, 4, 2, 4),
        (32768, 500000, 4, 2, 4),
        (96, 0, 4, 2, 4),
        (96, 0x1000000, 4, 2, 4),
        (96, 500000, 0, 2, 4),
        (96, 500000, 4, 8, 4),
        (96, 500000, 4, 2, 3),
    ] {
        let mut h = header(&bytes);
        h.resolution = resolution;
        h.tempo_us = tempo;
        h.time_numerator = numerator;
        h.time_denominator_power = power;
        h.data_bytes = length;
        assert!(translation::translate(&h, &bytes, &Limits::default()).is_err());
    }
    let unsupported = [0, 0xb0, 64, 127, 0, 0xff, 0x2f];
    assert!(
        translation::translate(&header(&unsupported), &unsupported, &Limits::default()).is_err()
    );
}

#[test]
fn expanded_sequence_plays_through_the_pc_renderer_with_a_linked_synthetic_bank() {
    use bof3_audio::soundfont::{Bank, Instrument, LoopMode, Preset, Sample, Zone};
    let bytes = [
        0, 0xb0, 99, 20, 0, 98, 127, 0, 0xc0, 0, 0, 0x90, 69, 100, 96, 0x90, 69, 0, 0, 0xb0, 99,
        30, 0, 0xff, 0x2f, 0,
    ];
    let translated = translation::translate(&header(&bytes), &bytes, &Limits::default()).unwrap();
    let mut zone = Zone::new(0);
    zone.loop_mode = LoopMode::Continuous;
    zone.envelope.release = -3600;
    let sf2 = Bank {
        name: "SEP playback test".into(),
        samples: vec![Sample {
            name: "100 Hz".into(),
            pcm: (0..960)
                .map(|i| (12000.0 * (std::f64::consts::TAU * i as f64 / 160.0).sin()) as i16)
                .collect(),
            rate: 16000,
            root_key: 69,
            correction_cents: 0,
            loop_range: Some(160..800),
        }],
        instruments: vec![Instrument {
            name: "tone".into(),
            zones: vec![zone],
        }],
        presets: vec![Preset {
            name: "program 0".into(),
            bank: 0,
            program: 0,
            instruments: vec![0],
        }],
    }
    .encode()
    .unwrap()
    .bytes;
    let rendered = pc_render::render(
        &translated.midi.to_bytes().unwrap(),
        &sf2,
        &pc_render::Options {
            sample_rate: 16000,
            repeats: 1,
            release_frames: 4000,
            ..pc_render::Options::default()
        },
    )
    .unwrap();
    assert_eq!(translated.report.final_tick, 192);
    assert_eq!(rendered.report.body_frames, 16000);
    assert_eq!(rendered.wave.frames(), 20000);
    assert_eq!(rendered.report.clipped_samples, 0);
    let crossings = (1600..6400)
        .filter(|&i| rendered.wave.pcm[i * 2] <= 0 && rendered.wave.pcm[(i + 1) * 2] > 0)
        .count();
    assert!((crossings as f64 / 0.3 - 100.0).abs() < 4.0);
    assert!(rendered.report.peak > 0.01);
}

#[test]
#[ignore = "requires original EMI corpus in BOF3_AUDIO_CORPUS"]
fn every_original_sequence_translates_independently_and_has_consumer_timing_agreement() {
    fn visit(path: &Path, files: &mut Vec<std::path::PathBuf>) {
        for entry in std::fs::read_dir(path).unwrap() {
            let entry = entry.unwrap();
            if entry.file_type().unwrap().is_dir() {
                visit(&entry.path(), files);
            } else if entry
                .path()
                .extension()
                .is_some_and(|s| s.eq_ignore_ascii_case("emi"))
            {
                files.push(entry.path());
            }
        }
    }
    let mut files = Vec::new();
    visit(
        Path::new(&std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()),
        &mut files,
    );
    let (mut sequences, mut loop_stops, mut steps, mut bends, mut total_bytes, mut max_steps) =
        (0, 0, 0, 0, 0, 0);
    for path in files {
        let MediaImage::Emi(image) = MediaImage::read(&path).unwrap() else {
            unreachable!()
        };
        for index in 0..image.entries().len() {
            let bytes = image.entry(index).unwrap();
            if !bytes.starts_with(b"pQES") {
                continue;
            }
            for sequence in SequenceSet::parse(bytes).unwrap().sequences {
                let data = &bytes[sequence.data_offset..sequence.data_offset + sequence.data_bytes];
                let translated = translation::translate(&sequence, data, &Limits::default())
                    .unwrap_or_else(|e| {
                        panic!(
                            "{} entry {index} sequence {}: {e}",
                            path.display(),
                            sequence.sequence_index
                        )
                    });
                assert_eq!(translated.report.sequence_index, sequence.sequence_index);
                assert_eq!(translated.report.sequence_id, sequence.sequence_id);
                let encoded = translated.midi.to_bytes().unwrap();
                let independent = rustysynth::MidiFile::new(&mut Cursor::new(&encoded)).unwrap();
                let scheduled = pc_render::schedule(&translated.midi, 44100).unwrap();
                assert!(
                    (independent.get_length() - scheduled.frames as f64 / 44100.0).abs()
                        < 1.0 / 44100.0 + 1e-6
                );
                assert_eq!(
                    Midi::from_bytes(&encoded).unwrap().to_bytes().unwrap(),
                    encoded
                );
                sequences += 1;
                loop_stops += usize::from(matches!(
                    translated.report.stop,
                    Stop::InfiniteLoopLimit { .. }
                ));
                steps += translated.report.executed_events;
                max_steps = max_steps.max(translated.report.executed_events);
                bends += translated.report.ignored_bend_low_bytes;
                total_bytes += encoded.len();
            }
        }
    }
    assert_eq!(sequences, 476);
    assert_eq!(loop_stops, 466); // Agrees with the independent physical loop-end survey.
    eprintln!("SEP MIDI corpus: {sequences} independent sequences, {loop_stops} infinite-loop stops, {steps} executed events, {max_steps} maximum events, {bends} nonzero bend low bytes normalized, {total_bytes} SMF bytes; bank response and real-time PSX fidelity not verified");
}
