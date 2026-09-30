use bof3_audio::{
    machine::adsr::Model, machine::adsr::Registers, soundfont::envelope,
    soundfont::envelope::Parameters, soundfont::envelope::Probe,
};

#[test]
fn fitting_reports_dynamic_sustain_and_does_not_hide_large_short_transients() {
    let probe = Probe {
        held_frames: vec![11025, 44100],
        release_frames: 11025,
        sample_stride: 441,
    };
    let moving = envelope::fit(
        Registers {
            adsr1: 0x000f,
            adsr2: 0x4c00,
        },
        Model::Published,
        &probe,
    )
    .unwrap();
    assert_eq!(moving.attack_end_frame, Some(3));
    assert_eq!(moving.decay_end_frame, Some(4));
    assert_eq!(moving.sustain_min, Some(0));
    assert!(moving.sustain_max.unwrap() > 16000);
    assert!(moving.error.weighted_rms <= moving.seed_error.weighted_rms);
    assert!(moving.error.weighted_rms.is_finite());
    assert!(moving.error.maximum_at_probe > 0.1);
    assert_eq!(moving.releases_still_active, 0);
    assert_eq!(moving.parameters.envelope().delay, i16::MIN);
    let transient = envelope::fit(
        Registers {
            adsr1: 0,
            adsr2: 0x4000,
        },
        Model::Published,
        &probe,
    )
    .unwrap();
    assert!(transient.error.weighted_rms < 0.03);
    assert!(transient.error.maximum_at_probe > 0.3); // RMS is not a fidelity gate.
}

#[test]
fn model_choice_frozen_release_and_probe_bounds_remain_explicit() {
    let probe = Probe {
        held_frames: vec![11025],
        release_frames: 11025,
        sample_stride: 441,
    };
    let frozen = envelope::fit(
        Registers {
            adsr1: 0x000f,
            adsr2: 31,
        },
        Model::Published,
        &probe,
    )
    .unwrap();
    assert_eq!(frozen.releases_still_active, 1);
    assert_eq!(frozen.parameters.release_tc, 8000);
    assert!(frozen.error.maximum_at_probe > 0.0);
    for model in [Model::Published, Model::EmulatorReference] {
        let fit = envelope::fit(
            Registers {
                adsr1: 0x89ba,
                adsr2: 0x514c,
            },
            model,
            &probe,
        )
        .unwrap();
        assert_eq!(fit.model, model);
        assert!(fit.error.weighted_rms <= fit.seed_error.weighted_rms);
    }
    for invalid in [
        Probe {
            held_frames: vec![],
            ..probe.clone()
        },
        Probe {
            sample_stride: 0,
            ..probe.clone()
        },
        Probe {
            release_frames: 0,
            ..probe.clone()
        },
        Probe {
            held_frames: vec![u32::MAX],
            ..probe.clone()
        },
    ] {
        assert!(
            envelope::fit(Registers { adsr1: 0, adsr2: 0 }, Model::Published, &invalid).is_err()
        );
    }
}

#[test]
fn target_curve_agrees_with_actual_soundfont_synthesis_away_from_block_boundaries() {
    use bof3_audio::soundfont::{Bank, Envelope, Instrument, LoopMode, Preset, Sample, Zone};
    use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
    use std::{io::Cursor, sync::Arc};
    let parameters = Parameters {
        attack_tc: -2400,
        decay_tc: 1200,
        sustain_cb: 140,
        release_tc: 0,
    };
    let synth = |envelope| {
        let mut zone = Zone::new(0);
        zone.loop_mode = LoopMode::Continuous;
        zone.envelope = envelope;
        let sf2 = Bank {
            name: "Envelope probe".into(),
            samples: vec![Sample {
                name: "DC".into(),
                pcm: vec![12000; 1024],
                rate: 44100,
                root_key: 69,
                correction_cents: 0,
                loop_range: Some(64..960),
            }],
            instruments: vec![Instrument {
                name: "voice".into(),
                zones: vec![zone],
            }],
            presets: vec![Preset {
                name: "program".into(),
                bank: 0,
                program: 0,
                instruments: vec![0],
            }],
        }
        .encode()
        .unwrap()
        .bytes;
        let font = Arc::new(SoundFont::new(&mut Cursor::new(sf2)).unwrap());
        let mut settings = SynthesizerSettings::new(44100);
        settings.block_size = 8;
        settings.enable_reverb_and_chorus = false;
        let mut synth = Synthesizer::new(&font, &settings).unwrap();
        synth.note_on(0, 69, 127);
        synth
    };
    let mut target = synth(parameters.envelope());
    let mut baseline = synth(Envelope {
        delay: i16::MIN,
        attack: i16::MIN,
        hold: i16::MIN,
        decay: i16::MIN,
        sustain_cb: 0,
        release: 0,
    });
    let held = 44104usize; // A consumer block boundary.
    let mut actual = vec![0f32; held * 2];
    let mut base = vec![0f32; held * 2];
    let mut right = vec![0f32; held * 2];
    baseline.render(&mut base, &mut right);
    target.render(&mut actual[..held], &mut right[..held]);
    target.note_off(0, 69);
    target.render(&mut actual[held..], &mut right[held..]);
    for frame in [
        2048,
        8192,
        12288,
        22048,
        40000,
        held + 4096,
        held + 16384,
        held + 40000,
    ] {
        let normalized = f64::from(actual[frame - 1] / base[frame - 1]);
        let expected = envelope::target_level(&parameters, held as u32, frame as u32);
        assert!(
            (normalized - expected).abs() < 0.003,
            "frame {frame}: consumer {normalized}, target {expected}"
        );
    }
}

#[test]
fn release_minimum_is_part_of_the_target_not_silently_represented_as_zero() {
    let mut parameters = Parameters {
        attack_tc: i16::MIN,
        decay_tc: i16::MIN,
        sustain_cb: 0,
        release_tc: i16::MIN,
    };
    let zero_request = envelope::target_level(&parameters, 1000, 1100);
    parameters.release_tc = -12000;
    assert_eq!(
        envelope::target_level(&parameters, 1000, 1100),
        zero_request
    );
    assert!(zero_request > 0.1);
    assert_eq!(envelope::target_level(&parameters, 1000, 1441), 0.0);
}

#[test]
#[ignore = "requires original EMI corpus in BOF3_AUDIO_CORPUS; measures both explicit ADSR models"]
fn corpus_fits_report_error_and_unfinished_envelopes_instead_of_claiming_equivalence() {
    use bof3_audio::{archive::MediaImage, bank::Bank};
    use std::{collections::BTreeMap, path::Path};
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
    let mut pairs = BTreeMap::<(u16, u16), usize>::new();
    for path in files {
        let MediaImage::Emi(image) = MediaImage::read(&path).unwrap() else {
            unreachable!()
        };
        for index in 0..image.entries().len() {
            let bytes = image.entry(index).unwrap();
            if !bytes.starts_with(b"pBAV") {
                continue;
            }
            for program in Bank::parse(bytes).unwrap().programs {
                for tone in program.tones {
                    *pairs.entry((tone.adsr1, tone.adsr2)).or_default() += 1;
                }
            }
        }
    }
    assert_eq!(pairs.len(), 543);
    assert_eq!(pairs.values().sum::<usize>(), 21112);
    for model in [Model::Published, Model::EmulatorReference] {
        let mut worst = Vec::new();
        let mut moving = 0;
        let mut active = 0;
        let mut over_five_percent = (0, 0);
        let mut no_attack_end = 0;
        for (&(adsr1, adsr2), &tones) in &pairs {
            let fit = envelope::fit(Registers { adsr1, adsr2 }, model, &Probe::default()).unwrap();
            assert!(fit.error.weighted_rms.is_finite());
            assert!(fit.error.weighted_rms <= fit.seed_error.weighted_rms);
            moving += usize::from(fit.sustain_min != fit.sustain_max);
            active += usize::from(fit.releases_still_active != 0);
            no_attack_end += usize::from(fit.attack_end_frame.is_none());
            if fit.error.weighted_rms > 0.05 {
                over_five_percent.0 += 1;
                over_five_percent.1 += tones;
            }
            worst.push((
                fit.error.weighted_rms,
                fit.error.maximum_at_probe,
                adsr1,
                adsr2,
                tones,
            ));
        }
        worst.sort_by(|a, b| b.0.total_cmp(&a.0));
        eprintln!("Envelope corpus {model:?}: 543 pairs / 21112 tones; {moving} moving-sustain pairs; {active} pairs with release active after probe; {no_attack_end} pairs without attack end; RMS >0.05: {} pairs / {} tones; worst (RMS,max,ADSR1,ADSR2,tones): {:?}", over_five_percent.0, over_five_percent.1, &worst[..5]);
    }
}
