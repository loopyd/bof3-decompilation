use bof3_audio::{
    bank::Bank, machine::adsr::Model, machine::adsr::Registers, soundfont::bank as binding,
    soundfont::bank::Gain, soundfont::bank::Identity, soundfont::bank::ToneContext,
    soundfont::envelope, soundfont::envelope::Probe, voice::tuning::KeyTuning,
};

#[path = "../support/corpus/assignment.rs"]
mod assignment_corpus;
use crate::fixture as bank_fixture;
#[path = "../support/corpus/pitch.rs"]
mod pitch_corpus;
#[path = "../support/corpus/tone.rs"]
mod tone_corpus;
#[path = "../support/corpus/activation.rs"]
mod unmute_corpus;
use bank_fixture::{contexts, fixture, identity, options};

#[test]
fn sparse_programs_layered_tones_shared_samples_and_empty_identities_survive_binding() {
    use rustysynth::SoundFont;
    let (vh, vb) = fixture();
    let bound = binding::bind(identity(), &vh, &vb, &contexts(&vh), &options()).unwrap();
    assert_eq!(bound.report.identity.game_bank_id, 7);
    assert_eq!(bound.report.source_metadata.header_id, 99);
    assert_eq!(bound.report.samples.len(), 2);
    assert_eq!(bound.report.samples[0].sf2_sample, None);
    assert_eq!(bound.report.samples[1].sf2_sample, Some(0));
    assert_eq!(bound.report.samples[1].loop_range, Some((0, 112)));
    assert_eq!(
        bound
            .report
            .programs
            .iter()
            .map(|p| (p.source_program, p.source_tone_block))
            .collect::<Vec<_>>(),
        [(0, 0), (5, 1)]
    );
    assert_eq!(bound.report.tones.len(), 3);
    for tone in &bound.report.tones {
        assert_eq!(tone.zone_count, 2);
        assert_eq!(tone.sf2_sample, 0);
    }
    let consumer = SoundFont::new(&mut std::io::Cursor::new(bound.bytes)).unwrap();
    assert_eq!(consumer.get_instruments().len(), 4);
    assert_eq!(
        consumer
            .get_presets()
            .iter()
            .filter(|p| [0, 5].contains(&p.get_patch_number()))
            .map(|p| (p.get_bank_number(), p.get_patch_number()))
            .collect::<Vec<_>>(),
        [(0, 0), (128, 0), (0, 5), (128, 5)]
    );
}

#[test]
fn consumer_plays_both_layers_and_percussion_alias_without_out_of_range_notes() {
    use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
    use std::{io::Cursor, sync::Arc};
    let (vh, vb) = fixture();
    let bound = binding::bind(identity(), &vh, &vb, &contexts(&vh), &options()).unwrap();
    let font = Arc::new(SoundFont::new(&mut Cursor::new(bound.bytes)).unwrap());
    let render = |channel, program, key| {
        let mut settings = SynthesizerSettings::new(44100);
        settings.enable_reverb_and_chorus = false;
        let mut synth = Synthesizer::new(&font, &settings).unwrap();
        synth.process_midi_message(channel, 0xc0, program, 0);
        synth.note_on(channel, key, 100);
        let mut l = vec![0.0; 4410];
        let mut r = vec![0.0; 4410];
        synth.render(&mut l, &mut r);
        (l, r)
    };
    let layers = render(0, 0, 60);
    let energy = |samples: &[f32]| samples.iter().map(|&s| f64::from(s).powi(2)).sum::<f64>();
    assert!(energy(&layers.0) > 1.0 && energy(&layers.1) > 1.0);
    assert!((energy(&layers.0) / energy(&layers.1) - 1.0).abs() < 0.05);
    assert_eq!(render(9, 0, 60), layers);
    assert!(energy(&render(0, 5, 60).0) < energy(&layers.0) / 2.0);
    let outside = render(0, 0, 62);
    assert!(outside.0.iter().chain(&outside.1).all(|&s| s == 0.0));
}

#[test]
fn missing_conflicting_and_unrepresentable_contexts_are_rejected() {
    let (vh, vb) = fixture();
    let context = contexts(&vh);
    let fails =
        |items: &[ToneContext]| binding::bind(identity(), &vh, &vb, items, &options()).is_err();
    assert!(fails(&context[..2]));
    let mut changed = context.clone();
    changed.push(changed[0].clone());
    assert!(fails(&changed));
    let mut changed = context.clone();
    changed[0].tuning.pop();
    assert!(fails(&changed));
    let mut changed = context.clone();
    changed[0].tuning[0] = KeyTuning::from_register(60, 60, 0, 0, 44100).unwrap();
    assert!(fails(&changed));
    let mut changed = context.clone();
    changed[0].gain.provenance.clear();
    assert!(fails(&changed));
    let mut changed = context.clone();
    changed[0].envelope.registers.adsr1 ^= 1;
    assert!(fails(&changed));
    let mut changed = context.clone();
    changed[0].tone = 9;
    assert!(fails(&changed));
    let mut changed_vh = vh.clone();
    changed_vh[0x820 + 1] = 1;
    assert!(binding::bind(identity(), &changed_vh, &vb, &context, &options()).is_err());
    let mut changed_vh = vh.clone();
    changed_vh[0x820 + 8] = 1;
    assert!(binding::bind(identity(), &changed_vh, &vb, &context, &options()).is_err());
    assert!(binding::bind(identity(), &vh, &vb[..63], &context, &options()).is_err());
}

#[test]
fn zero_sample_reference_retains_raw_identity_and_uses_sample_two_pcm() {
    let (mut vh, vb) = fixture();
    let context = contexts(&vh);
    let baseline = binding::bind(identity(), &vh, &vb, &context, &options()).unwrap();
    vh[0x820 + 22..0x820 + 24].copy_from_slice(&0u16.to_le_bytes());
    let alias = binding::bind(identity(), &vh, &vb, &context, &options()).unwrap();
    assert_eq!(alias.bytes, baseline.bytes);
    let tone = &alias.report.tones[0];
    assert_eq!(tone.source_tone.sample_reference, 0);
    assert_eq!(tone.sample_resolution.encoded_reference, 0);
    assert_eq!(tone.sample_resolution.selected_byte, 0);
    assert_eq!(tone.sample_resolution.sample_id, 2);
    assert_eq!(
        tone.source_sf2_sample,
        alias.report.samples[1].sf2_sample.unwrap()
    );
    assert_eq!(alias.report.samples[0].sf2_sample, None);
}

#[test]
fn changing_predictor_history_requires_explicit_loop_approximation() {
    let (vh, mut vb) = fixture();
    vb[0] = 0x10;
    vb[2..16].fill(0x11);
    let context = contexts(&vh);
    assert!(binding::bind(identity(), &vh, &vb, &context, &options())
        .err()
        .unwrap()
        .to_string()
        .contains("approximation policy"));
    let mut options = options();
    options.allow_predictor_loop_approximation = true;
    let bound = binding::bind(identity(), &vh, &vb, &context, &options).unwrap();
    assert!(bound.report.samples[1].predictor_loop_approximation);
    assert_eq!(bound.report.samples[1].loop_range, Some((0, 112)));
}

#[test]
fn verified_program_mode_and_explicit_tone_reverb_policy_keep_source_identity() {
    use bof3_audio::{soundfont::bank::ReverbApproximation, soundfont::gain};
    let (mut vh, vb) = fixture();
    vh[0x23] = 255;
    vh[0x820 + 1] = 4;
    let metadata = Bank::parse(&vh).unwrap();
    let mut context = contexts(&vh);
    for program in &metadata.programs {
        for tone in &program.tones {
            let row = context
                .iter_mut()
                .find(|r| r.program == program.program && r.tone == tone.index)
                .unwrap();
            row.gain =
                Gain::from_bank(&metadata, program, tone, gain::Model::SpecificationScale).unwrap();
            assert!(row.gain.fit.as_ref().unwrap().max_absolute_error < 0.0001);
        }
    }
    let mut opts = options();
    assert!(binding::bind(identity(), &vh, &vb, &context, &opts)
        .err()
        .unwrap()
        .to_string()
        .contains("reverb approximation"));
    opts.reverb = Some(ReverbApproximation {
        send_tenths_percent: 1000,
        provenance: "Synthetic full-send approximation; not the game's reverb preset/depth".into(),
    });
    let bound = binding::bind(identity(), &vh, &vb, &context, &opts).unwrap();
    assert_eq!(bound.report.source_metadata.programs[0].mode, 255);
    assert_eq!(bound.report.tones[0].source_tone.mode, 4);
    assert_eq!(
        bound
            .report
            .reverb_approximation
            .unwrap()
            .send_tenths_percent,
        1000
    );
    assert!(bound.soundfont.instruments[0]
        .zones
        .iter()
        .all(|z| z.reverb_send == 1000));
    assert!(bound.soundfont.instruments[1]
        .zones
        .iter()
        .all(|z| z.reverb_send == 0));
    opts.reverb.as_mut().unwrap().send_tenths_percent = 1001;
    assert!(binding::bind(identity(), &vh, &vb, &context, &opts).is_err());
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE; gain remains a synthetic context"]
fn original_pitch_context_binds_every_key_without_changing_source_identities() {
    use bof3_audio::{machine::executable::Executable, voice::tuning::Reference};
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut reference = Reference::from_executable(&exe).unwrap();
    let (vh, vb) = fixture();
    let mut context = contexts(&vh);
    for program in Bank::parse(&vh).unwrap().programs {
        for tone in program.tones {
            let item = context
                .iter_mut()
                .find(|c| c.program == program.program && c.tone == tone.index)
                .unwrap();
            *item = ToneContext::from_reference(
                &mut reference,
                program.program,
                &tone,
                44100,
                item.gain.clone(),
                item.envelope.clone(),
            )
            .unwrap();
        }
    }
    let bound = binding::bind(identity(), &vh, &vb, &context, &options()).unwrap();
    for tone in bound.report.tones {
        assert!(tone.context.pitch_provenance.contains("exe/slus_004_22"));
        for row in tone.context.tuning {
            assert!(row.sf2.unwrap().error_cents.abs() <= 0.50000001);
        }
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS; static SF2 gain/reverb approximation, not full playback acceptance"]
fn corpus_binding_reports_supported_structure_and_rejects_unmapped_contexts() {
    use bof3_audio::{
        archive::MediaImage, catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
        machine::executable::Executable, voice::tuning::Reference,
    };
    use std::{collections::BTreeMap, io::Cursor, path::PathBuf};
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut reference = Reference::from_executable(&exe).unwrap();
    let mut catalog = Catalog::read(Some(&root), &[]).unwrap();
    loader::resolve(&mut catalog, &exe).unwrap();
    let mut fits = BTreeMap::new();
    let mut accepted = 0;
    let mut rejected = BTreeMap::<String, usize>::new();
    let mut zones = 0;
    let mut silent_tones = 0;
    let mut silent_banks = 0;
    let mut zero_aliases = 0;
    let mut edited_gain_banks = 0;
    let mut edited_pitch_banks = 0;
    let mut reassigned_banks = 0;
    let mut unmuted_banks = 0;
    let mut test_options = options();
    test_options.allow_predictor_loop_approximation = true;
    test_options.reverb = Some(binding::ReverbApproximation {
        send_tenths_percent: 1000,
        provenance: "Corpus inspection: full SF2 send for enabled tones; game reverb preset/depth and PCM equivalence remain unverified".into(),
    });
    'banks: for asset in &catalog.assets {
        let AssetData::Bank {
            content, metadata, ..
        } = &asset.data
        else {
            continue;
        };
        let content = content.as_ref().unwrap();
        let source = catalog
            .sources
            .iter()
            .find(|s| s.source == asset.source)
            .unwrap();
        let body_entry = source
            .entries
            .iter()
            .find(|e| e.id == content.body_entry)
            .unwrap()
            .entry;
        let MediaImage::Emi(image) = MediaImage::read(&root.join(&asset.source)).unwrap() else {
            unreachable!()
        };
        let mut contexts = Vec::new();
        for program in &metadata.programs {
            for tone in &program.tones {
                let pair = (tone.adsr1, tone.adsr2);
                if let std::collections::btree_map::Entry::Vacant(entry) = fits.entry(pair) {
                    entry.insert(
                        envelope::fit(
                            Registers {
                                adsr1: pair.0,
                                adsr2: pair.1,
                            },
                            Model::Published,
                            &Probe::default(),
                        )
                        .unwrap(),
                    );
                }
                let gain = match Gain::from_bank(
                    metadata,
                    program,
                    tone,
                    bof3_audio::soundfont::gain::Model::SpecificationScale,
                ) {
                    Ok(gain) => gain,
                    Err(_) => {
                        *rejected.entry("gain_range".into()).or_default() += 1;
                        continue 'banks;
                    }
                };
                contexts.push(
                    ToneContext::from_reference(
                        &mut reference,
                        program.program,
                        tone,
                        44100,
                        gain,
                        fits[&pair].clone(),
                    )
                    .unwrap(),
                );
            }
        }
        let identity = Identity {
            source: asset.source.clone(),
            header_entry: asset.entry.unwrap(),
            body_entry,
            game_bank_id: u32::from(asset.game_bank_id.unwrap()),
        };
        match binding::bind(
            identity,
            image.entry(asset.entry.unwrap()).unwrap(),
            image.entry(body_entry).unwrap(),
            &contexts,
            &test_options,
        ) {
            Ok(bound) => {
                let consumer = rustysynth::SoundFont::new(&mut Cursor::new(&bound.bytes)).unwrap();
                let parsed =
                    bof3_audio::soundfont::reader::Font::from_bytes(bound.bytes.clone()).unwrap();
                assert_eq!(parsed.original_bytes(), bound.bytes);
                assert_eq!(parsed.presets.len(), consumer.get_presets().len());
                assert_eq!(parsed.instruments.len(), consumer.get_instruments().len());
                assert_eq!(parsed.samples.len(), bound.soundfont.samples.len());
                let reconstructed = bof3_audio::soundfont::packing::sample::pack_bank(
                    &bound.report,
                    image.entry(asset.entry.unwrap()).unwrap(),
                    image.entry(body_entry).unwrap(),
                    &parsed,
                    &parsed,
                    None,
                )
                .unwrap();
                assert_eq!(
                    reconstructed.header,
                    image.entry(asset.entry.unwrap()).unwrap()
                );
                assert!(reconstructed.tone_controls.tones.iter().all(|t| !t.changed));
                assert_eq!(reconstructed.body, image.entry(body_entry).unwrap());
                edited_gain_banks += usize::from(tone_corpus::check(
                    &bound,
                    image.entry(asset.entry.unwrap()).unwrap(),
                    image.entry(body_entry).unwrap(),
                    &test_options,
                ));
                reassigned_banks += usize::from(assignment_corpus::check(
                    &bound,
                    image.entry(asset.entry.unwrap()).unwrap(),
                    image.entry(body_entry).unwrap(),
                    &test_options,
                ));
                unmuted_banks += usize::from(unmute_corpus::check(
                    &bound,
                    image.entry(asset.entry.unwrap()).unwrap(),
                    image.entry(body_entry).unwrap(),
                    &test_options,
                ));
                edited_pitch_banks += usize::from(pitch_corpus::check(
                    &bound,
                    image.entry(asset.entry.unwrap()).unwrap(),
                    image.entry(body_entry).unwrap(),
                    &test_options,
                    &mut reference,
                ));
                assert!(reconstructed
                    .report
                    .samples
                    .iter()
                    .all(|s| !s.content_changed && !s.encoded_changed));
                for (index, sample) in bound.soundfont.samples.iter().enumerate() {
                    assert_eq!(
                        parsed.sample_pcm24(index).unwrap(),
                        sample
                            .pcm
                            .iter()
                            .map(|value| i32::from(*value) * 256)
                            .collect::<Vec<_>>()
                    );
                }
                assert_eq!(consumer.get_presets().len(), 256);
                assert_eq!(
                    bound.report.tones.len(),
                    metadata
                        .programs
                        .iter()
                        .map(|p| p.tones.len())
                        .sum::<usize>()
                );
                for tone in &bound.report.tones {
                    assert_eq!(
                        tone.zone_count,
                        usize::from(tone.source_tone.key_max - tone.source_tone.key_min) + 1
                    );
                    let source =
                        &bound.report.samples[usize::from(tone.sample_resolution.sample_id) - 1];
                    assert_eq!(source.sf2_sample, Some(tone.source_sf2_sample));
                    zero_aliases += usize::from(tone.sample_resolution.selected_byte == 0);
                    if tone.silent {
                        assert_eq!(Some(tone.sf2_sample), bound.report.silence_sample);
                        assert!(tone.context.gain.fit.as_ref().unwrap().parameters.is_none());
                        silent_tones += 1;
                    } else {
                        assert_eq!(tone.sf2_sample, tone.source_sf2_sample);
                    }
                }
                if let Some(sample) = bound.report.silence_sample {
                    silent_banks += 1;
                    assert!(bound.soundfont.samples[sample].pcm.iter().all(|&v| v == 0));
                }
                let mut offset = 0;
                for sample in &bound.soundfont.samples {
                    assert_eq!(
                        &consumer.get_wave_data()[offset..offset + sample.pcm.len()],
                        &sample.pcm
                    );
                    offset += sample.pcm.len() + 46;
                }
                accepted += 1;
                zones += bound
                    .report
                    .tones
                    .iter()
                    .map(|t| t.zone_count)
                    .sum::<usize>();
            }
            Err(error) => {
                let message = error.to_string();
                let category = if message.contains("mode") {
                    "mode"
                } else if message.contains("pitch table") {
                    "pitch table"
                } else if message.contains("zero SPU") {
                    "zero pitch"
                } else if message.contains("tuning exceeds supported generator range") {
                    "tuning range"
                } else if message.contains("empty allocation") {
                    "empty referenced sample"
                } else if message.contains("invalid sample reference") {
                    "invalid sample reference"
                } else {
                    panic!("{}: unexpected binding rejection: {message}", asset.id)
                };
                *rejected.entry(category.into()).or_default() += 1;
            }
        }
    }
    assert_eq!(accepted + rejected.values().sum::<usize>(), 1020);
    assert_eq!((accepted, zones), (842, 15095));
    assert_eq!(
        rejected,
        BTreeMap::from([("pitch table".into(), 137), ("zero pitch".into(), 41),])
    );
    eprintln!("Corpus bank binding with static game gain fits and explicit full reverb send: {accepted} structurally accepted banks / {zones} zones; rejected={rejected:?}. Dynamic controls, reverb fidelity, rejected contexts and full bank/song acceptance remain unverified.");
    eprintln!("Explicit silent playback: {silent_tones} tones in {silent_banks} banks; original PCM preserved and checked through consumer sample data.");
    eprintln!("Resolved zero sample aliases in constructed banks: {zero_aliases}");
    assert!(edited_gain_banks > 0);
    assert!(edited_pitch_banks > 0);
    assert!(reassigned_banks > 0);
    assert!(unmuted_banks > 0);
    eprintln!("Silent-tone inverse: {unmuted_banks} banks unmute one eligible tone with exact semantic re-export and unchanged body; {} banks have no eligible muted tone under this probe.", accepted-unmuted_banks);
    eprintln!("Sample assignment inverse: {reassigned_banks} banks reassign one eligible nonsilent tone to another existing PCM allocation with exact re-export and unchanged body; {} banks have no eligible target under this probe.",accepted-reassigned_banks);
    eprintln!("Tone pitch inverse: {edited_pitch_banks} banks with an eligible center edit preserve requested note-on cents; {} banks had no eligible edit under this probe.", accepted-edited_pitch_banks);
    eprintln!("Tone gain/pan inverse: {edited_gain_banks} banks with an eligible changed nonsilent tone reconstruct the requested SF2 bytes and pass independent consumer parsing; {} banks had no eligible edit under this probe.", accepted - edited_gain_banks);
    // This is a coverage/classification probe, not a full-bank acceptance gate.
    // Rejected contexts stay explicit; structural construction of other banks
    // does not prove complete song translation or PSX/PC audio fidelity.
}
