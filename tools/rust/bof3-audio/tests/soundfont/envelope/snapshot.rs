use crate::fixture as common;
use bof3_audio::{
    soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::ToneContext,
    soundfont::packing::sample, soundfont::reader::Font, voice::tuning::KeyTuning,
};

fn fixture() -> (Vec<u8>, Vec<u8>, Vec<ToneContext>, binding::Options) {
    let (header, mut body) = common::fixture();
    body[2] &= 0xf0; // First decoded sample is zero; following PCM is audible.
    let mut contexts = common::contexts(&header);
    for tone in contexts.iter_mut().filter(|t| t.program == 0) {
        tone.tuning[0] = KeyTuning::from_register(60, 60, 0, 0, 44100).unwrap();
    }
    let mut options = common::options();
    options.allow_predictor_loop_approximation = true;
    options.allow_stopped_pitch_approximation = true;
    (header, body, contexts, options)
}
fn bind(header: &[u8], body: &[u8], contexts: &[ToneContext], options: &binding::Options) -> Bound {
    binding::bind(common::identity(), header, body, contexts, options).unwrap()
}
fn play(bytes: &[u8], key: i32, bend: bool) -> Vec<f32> {
    use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
    use std::{io::Cursor, sync::Arc};
    let font = Arc::new(SoundFont::new(&mut Cursor::new(bytes)).unwrap());
    let mut settings = SynthesizerSettings::new(44100);
    settings.enable_reverb_and_chorus = false;
    let mut synth = Synthesizer::new(&font, &settings).unwrap();
    synth.note_on(0, key, 127);
    let mut left = vec![0.0; 4410];
    let mut right = left.clone();
    synth.render(&mut left, &mut right);
    if bend {
        synth.process_midi_message(0, 0xe0, 127, 127);
    }
    synth.render(&mut left, &mut right);
    left.extend(right);
    left
}

#[test]
fn stopped_key_mapping_requires_opt_in_retains_pcm_and_preserves_unchanged_pack() {
    let (header, body, contexts, mut options) = fixture();
    options.allow_stopped_pitch_approximation = false;
    assert!(binding::bind(common::identity(), &header, &body, &contexts, &options).is_err());
    options.allow_stopped_pitch_approximation = true;
    let bound = bind(&header, &body, &contexts, &options);
    assert_eq!(bound.report.tones[0].stopped_keys, [60]);
    assert_eq!(bound.report.tones[1].stopped_keys, [60]);
    assert!(bound.report.tones[2].stopped_keys.is_empty());
    let original = bound.report.tones[0].source_sf2_sample;
    assert!(bound.soundfont.samples[original]
        .pcm
        .iter()
        .any(|&s| s != 0));
    assert!(play(&bound.bytes, 60, false).iter().all(|&s| s == 0.0));
    assert!(play(&bound.bytes, 60, true).iter().all(|&s| s == 0.0));
    assert!(play(&bound.bytes, 61, false).iter().any(|&s| s != 0.0));
    assert!(bound
        .report
        .diagnostics
        .iter()
        .any(|s| s.contains("Later pitch changes can activate")));
    let font = Font::from_bytes(bound.bytes.clone()).unwrap();
    let packed = sample::pack_bank(&bound.report, &header, &body, &font, &font, None).unwrap();
    assert_eq!(packed.header, header);
    assert_eq!(packed.body, body);
    assert!(packed.report.soundfont_byte_equal);
    // Changing a running PSX voice from zero to a positive step can activate it.
    // The explicitly reported static SF2 approximation above stays silent.
    use bof3_audio::machine::{
        spu_sample::{Model, Player},
        spu_transfer::RAM_BYTES,
    };
    let mut ram = vec![0; RAM_BYTES];
    ram[..body.len()].copy_from_slice(&body);
    let mut player = Player::new(Model::EmulatorReference);
    player.key_on(0);
    for _ in 0..100 {
        assert_eq!(player.tick(&ram, 0, None).unwrap().sample, 0);
    }
    assert!((0..20).any(|_| player.tick(&ram, 4096, None).unwrap().sample != 0));
}

#[test]
fn stopped_key_edits_reject_activation_reassignment_and_nonzero_held_samples() {
    let (header, body, contexts, options) = fixture();
    let bound = bind(&header, &body, &contexts, &options);
    let baseline = Font::from_bytes(bound.bytes.clone()).unwrap();
    let source = bound.report.tones[0].source_sf2_sample;
    let pack = |edited: &bof3_audio::soundfont::Bank| {
        sample::pack_bank(
            &bound.report,
            &header,
            &body,
            &baseline,
            &Font::from_bytes(edited.encode().unwrap().bytes).unwrap(),
            None,
        )
    };
    let mut edited = bound.soundfont.clone();
    edited.instruments[0].zones[0].sample = source;
    assert!(pack(&edited)
        .err()
        .unwrap()
        .to_string()
        .contains("stopped-pitch tones"));
    let mut edited = bound.soundfont.clone();
    edited.instruments[0].zones[0].coarse_tune = 1;
    assert!(pack(&edited)
        .err()
        .unwrap()
        .to_string()
        .contains("stopped-pitch tones"));
    let mut edited = bound.soundfont.clone();
    edited.samples[bound.report.silence_sample.unwrap()].pcm[15] = 1;
    assert!(pack(&edited).is_err());
    let mut edited = bound.soundfont.clone();
    edited.samples[source].pcm[0] = 10000;
    assert!(pack(&edited)
        .err()
        .unwrap()
        .to_string()
        .contains("held value"));
    let mut edited = bound.soundfont.clone();
    edited.samples[source].pcm[20] = 2000;
    let packed = pack(&edited).unwrap();
    assert_ne!(packed.body, body);
    let rebuilt = bind(&header, &packed.body, &contexts, &options);
    assert!(play(&rebuilt.bytes, 60, false).iter().all(|&s| s == 0.0));
}

#[test]
fn opt_in_does_not_accept_unverified_lookup_or_nonzero_held_pcm() {
    use bof3_audio::{voice::tuning::Lookup, voice::tuning::LookupRegion};
    let (header, body, mut contexts, options) = fixture();
    for nibble in [7, 8] {
        let mut nonzero = body.clone();
        nonzero[2] |= nibble;
        assert!(
            binding::bind(common::identity(), &header, &nonzero, &contexts, &options)
                .err()
                .unwrap()
                .to_string()
                .contains("zero held key-on sample")
        );
    }
    contexts[0].tuning[0].pitch_lookup = Some(Lookup {
        runtime_address: 0x80184440,
        source_file_offset: 0xee440,
        value: 5,
        region: LookupRegion::AdjacentData,
        execution_state: "synthetic unverified lookup",
    });
    assert!(binding::bind(common::identity(), &header, &body, &contexts, &options).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn corpus_stopped_note_approximation_preserves_all_constructible_bank_bytes() {
    use bof3_audio::{
        archive::MediaImage, catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
        machine::adsr::Model, machine::adsr::Registers, machine::executable::Executable,
        soundfont::envelope, soundfont::envelope::Probe, soundfont::gain, voice::tuning::Reference,
    };
    use std::{
        collections::{BTreeMap, BTreeSet},
        fs,
        path::PathBuf,
    };
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut reference = Reference::from_executable(&exe).unwrap();
    let mut catalog = Catalog::read(Some(&root), &[]).unwrap();
    loader::resolve(&mut catalog, &exe).unwrap();
    let mut envelopes = BTreeMap::new();
    let mut options = common::options();
    options.allow_predictor_loop_approximation = true;
    options.allow_stopped_pitch_approximation = true;
    options.reverb = Some(binding::ReverbApproximation {
        send_tenths_percent: 0,
        provenance: "explicit dry corpus inspection".into(),
    });
    let mut accepted = 0;
    let mut stopped_keys = 0;
    let mut stopped_banks = BTreeSet::new();
    let mut failures = BTreeMap::new();
    for asset in &catalog.assets {
        let AssetData::Bank {
            metadata, content, ..
        } = &asset.data
        else {
            continue;
        };
        let source = catalog
            .sources
            .iter()
            .find(|s| s.source == asset.source)
            .unwrap();
        let body_entry = source
            .entries
            .iter()
            .find(|e| e.id == content.as_ref().unwrap().body_entry)
            .unwrap()
            .entry;
        let MediaImage::Emi(image) = MediaImage::read(&root.join(&asset.source)).unwrap() else {
            unreachable!()
        };
        let header = image.entry(asset.entry.unwrap()).unwrap();
        let body = image.entry(body_entry).unwrap();
        let mut contexts = Vec::new();
        for program in &metadata.programs {
            for tone in &program.tones {
                let envelope = envelopes
                    .entry((tone.adsr1, tone.adsr2))
                    .or_insert_with(|| {
                        envelope::fit(
                            Registers {
                                adsr1: tone.adsr1,
                                adsr2: tone.adsr2,
                            },
                            Model::Published,
                            &Probe::default(),
                        )
                        .unwrap()
                    });
                let gain = binding::Gain::from_bank(
                    metadata,
                    program,
                    tone,
                    gain::Model::SpecificationScale,
                )
                .unwrap();
                contexts.push(
                    ToneContext::from_reference(
                        &mut reference,
                        program.program,
                        tone,
                        44100,
                        gain,
                        envelope.clone(),
                    )
                    .unwrap(),
                );
            }
        }
        let bound = match binding::bind(
            binding::Identity {
                source: asset.source.clone(),
                header_entry: asset.entry.unwrap(),
                body_entry,
                game_bank_id: u32::from(asset.game_bank_id.unwrap()),
            },
            header,
            body,
            &contexts,
            &options,
        ) {
            Ok(bound) => bound,
            Err(error) => {
                assert!(
                    error.to_string().contains("pitch table"),
                    "{}: {error}",
                    asset.id
                );
                *failures.entry("pitch table").or_insert(0usize) += 1;
                continue;
            }
        };
        let count = bound
            .report
            .tones
            .iter()
            .map(|t| t.stopped_keys.len())
            .sum::<usize>();
        stopped_keys += count;
        if count > 0 {
            stopped_banks.insert(asset.id.clone());
        }
        rustysynth::SoundFont::new(&mut std::io::Cursor::new(&bound.bytes)).unwrap();
        let font = Font::from_bytes(bound.bytes.clone()).unwrap();
        let packed = sample::pack_bank(&bound.report, header, body, &font, &font, None).unwrap();
        assert_eq!(packed.header, header, "{}", asset.id);
        assert_eq!(packed.body, body, "{}", asset.id);
        accepted += 1;
    }
    eprintln!("Stopped corpus: accepted={accepted}, keys={stopped_keys}, banks={stopped_banks:?}, failures={failures:?}");
    assert_eq!(accepted, 843);
    assert_eq!(failures, BTreeMap::from([("pitch table", 177)]));
    assert_eq!(stopped_keys, 4);
    assert_eq!(stopped_banks.len(), 1);
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn stopped_music_cli_preserves_xml_key_mapping_and_whole_archive_round_trip() {
    use std::{fs, process::Command};
    let root = std::env::temp_dir().join(format!("bof3-stopped-music-{}", std::process::id()));
    fs::create_dir(&root).unwrap();
    let archive = source_archive();
    let input = root.join("edited.emi");
    fs::write(&input, archive.bytes()).unwrap();
    let extracted = root.join("extracted");
    let exe = std::env::var_os("BOF3_AUDIO_EXE").unwrap();
    let strict = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["extract", "--mode", "music", "--archive"])
        .arg(&input)
        .arg("--executable")
        .arg(&exe)
        .arg("--output")
        .arg(&extracted)
        .output()
        .unwrap();
    assert!(!strict.status.success());
    assert!(String::from_utf8_lossy(&strict.stderr).contains("allow-approximations"));
    assert!(!extracted.exists());
    let extract = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args([
            "extract",
            "--mode",
            "music",
            "--allow-approximations",
            "--archive",
        ])
        .arg(&input)
        .arg("--executable")
        .arg(&exe)
        .arg("--output")
        .arg(&extracted)
        .arg("--json")
        .output()
        .unwrap();
    assert!(
        extract.status.success(),
        "{}",
        String::from_utf8_lossy(&extract.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&extract.stdout).unwrap();
    assert!(report["songs"][0]["playback_approximations"]
        .as_array()
        .unwrap()
        .iter()
        .any(|s| s.as_str().unwrap().contains("stopped-pitch keys [108]")));
    let song = extracted.join(report["songs"][0]["path"].as_str().unwrap());
    let xml = fs::read_to_string(song).unwrap();
    assert!(xml.contains("<stopped_key tone=\"0\" key=\"108\""));
    assert!(xml.contains("scope=\"key_on_only\""));
    let output = root.join("packed");
    let pack = Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["pack", "--mode", "music", "--input"])
        .arg(&extracted)
        .arg("--executable")
        .arg(&exe)
        .arg("--output")
        .arg(&output)
        .arg("--json")
        .output()
        .unwrap();
    assert!(
        pack.status.success(),
        "{}",
        String::from_utf8_lossy(&pack.stderr)
    );
    let report: serde_json::Value = serde_json::from_slice(&pack.stdout).unwrap();
    assert_eq!(report["archives"][0]["byte_equal"], true);
    assert_eq!(
        fs::read(output.join(report["archives"][0]["path"].as_str().unwrap())).unwrap(),
        archive.bytes()
    );
    fs::remove_dir_all(root).unwrap();
}

fn source_archive() -> emi_ex_v2::image::ArchiveImage {
    use bof3_audio::bank::Bank;
    use emi_ex_v2::image::ArchiveImage;
    use std::{fs, path::PathBuf};
    let corpus = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let archive =
        ArchiveImage::from_bytes(fs::read(corpus.join("BIN/BGM/BGM000.EMI")).unwrap()).unwrap();
    let mut header = archive.entry(0).unwrap().to_vec();
    let bank = Bank::parse(&header).unwrap();
    // Valid edited source fixture: one stopped key, other tone contexts in the
    // declared pitch table. Archive allocation and original PCM stay intact.
    for program in &bank.programs {
        for tone in &program.tones {
            let at = 0x820 + program.tone_block * 512 + tone.index * 32;
            header[at + 4..at + 8].copy_from_slice(&[60, 0, 60, 60]);
        }
    }
    header[0x826..0x828].copy_from_slice(&[108, 108]);
    archive.replace_entries(&[(0, header.as_slice())]).unwrap()
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn original_runtime_pitch_bend_activates_a_note_that_started_at_zero_step() {
    use bof3_audio::{
        machine::{executable::Executable, firmware::Image},
        psx_render::{self, Body, Options},
    };
    use std::fs;
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let bios = fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    let source = source_archive();
    let mut header = source.entry(0).unwrap().to_vec();
    header[0x82c..0x82e].copy_from_slice(&[12, 12]);
    // Hold the edited test voice through the bend; the original percussion
    // envelope can finish before that event, independently of pitch.
    header[0x830..0x834].copy_from_slice(&[0x0f, 0, 0xc0, 0x1f]);
    let mut results = Vec::new();
    for bend in [false, true] {
        let mut events = vec![0, 0xc0, 0, 0, 0x90, 108, 127];
        // Downward octave changes the wrapped zero register to a nonzero step.
        // Upward octave also wraps to zero and cannot demonstrate activation.
        if bend {
            events.extend([48, 0xe0, 0, 0]);
        }
        events.extend([96, 0x90, 108, 0, 0, 0xff, 0x2f]);
        let mut sep = source.entry(1).unwrap()[..19].to_vec();
        sep[8..10].copy_from_slice(&96u16.to_be_bytes());
        sep[10..13].copy_from_slice(&[7, 0xa1, 0x20]);
        sep[15..19].copy_from_slice(&(events.len() as u32).to_be_bytes());
        sep.extend(events);
        sep.resize(source.entry(1).unwrap().len(), 0);
        let archive = source
            .replace_entries(&[(0, header.as_slice()), (1, sep.as_slice())])
            .unwrap();
        let rendered = psx_render::render(
            &exe,
            Image::from_bytes(bios.clone()).unwrap(),
            &archive,
            &Options {
                sequence: 0,
                layout: None,
                body: Body::Duration(33075),
                release_frames: 0,
                safety_frames: 44100,
            },
        )
        .unwrap();
        assert!(rendered.wave.pcm[..4410 * 2].iter().all(|&v| v == 0));
        results.push(rendered.report.peak_magnitude);
        eprintln!(
            "bend={bend} peak={} first audible frame={:?}",
            rendered.report.peak_magnitude,
            rendered
                .wave
                .pcm
                .iter()
                .position(|&v| v != 0)
                .map(|i| i / 2)
        );
        if bend {
            assert!(rendered.wave.pcm[11025 * 2..].iter().any(|&v| v != 0));
        } else {
            assert!(rendered.wave.pcm.iter().all(|&v| v == 0));
        }
    }
    eprintln!("Original stopped-note PCM peaks without/with bend: {results:?}");
}
