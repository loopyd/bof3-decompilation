use bof3_audio::soundfont::{Bank, Instrument, LoopMode, Preset, Sample, Zone};
use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
use std::io::Cursor;
use std::sync::Arc;

fn bank() -> Bank {
    let sample = Sample {
        name: "shared sine".into(),
        pcm: (0..2646)
            .map(|i| (6000.0 * (std::f64::consts::TAU * i as f64 / 441.0).sin()) as i16)
            .collect(),
        rate: 44100,
        root_key: 69,
        correction_cents: 0,
        loop_range: Some(441..2205),
    };
    let mut zone = Zone::new(0);
    zone.keys = (69, 81);
    zone.velocities = (40, 127);
    zone.loop_mode = LoopMode::Continuous;
    zone.envelope.release = -3600;
    let mut octave = zone.clone();
    octave.coarse_tune = 12;
    Bank {
        name: "Synthetic SF2 test".into(),
        samples: vec![sample],
        instruments: vec![
            Instrument {
                name: "base".into(),
                zones: vec![zone],
            },
            Instrument {
                name: "octave".into(),
                zones: vec![octave],
            },
        ],
        presets: vec![
            Preset {
                name: "base preset".into(),
                bank: 0,
                program: 0,
                instruments: vec![0],
            },
            Preset {
                name: "octave preset".into(),
                bank: 0,
                program: 5,
                instruments: vec![1],
            },
        ],
    }
}

fn consumer(bank: &Bank) -> SoundFont {
    SoundFont::new(&mut Cursor::new(bank.encode().unwrap().bytes)).unwrap()
}

fn chunks(bytes: &[u8]) -> Vec<([u8; 4], &[u8])> {
    let mut result = Vec::new();
    let mut at = 0;
    while at < bytes.len() {
        assert!(at + 8 <= bytes.len());
        let size = u32::from_le_bytes(bytes[at + 4..at + 8].try_into().unwrap()) as usize;
        assert!(at + 8 + size <= bytes.len());
        result.push((
            bytes[at..at + 4].try_into().unwrap(),
            &bytes[at + 8..at + 8 + size],
        ));
        at += 8 + size;
        if !size.is_multiple_of(2) {
            assert_eq!(bytes[at], 0);
            at += 1;
        }
    }
    assert_eq!(at, bytes.len());
    result
}

#[test]
fn generated_riff_has_complete_tables_terminal_records_and_per_sample_guards() {
    let mut bank = bank();
    let mut second = bank.samples[0].clone();
    second.name = "second".into();
    second.pcm = vec![i16::MIN, i16::MAX, 1, -1];
    second.loop_range = None;
    bank.samples.push(second);
    let encoded = bank.encode().unwrap();
    assert_eq!(encoded.diagnostics.len(), 1);
    let riff = chunks(&encoded.bytes);
    assert_eq!(riff.len(), 1);
    assert_eq!(riff[0].0, *b"RIFF");
    assert_eq!(&riff[0].1[..4], b"sfbk");
    let lists = chunks(&riff[0].1[4..]);
    assert_eq!(
        lists
            .iter()
            .map(|(id, data)| (id, &data[..4]))
            .collect::<Vec<_>>(),
        vec![
            (b"LIST", b"INFO".as_slice()),
            (b"LIST", b"sdta".as_slice()),
            (b"LIST", b"pdta".as_slice())
        ]
    );
    let info = chunks(&lists[0].1[4..]);
    assert_eq!(info[0], (*b"ifil", [2, 0, 1, 0].as_slice()));
    for (_, s) in &info[1..] {
        assert_eq!(s.len() % 2, 0);
        assert_eq!(s.last(), Some(&0));
    }
    let sdta = chunks(&lists[1].1[4..]);
    assert_eq!(sdta.len(), 1);
    assert_eq!(sdta[0].0, *b"smpl");
    let pcm: Vec<_> = sdta[0]
        .1
        .as_chunks::<2>()
        .0
        .iter()
        .map(|b| i16::from_le_bytes(*b))
        .collect();
    let mut at = 0;
    for sample in &bank.samples {
        assert_eq!(&pcm[at..at + sample.pcm.len()], &sample.pcm);
        at += sample.pcm.len();
        assert!(pcm[at..at + 46].iter().all(|&p| p == 0));
        at += 46;
    }
    assert_eq!(at, pcm.len());
    let pdta = chunks(&lists[2].1[4..]);
    let expected = [
        (*b"phdr", 38),
        (*b"pbag", 4),
        (*b"pmod", 10),
        (*b"pgen", 4),
        (*b"inst", 22),
        (*b"ibag", 4),
        (*b"imod", 10),
        (*b"igen", 4),
        (*b"shdr", 46),
    ];
    assert_eq!(pdta.len(), expected.len());
    for ((id, data), (expected_id, size)) in pdta.iter().zip(expected) {
        assert_eq!(*id, expected_id);
        assert!(!data.is_empty());
        assert_eq!(data.len() % size, 0);
    }
    for (part, size, terminal) in [(0, 38, b"EOP"), (4, 22, b"EOI"), (8, 46, b"EOS")] {
        let data = pdta[part].1;
        assert_eq!(&data[data.len() - size..][..3], terminal);
    }
    let half = |data: &[u8], at: usize| {
        usize::from(u16::from_le_bytes(data[at..at + 2].try_into().unwrap()))
    };
    // Consumers may ignore sentinel fields, so check their cross-table bounds too.
    for (headers, record, bag_offset, bags, generators) in [(0, 38, 24, 1, 3), (4, 22, 20, 5, 7)] {
        let h = pdta[headers].1;
        let b = pdta[bags].1;
        let g = pdta[generators].1;
        assert_eq!(half(h, h.len() - record + bag_offset), b.len() / 4 - 1);
        assert_eq!(half(b, b.len() - 4), g.len() / 4 - 1);
        assert!(b.as_chunks::<4>().0.iter().all(|b| half(b, 2) == 0));
        assert_eq!(&g[g.len() - 4..], &[0; 4]);
    }
    assert_eq!(pdta[2].1, &[0; 10]);
    assert_eq!(pdta[6].1, &[0; 10]);
    // Consumer parses samples and all cross-table links, including the unused sample.
    let sf = consumer(&bank);
    assert_eq!(sf.get_sample_headers().len(), 2);
    assert_eq!(sf.get_presets().len(), 2);
    assert_eq!(sf.get_instruments().len(), 2);
    assert_eq!(sf.get_sample_headers()[1].get_start(), 2692);
    assert_eq!(sf.get_sample_headers()[1].get_end(), 2696);
}

#[test]
fn independent_consumer_preserves_shared_samples_layers_ranges_and_zone_parameters() {
    let mut bank = bank();
    bank.samples[0].correction_cents = -7;
    let mut layer = bank.instruments[0].zones[0].clone();
    layer.keys = (72, 90);
    layer.velocities = (50, 100);
    layer.root_key = Some(71);
    layer.fine_tune = 23;
    layer.scale_tuning = 75;
    layer.pan = -250;
    layer.attenuation_cb = 100;
    layer.envelope.attack = -1200;
    layer.envelope.decay = 0;
    layer.envelope.sustain_cb = 200;
    layer.envelope.release = 1200;
    layer.loop_mode = LoopMode::UntilRelease;
    bank.instruments[0].zones.push(layer);
    let sf = consumer(&bank);
    let regions = sf.get_instruments()[0].get_regions();
    assert_eq!(regions.len(), 2);
    assert!(regions.iter().all(|r| r.contains(75, 80)));
    let r = &regions[1];
    assert!(!r.contains(71, 80));
    assert!(!r.contains(91, 80));
    assert!(!r.contains(75, 49));
    assert!(!r.contains(75, 101));
    assert_eq!(r.get_sample_id(), regions[0].get_sample_id());
    assert_eq!(r.get_root_key(), 71);
    // The consumer combines zone tuning with sample-header correction.
    assert_eq!(r.get_fine_tune(), 16);
    assert_eq!(r.get_scale_tuning(), 75);
    assert_eq!(r.get_pan(), -25.0);
    assert_eq!(r.get_initial_attenuation(), 10.0);
    assert_eq!(r.get_sustain_volume_envelope(), 20.0);
    assert_eq!(r.get_sample_start_loop(), 441);
    assert_eq!(r.get_sample_end_loop(), 2205);
    assert_eq!(r.get_sample_modes(), rustysynth::LoopMode::LoopUntilNoteOff);
    assert!((r.get_attack_volume_envelope() - 0.5).abs() < 1e-6);
    assert!((r.get_decay_volume_envelope() - 1.0).abs() < 1e-6);
    assert!((r.get_release_volume_envelope() - 2.0).abs() < 1e-6);
    assert_eq!(sf.get_sample_headers()[0].get_pitch_correction(), -7);
    assert_eq!(sf.get_presets()[1].get_patch_number(), 5);
    assert_eq!(sf.get_presets()[1].get_regions()[0].get_instrument_id(), 1);
}

fn synth(bank: &Bank) -> Synthesizer {
    let mut settings = SynthesizerSettings::new(44100);
    settings.enable_reverb_and_chorus = false;
    Synthesizer::new(&Arc::new(consumer(bank)), &settings).unwrap()
}

fn render(s: &mut Synthesizer, frames: usize) -> (Vec<f32>, Vec<f32>) {
    let (mut left, mut right) = (vec![0.0; frames], vec![0.0; frames]);
    s.render(&mut left, &mut right);
    assert!(left.iter().chain(&right).all(|x| x.is_finite()));
    (left, right)
}

fn energy(pcm: &[f32]) -> f32 {
    pcm.iter().map(|v| v * v).sum()
}

#[test]
fn independent_synthesis_exercises_program_tuning_loop_release_and_key_velocity_limits() {
    let bank = bank();
    let mut frequencies = Vec::new();
    for (program, key, velocity) in [
        (0, 69, 100),
        (5, 69, 100),
        (0, 81, 100),
        (0, 68, 100),
        (0, 69, 39),
    ] {
        let mut s = synth(&bank);
        s.process_midi_message(0, 0xc0, program, 0);
        s.note_on(0, key, velocity);
        let (left, _) = render(&mut s, 22050);
        let stable = &left[11025..];
        let frequency = stable
            .windows(2)
            .filter(|p| p[0] <= 0.0 && p[1] > 0.0)
            .count()
            * 4;
        frequencies.push(frequency);
        if key >= 69 && velocity >= 40 {
            assert!(energy(stable) > 1.0); // Still playing after many sample traversals.
            s.note_off(0, key);
            let (tail, _) = render(&mut s, 22050);
            assert!(energy(&tail[..1024]) > 0.0);
            assert_eq!(energy(&tail[11025..]), 0.0);
        } else {
            assert_eq!(energy(&left), 0.0);
        }
    }
    assert_eq!(frequencies, vec![100, 200, 200, 0, 0]);
}

#[test]
fn layers_are_audible_and_until_release_exits_sample_loop() {
    let mut bank = bank();
    bank.instruments[0].zones[0].pan = -500;
    let mut s = synth(&bank);
    s.note_on(0, 69, 100);
    let (left, right) = render(&mut s, 4410);
    assert!(energy(&left) > 1.0);
    assert!(energy(&right) < 1e-8);
    let mut right_layer = bank.instruments[0].zones[0].clone();
    right_layer.pan = 500;
    bank.instruments[0].zones.push(right_layer);
    let mut s = synth(&bank);
    s.note_on(0, 69, 100);
    let (left, right) = render(&mut s, 4410);
    assert!(energy(&left) > 1.0 && energy(&right) > 1.0);

    bank.instruments[0].zones.truncate(1);
    bank.instruments[0].zones[0].envelope.release = 2400; // 4 seconds
    let mut continuous = synth(&bank);
    bank.instruments[0].zones[0].loop_mode = LoopMode::UntilRelease;
    let mut until_release = synth(&bank);
    for s in [&mut continuous, &mut until_release] {
        s.note_on(0, 69, 100);
        render(s, 44100);
        s.note_off(0, 69);
    }
    let (a, _) = render(&mut continuous, 22050);
    let (b, _) = render(&mut until_release, 22050);
    assert!(energy(&a[11025..]) > 1.0);
    assert_eq!(energy(&b[11025..]), 0.0);
}

#[test]
fn exported_midi_and_soundfont_play_together_in_independent_consumer() {
    use bof3_audio::interchange::midi::{Event, Message, Midi, Track};
    let channel = |tick, status, data: &[u8]| Event {
        tick,
        message: Message::Channel {
            status,
            data: data.to_vec(),
        },
    };
    let end = Event {
        tick: 192,
        message: Message::end(),
    };
    let midi = Midi::new(
        1,
        96,
        vec![
            Track::new(vec![
                Event {
                    tick: 0,
                    message: Message::Meta {
                        kind: 0x51,
                        data: vec![7, 0xa1, 0x20],
                    },
                },
                end.clone(),
            ]),
            Track::new(vec![
                channel(0, 0xc0, &[0]),
                channel(0, 0x90, &[69, 100]),
                channel(96, 0x80, &[69, 0]),
                channel(96, 0xc0, &[5]),
                channel(96, 0x90, &[69, 100]),
                channel(192, 0x80, &[69, 0]),
                end,
            ]),
        ],
    )
    .unwrap();
    let independent =
        rustysynth::MidiFile::new(&mut Cursor::new(midi.to_bytes().unwrap())).unwrap();
    assert_eq!(independent.get_length(), 1.0);
    let mut sequencer = rustysynth::MidiFileSequencer::new(synth(&bank()));
    sequencer.play(&Arc::new(independent), false);
    let (mut left, mut right) = (vec![0.0; 66150], vec![0.0; 66150]);
    sequencer.render(&mut left, &mut right);
    assert!(sequencer.end_of_sequence());
    assert!(left.iter().chain(&right).all(|x| x.is_finite()));
    for (start, hz) in [(5512, 100), (27562, 200)] {
        let section = &left[start..start + 11025];
        assert!(energy(section) > 1.0);
        let measured = section
            .windows(2)
            .filter(|p| p[0] <= 0.0 && p[1] > 0.0)
            .count()
            * 4;
        assert_eq!(measured, hz);
    }
    assert_eq!(energy(&left[55125..]), 0.0);
    assert_eq!(energy(&right[55125..]), 0.0);
}

#[test]
fn invalid_or_ambiguous_banks_fail_and_portability_limits_are_reported_without_rewriting() {
    let base = bank();
    let mutations: &[fn(&mut Bank)] = &[
        |b| b.samples.clear(),
        |b| b.instruments.clear(),
        |b| b.presets.clear(),
        |b| b.samples[0].pcm.clear(),
        |b| b.samples[0].rate = 0,
        |b| b.samples[0].root_key = 128,
        |b| b.samples[0].correction_cents = 100,
        |b| b.samples[0].loop_range = Some(500..500),
        |b| b.samples[0].loop_range = Some(500..3000),
        |b| b.samples[0].loop_range = None,
        |b| b.samples[0].name = "EOS".into(),
        |b| b.name = "non-ASCII 🦀".into(),
        |b| b.instruments[0].name = "base\0tail".into(),
        |b| b.instruments[0].name = "12345678901234567890".into(),
        |b| b.instruments[0].zones[0].sample = 1,
        |b| b.instruments[0].zones[0].keys = (82, 81),
        |b| b.instruments[0].zones[0].velocities = (0, 128),
        |b| b.instruments[0].zones[0].pan = -501,
        |b| b.instruments[0].zones[0].fine_tune = -100,
        |b| b.instruments[0].zones[0].coarse_tune = 121,
        |b| b.instruments[0].zones[0].attenuation_cb = 1441,
        |b| b.instruments[0].zones[0].envelope.hold = 5001,
        |b| b.instruments[0].zones[0].envelope.attack = -12001,
        |b| b.instruments[0].zones[0].envelope.sustain_cb = 1441,
        |b| b.presets[0].instruments.push(2),
        |b| b.presets[0].program = 128,
        |b| b.presets[0].bank = 129,
        |b| b.presets[1].program = 0,
        |b| b.instruments[1].name = b.instruments[0].name.clone(),
    ];
    for (i, mutate) in mutations.iter().enumerate() {
        let mut b = base.clone();
        mutate(&mut b);
        assert!(b.encode().is_err(), "accepted mutation {i}");
    }
    let mut b = base.clone();
    b.samples[0].loop_range = Some(0..2646);
    let encoded = b.encode().unwrap();
    assert_eq!(encoded.diagnostics.len(), 1);
    let sf = consumer(&b);
    assert_eq!(sf.get_sample_headers()[0].get_start_loop(), 0);
    assert_eq!(sf.get_sample_headers()[0].get_end_loop(), 2646);
    assert_eq!(&sf.get_wave_data()[..2646], &base.samples[0].pcm);
    b.instruments[0].zones = vec![Zone::new(0); 5000];
    assert!(b
        .encode()
        .unwrap_err()
        .to_string()
        .contains("generator count"));
}
