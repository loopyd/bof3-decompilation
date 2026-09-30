use bof3_audio::{
    bank::Bank, codec::adpcm, machine::adsr::Model, machine::adsr::Registers,
    soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::Gain,
    soundfont::bank::Identity, soundfont::bank::Options, soundfont::bank::ToneContext,
    soundfont::envelope, soundfont::envelope::Probe, soundfont::gain, voice::tuning::KeyTuning,
};
use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
use std::{io::Cursor, sync::Arc};

fn fixture() -> (Vec<u8>, Vec<u8>, Vec<ToneContext>) {
    let mut vh = vec![0; 0x820 + 512 + 512];
    vh[..4].copy_from_slice(b"pBAV");
    vh[4..8].copy_from_slice(&7u32.to_le_bytes());
    for (offset, value) in [(0x12, 1u16), (0x14, 2), (0x16, 2)] {
        vh[offset..offset + 2].copy_from_slice(&value.to_le_bytes());
    }
    vh[0x18] = 127;
    vh[0x19] = 64;
    vh[0x20] = 2;
    vh[0x21] = 127;
    vh[0x24] = 64;
    for i in 0..2 {
        let t = 0x820 + i * 32;
        vh[t + 3] = 64;
        vh[t + 4] = 60;
        vh[t + 6] = 60;
        vh[t + 7] = 60;
        vh[t + 16..t + 18].copy_from_slice(&0x000fu16.to_le_bytes());
        vh[t + 18..t + 20].copy_from_slice(&0x1fc0u16.to_le_bytes());
        vh[t + 22..t + 24].copy_from_slice(&(i as u16 + 1).to_le_bytes());
        let table = 0xa20 + 2 * (i + 1);
        vh[table..table + 2].copy_from_slice(&4u16.to_le_bytes());
    }
    let mut vb = vec![0; 64];
    for (i, block) in vb.chunks_mut(16).enumerate() {
        block[1] = if i % 2 == 0 { 4 } else { 3 };
        block[2..].fill(if i < 2 { 0x11 } else { 0x77 });
    }
    let bank = Bank::parse(&vh).unwrap();
    let envelope = envelope::fit(
        Registers {
            adsr1: 0x000f,
            adsr2: 0x1fc0,
        },
        Model::Published,
        &Probe {
            held_frames: vec![4410],
            release_frames: 4410,
            sample_stride: 441,
        },
    )
    .unwrap();
    let program = &bank.programs[0];
    let context = program
        .tones
        .iter()
        .map(|tone| ToneContext {
            program: 0,
            tone: tone.index,
            gain: Gain::from_bank(&bank, program, tone, gain::Model::SpecificationScale).unwrap(),
            envelope: envelope.clone(),
            tuning: vec![KeyTuning::from_register(60, 60, 0, 4096, 44100).unwrap()],
            pitch_provenance: "Synthetic unity-pitch context".into(),
        })
        .collect();
    (vh, vb, context)
}

fn bind(vh: &[u8], vb: &[u8], contexts: &[ToneContext]) -> bof3_audio::Result<Bound> {
    binding::bind(
        Identity {
            source: "synthetic/SILENCE.EMI".into(),
            header_entry: 0,
            body_entry: 1,
            game_bank_id: 3,
        },
        vh,
        vb,
        contexts,
        &Options {
            sf2_bank: 0,
            percussion_alias: false,
            sample_rate: 44100,
            allow_predictor_loop_approximation: false,
            allow_stopped_pitch_approximation: false,
            reverb: None,
        },
    )
}

fn play(bytes: &[u8], velocity: i32, volume: i32, pan: i32) -> Vec<f32> {
    let font = SoundFont::new(&mut Cursor::new(bytes)).unwrap();
    let mut settings = SynthesizerSettings::new(44100);
    settings.block_size = 8;
    let mut synth = Synthesizer::new(&Arc::new(font), &settings).unwrap();
    synth.set_master_volume(1.0);
    for (cc, value) in [
        (7, volume),
        (39, 127),
        (11, 127),
        (43, 127),
        (10, pan),
        (91, 127),
        (64, 127),
    ] {
        synth.process_midi_message(0, 0xb0, cc, value);
    }
    synth.note_on(0, 60, velocity);
    let mut left = vec![0.0; 4410];
    let mut right = vec![0.0; 4410];
    synth.render(&mut left, &mut right);
    synth.note_off(0, 60);
    synth.process_midi_message(0, 0xb0, 64, 0);
    let mut tail_left = vec![0.0; 4410];
    let mut tail_right = vec![0.0; 4410];
    synth.render(&mut tail_left, &mut tail_right);
    left.extend(right);
    left.extend(tail_left);
    left.extend(tail_right);
    left
}

#[test]
fn silent_tones_preserve_original_samples_and_play_exact_zero_with_live_controls() {
    let (vh, vb, contexts) = fixture();
    let bound = bind(&vh, &vb, &contexts).unwrap();
    assert_eq!(bound.report.samples.len(), 2);
    assert_eq!(bound.report.tones.len(), 2);
    assert_eq!(bound.soundfont.samples.len(), 4);
    let silence = bound.report.silence_sample.unwrap();
    assert_eq!(silence, 2);
    assert!(bound.soundfont.samples[silence].pcm.iter().all(|&v| v == 0));
    for (i, tone) in bound.report.tones.iter().enumerate() {
        assert!(tone.silent && tone.context.gain.is_silent());
        assert_eq!(tone.source_tone.sample_reference, i as i16 + 1);
        assert_eq!(tone.source_sf2_sample, i);
        assert_eq!(tone.sf2_sample, silence);
        assert_eq!(tone.zone_count, 1);
        assert_eq!(bound.report.samples[i].sf2_sample, Some(i));
        assert_eq!(
            bound.soundfont.samples[i].pcm,
            adpcm::decode_sample(&vb[i * 32..i * 32 + 32]).unwrap().pcm
        );
        assert!(bound.soundfont.samples[i].pcm.iter().any(|&v| v != 0));
    }
    for velocity in [1, 64, 127] {
        for volume in [0, 64, 127] {
            for pan in [0, 64, 127] {
                assert!(play(&bound.bytes, velocity, volume, pan)
                    .iter()
                    .all(|&v| v == 0.0));
            }
        }
    }
    // The authored SF2 retains original PCM: relinking one zone restores it.
    // This proves editability of the artifact, not an implemented game packer.
    let mut edited = bound.soundfont;
    edited.instruments[0].zones[0].sample = 0;
    let bytes = edited.encode().unwrap().bytes;
    assert!(play(&bytes, 127, 127, 64).iter().any(|&v| v.abs() > 0.001));
}

#[test]
fn forged_or_temporary_silence_and_unsupported_source_contexts_are_rejected() {
    let (vh, vb, contexts) = fixture();
    let mut changed = contexts.clone();
    changed[0].gain.fit.as_mut().unwrap().source.channel_volume = 0;
    assert!(bind(&vh, &vb, &changed)
        .err()
        .unwrap()
        .to_string()
        .contains("temporary"));
    let mut changed = contexts.clone();
    changed[0].gain.fit.as_mut().unwrap().source.tone_volume = 127;
    assert!(bind(&vh, &vb, &changed)
        .err()
        .unwrap()
        .to_string()
        .contains("does not match"));
    let mut changed = contexts.clone();
    changed[0].gain.pan = 500;
    assert!(bind(&vh, &vb, &changed)
        .err()
        .unwrap()
        .to_string()
        .contains("conflict"));
    let mut changed = contexts.clone();
    changed[0].tuning[0].sf2 = None;
    assert!(bind(&vh, &vb, &changed).is_err());
    let mut invalid = vh.clone();
    invalid[0x820 + 22..0x820 + 24].copy_from_slice(&3u16.to_le_bytes());
    assert!(bind(&invalid, &vb, &contexts)
        .err()
        .unwrap()
        .to_string()
        .contains("sample reference"));
}

#[test]
fn transferred_prefix_is_excluded_from_samples_without_changing_their_pcm() {
    let (mut vh, vb, contexts) = fixture();
    let baseline = bind(&vh, &vb, &contexts).unwrap();
    vh[0xa20..0xa22].copy_from_slice(&2u16.to_le_bytes());
    let mut prefixed = vec![0xaa; 16];
    prefixed.extend(&vb);
    let bound = bind(&vh, &prefixed, &contexts).unwrap();
    assert_eq!(bound.report.source_metadata.body_prefix_bytes, 16);
    assert_eq!(bound.report.samples[0].body_offset, 16);
    assert_eq!(bound.report.samples[1].body_offset, 48);
    assert_ne!(bound.report.body_sha256, baseline.report.body_sha256);
    assert_eq!(bound.bytes, baseline.bytes);
}
