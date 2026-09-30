use bof3_audio::{
    soundfont::gain, soundfont::gain::Model, soundfont::gain::Parameters, soundfont::Bank,
    soundfont::Instrument, soundfont::LoopMode, soundfont::Preset, soundfont::Sample,
    soundfont::Zone, voice::gain::Context,
};
use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
use std::{io::Cursor, sync::Arc};

fn context() -> Context {
    Context {
        bank_volume: 127,
        program_volume: 127,
        tone_volume: 127,
        velocity: 127,
        channel_volume: 127,
        sequence_volume: [127, 127],
        tone_pan: 64,
        program_pan: 64,
        channel_pan: 64,
        mono: false,
    }
}

fn font(parameters: Parameters, reverb: u16, pulse: bool) -> Bank {
    let mut zone = Zone::new(0);
    zone.pan = parameters.pan;
    zone.attenuation_cb = parameters.attenuation_cb;
    zone.reverb_send = reverb;
    zone.envelope.delay = i16::MIN;
    zone.envelope.attack = i16::MIN;
    zone.envelope.hold = i16::MIN;
    zone.envelope.decay = i16::MIN;
    zone.envelope.release = -12000;
    zone.loop_mode = LoopMode::Continuous;
    let pcm = if pulse {
        let mut pcm = vec![0; 44100];
        pcm[..128].fill(10000);
        pcm
    } else {
        vec![10000; 1024]
    };
    let length = pcm.len();
    Bank {
        name: "Gain comparison".into(),
        samples: vec![Sample {
            name: "constant".into(),
            pcm,
            rate: 44100,
            root_key: 60,
            correction_cents: 0,
            loop_range: Some(0..length),
        }],
        instruments: vec![Instrument {
            name: "gain".into(),
            zones: vec![zone],
        }],
        presets: vec![Preset {
            name: "gain".into(),
            bank: 0,
            program: 0,
            instruments: vec![0],
        }],
    }
}

fn render(parameters: Parameters, reverb: u16, pulse: bool, effects: bool) -> [Vec<f32>; 2] {
    let bytes = font(parameters, reverb, pulse).encode().unwrap().bytes;
    let soundfont = SoundFont::new(&mut Cursor::new(bytes)).unwrap();
    let mut settings = SynthesizerSettings::new(44100);
    settings.block_size = 8;
    settings.enable_reverb_and_chorus = effects;
    let mut synth = Synthesizer::new(&Arc::new(soundfont), &settings).unwrap();
    synth.set_master_volume(1.0);
    for (cc, value) in [
        (7, 127),
        (39, 127),
        (11, 127),
        (43, 127),
        (10, 64),
        (42, 0),
        (91, 0),
        (93, 0),
    ] {
        synth.process_midi_message(0, 0xb0, cc, value);
    }
    synth.note_on(0, 60, 127);
    let mut left = vec![0.0; 22050];
    let mut right = vec![0.0; 22050];
    synth.render(&mut left, &mut right);
    [left, right]
}

#[test]
fn fits_keep_model_choice_source_registers_and_quantization_explicit() {
    for model in [Model::SpecificationScale, Model::RustySynth136] {
        for volume in [1, 32, 64, 96, 127] {
            for pan in [0, 32, 63, 64, 96, 127] {
                let mut c = context();
                c.tone_volume = volume;
                c.tone_pan = pan;
                let fit = gain::fit(c, model).unwrap();
                assert_eq!(fit.registers, c.registers().unwrap());
                assert_eq!(fit.predicted, gain::predict(fit.parameters.unwrap(), model));
                assert!(fit.max_absolute_error < 0.003, "{fit:?}");
            }
        }
    }
    let mut quiet = context();
    quiet.tone_volume = 64;
    let standard = gain::fit(quiet, Model::SpecificationScale).unwrap();
    let engine = gain::fit(quiet, Model::RustySynth136).unwrap();
    assert!(
        engine.parameters.unwrap().attenuation_cb > standard.parameters.unwrap().attenuation_cb * 2
    );
    assert!(standard.rustysynth136_prediction[0] > standard.target[0] * 2.0);
    assert!(engine.specification_scale_prediction[0] < engine.target[0] * 0.2);
    quiet.tone_volume = 0;
    let silent = gain::fit(quiet, Model::SpecificationScale).unwrap();
    assert!(silent.parameters.is_none());
    assert_eq!(silent.predicted, [0.0; 2]);
    assert_eq!(silent.max_absolute_error, 0.0);
    quiet.tone_volume = 128;
    assert!(gain::fit(quiet, Model::SpecificationScale).is_err());
}

#[test]
fn predicted_engine_gains_match_independent_consumer_pcm() {
    let parameters = Parameters {
        attenuation_cb: 0,
        pan: 0,
    };
    let baseline = render(parameters, 0, false, false);
    let mean = |pcm: &[f32]| {
        pcm[16384..].iter().map(|&v| f64::from(v)).sum::<f64>() / (pcm.len() - 16384) as f64
    };
    let baseline_prediction = gain::predict(parameters, Model::RustySynth136);
    for model in [Model::SpecificationScale, Model::RustySynth136] {
        for (volume, pan) in [(127, 0), (100, 32), (64, 64), (32, 96), (127, 127)] {
            let mut c = context();
            c.tone_volume = volume;
            c.tone_pan = pan;
            let fit = gain::fit(c, model).unwrap();
            let pcm = render(fit.parameters.unwrap(), 0, false, false);
            for channel in 0..2 {
                let actual =
                    mean(&pcm[channel]) / mean(&baseline[channel]) * baseline_prediction[channel];
                assert!(
                    (actual - fit.rustysynth136_prediction[channel]).abs() < 0.00001,
                    "{model:?} {volume}/{pan} channel {channel}: {actual} != {}",
                    fit.rustysynth136_prediction[channel]
                );
            }
        }
    }
}

#[test]
fn reverb_generator_reaches_consumer_and_invalid_send_is_rejected() {
    let p = Parameters {
        attenuation_cb: 0,
        pan: 0,
    };
    let dry = render(p, 0, true, true);
    let wet = render(p, 1000, true, true);
    let energy = |channels: &[Vec<f32>; 2]| {
        channels
            .iter()
            .flat_map(|v| &v[4410..])
            .map(|&v| f64::from(v).powi(2))
            .sum::<f64>()
    };
    assert!(energy(&dry) < 1e-12);
    assert!(energy(&wet) > 1e-5);
    // Region percent conversion is followed by 0.01 in Voice::start:
    // generator 500 is a half-amplitude send, not saturation at 10.
    let half_send = render(p, 500, true, true);
    assert!((energy(&half_send) / energy(&wet) - 0.25).abs() < 0.00001);
    let mut invalid = font(p, 0, false);
    invalid.instruments[0].zones[0].reverb_send = 1001;
    assert!(invalid.encode().is_err());
}
