use bof3_audio::{soundfont::Zone, voice::tuning::KeyTuning};

#[test]
fn register_tuning_reports_quantization_and_keeps_export_rate_explicit() {
    for rate in [11025, 22050, 44100, 48000] {
        for key in [0, 36, 60, 84, 127] {
            for register in [1, 127, 128, 4095, 4096, 4660, 8192, 16384, 65535] {
                let row = KeyTuning::from_register(key, 60, 63, register, rate).unwrap();
                assert_eq!(row.pitch_register, register);
                assert_eq!(row.effective_step, register.min(16384));
                assert_eq!(row.sf2_sample_rate, rate);
                assert!(row.pitch_lookup.is_none());
                if let Some(parameters) = &row.sf2 {
                    assert!(parameters.error_cents.abs() <= 0.50000001);
                    let ratio = 2.0f64.powf(
                        (f64::from(key) - f64::from(parameters.root_key)
                            + f64::from(parameters.coarse_tune)
                            + f64::from(parameters.fine_tune) / 100.0)
                            / 12.0,
                    );
                    assert!((ratio * f64::from(rate) - parameters.sf2_pcm_rate).abs() < 1e-8);
                    assert_eq!(
                        parameters.target_pcm_rate,
                        f64::from(register.min(16384)) * 44100.0 / 4096.0
                    );
                } else {
                    assert!(row.unsupported.is_some());
                    assert!(row.zone(&Zone::new(0)).is_err());
                }
            }
        }
    }
    let standard = KeyTuning::from_register(60, 60, 0, 4096, 44100).unwrap();
    let half_rate = KeyTuning::from_register(60, 60, 0, 4096, 22050).unwrap();
    assert_eq!(standard.sf2.unwrap().coarse_tune, 0);
    assert_eq!(half_rate.sf2.unwrap().coarse_tune, 12);
}

#[test]
fn zero_steps_invalid_contexts_and_unrepresentable_tuning_fail_explicitly() {
    let stopped = KeyTuning::from_register(108, 60, 0, 0, 44100).unwrap();
    assert!(stopped.sf2.is_none());
    assert!(stopped.unsupported.unwrap().contains("zero SPU step"));
    assert!(stopped.zone(&Zone::new(0)).is_err());
    let extreme = KeyTuning::from_register(127, 0, 0, 1, 44100).unwrap();
    assert!(extreme.sf2.is_none());
    assert!(extreme.unsupported.unwrap().contains("generator range"));
    assert!(KeyTuning::from_register(128, 60, 0, 4096, 44100).is_err());
    assert!(KeyTuning::from_register(60, 60, 0, 4096, 0).is_err());
    assert!(KeyTuning::from_register(60, 60, 0, 4096, u32::MAX).is_err());
}

#[test]
fn calibrated_single_key_zones_retain_non_pitch_tone_parameters() {
    let row = KeyTuning::from_register(61, 60, 7, 4352, 44100).unwrap();
    let mut template = Zone::new(3);
    template.keys = (50, 70);
    template.velocities = (40, 100);
    template.pan = -250;
    template.attenuation_cb = 150;
    template.envelope.release = 1200;
    template.loop_mode = bof3_audio::soundfont::LoopMode::Continuous;
    template.scale_tuning = 50;
    let zone = row.zone(&template).unwrap();
    assert_eq!(zone.keys, (61, 61));
    assert_eq!(zone.sample, 3);
    assert_eq!(zone.velocities, (40, 100));
    assert_eq!(zone.pan, -250);
    assert_eq!(zone.attenuation_cb, 150);
    assert_eq!(zone.envelope.release, 1200);
    assert_eq!(zone.loop_mode, template.loop_mode);
    assert_eq!(zone.scale_tuning, 100);
    assert_eq!(zone.root_key, Some(60));
    template.keys = (50, 60);
    assert!(row.zone(&template).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS original inputs"]
fn every_corpus_tone_key_executes_original_pitch_and_reports_sf2_error() {
    use bof3_audio::{
        catalog::model::AssetData, catalog::model::Catalog, machine::executable::Executable,
        machine::profile::Profile, voice::tuning::LookupRegion, voice::tuning::Reference,
    };
    use std::collections::{BTreeMap, BTreeSet};
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let profile = Profile::identify(&exe).unwrap();
    let ram = exe.load_ram();
    let mut reference = Reference::from_executable(&exe).unwrap();
    assert_eq!(reference.profile().exe_sha256, profile.exe_sha256);
    assert!(reference.pitch_register(128, 60, 0).is_err());
    for (key, expected) in [
        (0, 128),
        (60, 4096),
        (72, 8192),
        (84, 16384),
        (96, 32768),
        (108, 0),
    ] {
        assert_eq!(reference.pitch_register(key, 60, 0).unwrap(), expected);
    }
    let root = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let catalog = Catalog::read(Some(&root), &[]).unwrap();
    let mut contexts = BTreeMap::new();
    let mut keys = BTreeSet::new();
    let (
        mut banks,
        mut tones,
        mut key_count,
        mut zero,
        mut unrepresentable,
        mut clamped,
        mut outside_table,
    ) = (0, 0, 0, 0, 0, 0, 0);
    let mut max_error = 0.0f64;
    let mut lookup_regions = [0usize; 3];
    for asset in &catalog.assets {
        let AssetData::Bank { metadata, .. } = &asset.data else {
            continue;
        };
        banks += 1;
        for program in &metadata.programs {
            for tone in &program.tones {
                tones += 1;
                let context = (tone.center, tone.shift, tone.key_min, tone.key_max);
                if let std::collections::btree_map::Entry::Vacant(entry) = contexts.entry(context) {
                    entry.insert(reference.tone(tone, 44100).unwrap_or_else(|e| {
                        panic!(
                            "{} program={} tone={}: {e}",
                            asset.id, program.program, tone.index
                        )
                    }));
                }
                for row in &contexts[&context] {
                    key_count += 1;
                    keys.insert((row.key, row.center, row.shift));
                    // Independent arithmetic from the instruction sequence; no
                    // call through Reference or the old C renderer for expected values.
                    let fine = i32::from(tone.shift) / 8;
                    let semitone =
                        i32::from(row.key) + 60 - i32::from(tone.center) + i32::from(fine >= 16);
                    let index = (semitone % 12) * 16 + fine % 16;
                    assert_eq!(row.pitch_table_index, Some(index));
                    if row.pitch_lookup.unwrap().region == LookupRegion::AdjacentData {
                        assert!(row.sf2.is_none());
                        assert!(row
                            .unsupported
                            .unwrap()
                            .contains("outside verified pitch table"));
                    }
                    outside_table += usize::from(!(0..193).contains(&index));
                    let address = (i64::from(profile.pitch_table - 0x8000_0000)
                        + i64::from(index) * 2) as usize;
                    let value = u32::from(u16::from_le_bytes(
                        ram[address..address + 2].try_into().unwrap(),
                    ));
                    let lookup = row.pitch_lookup.unwrap();
                    assert_eq!(lookup.runtime_address as usize, address + 0x8000_0000);
                    assert_eq!(lookup.source_file_offset as usize, address - 0x96000);
                    assert_eq!(u32::from(lookup.value), value);
                    lookup_regions[match lookup.region {
                        LookupRegion::NoteOnPitchTable => 0,
                        LookupRegion::EarlierSpuPitchTable => 1,
                        LookupRegion::AdjacentData => 2,
                    }] += 1;
                    let octave = semitone / 12 - 5;
                    let expected = if octave >= 0 {
                        value.wrapping_shl(octave as u32)
                    } else {
                        value.wrapping_shr((-octave) as u32)
                    } as u16;
                    assert_eq!(
                        row.pitch_register, expected,
                        "{} program={} tone={} key={}",
                        asset.id, program.program, tone.index, row.key
                    );
                    clamped += usize::from(expected > 0x4000);
                    zero += usize::from(expected == 0);
                    if let Some(p) = &row.sf2 {
                        assert!(p.error_cents.abs() <= 0.50000001);
                        max_error = max_error.max(p.error_cents.abs());
                    } else {
                        unrepresentable += 1;
                        assert!(row.zone(&Zone::new(0)).is_err());
                    }
                }
            }
        }
    }
    assert_eq!(
        (banks, tones, contexts.len(), keys.len(), key_count),
        (1020, 21112, 1549, 30119, 236302)
    );
    assert_eq!((zero, clamped, outside_table), (3177, 32977, 17286));
    assert_eq!(unrepresentable, 4976);
    assert_eq!(lookup_regions[1] + lookup_regions[2], outside_table);
    assert_eq!(lookup_regions, [219016, 12623, 4663]);
    eprintln!(
        "Lookup witnesses: note-on table={} earlier SPU table={} adjacent data={}",
        lookup_regions[0], lookup_regions[1], lookup_regions[2]
    );
    eprintln!("Pitch corpus: banks={banks} tones={tones} contexts={} unique_keys={} key_contexts={key_count} zero={zero} unrepresentable={unrepresentable} clamped={clamped} outside_table={outside_table} max_error_cents={max_error:.9}", contexts.len(), keys.len());
}

#[test]
fn alternate_root_encodes_extreme_tuning_without_changing_the_requested_pitch() {
    use bof3_audio::soundfont::{Bank, Instrument, LoopMode, Preset, Sample};
    use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
    use std::{io::Cursor, sync::Arc};
    // Actual corpus context: preferred root 46 requires coarse -127. A root
    // change preserves combined cents and brings both generators into range.
    let row = KeyTuning::from_register(121, 46, 114, 192, 44100).unwrap();
    let fit = row.sf2.as_ref().unwrap();
    assert_eq!((row.center, row.shift), (46, 114));
    assert_eq!(
        (fit.root_key, fit.coarse_tune, fit.fine_tune),
        (53, -120, -98)
    );
    assert!(fit.error_cents.abs() < 0.5);
    let mut zone = row.zone(&Zone::new(0)).unwrap();
    zone.loop_mode = LoopMode::Continuous;
    let bank = Bank {
        name: "extreme tuning".into(),
        samples: vec![Sample {
            name: "sine".into(),
            pcm: (0..1024)
                .map(|i| (12000.0 * (std::f64::consts::TAU * f64::from(i) / 64.0).sin()) as i16)
                .collect(),
            rate: 44100,
            root_key: 60,
            correction_cents: 0,
            loop_range: Some(64..960),
        }],
        instruments: vec![Instrument {
            name: "tone".into(),
            zones: vec![zone],
        }],
        presets: vec![Preset {
            name: "preset".into(),
            bank: 0,
            program: 0,
            instruments: vec![0],
        }],
    };
    let font = SoundFont::new(&mut Cursor::new(bank.encode().unwrap().bytes)).unwrap();
    let mut settings = SynthesizerSettings::new(44100);
    settings.enable_reverb_and_chorus = false;
    let mut synth = Synthesizer::new(&Arc::new(font), &settings).unwrap();
    synth.note_on(0, 121, 127);
    let mut left = vec![0.0; 44100];
    let mut right = left.clone();
    synth.render(&mut left, &mut right);
    let crossings: Vec<_> = left
        .windows(2)
        .enumerate()
        .filter(|(i, w)| *i >= 4410 && w[0] <= 0.0 && w[1] > 0.0)
        .map(|(i, _)| i)
        .collect();
    assert!(crossings.len() > 20);
    let hz =
        (crossings.len() - 1) as f64 * 44100.0 / (crossings.last().unwrap() - crossings[0]) as f64;
    let expected = fit.target_pcm_rate / 64.0;
    assert!(
        (hz / expected - 1.0).abs() < 0.002,
        "{hz} Hz versus {expected} Hz"
    );
}
