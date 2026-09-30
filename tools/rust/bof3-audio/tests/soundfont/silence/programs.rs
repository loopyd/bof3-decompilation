use crate::fixture as common;
use bof3_audio::{soundfont::bank as binding, soundfont::packing::sample, soundfont::reader::Font};
use std::{io::Cursor, sync::Arc};

#[test]
fn empty_presets_are_silent_source_preserving_and_reject_unrepresented_edits() {
    let (vh, vb) = common::fixture();
    let options = common::options();
    let bound = binding::bind(
        common::identity(),
        &vh,
        &vb,
        &common::contexts(&vh),
        &options,
    )
    .unwrap();
    assert_eq!(bound.report.programs.len(), 2);
    assert_eq!(bound.report.empty_programs.len(), 126);
    assert_eq!(bound.soundfont.presets.len(), 256);
    let baseline = Font::from_bytes(bound.bytes.clone()).unwrap();
    let unchanged = sample::pack_bank(&bound.report, &vh, &vb, &baseline, &baseline, None).unwrap();
    assert_eq!(unchanged.header, vh);
    assert_eq!(unchanged.body, vb);
    let font = Arc::new(rustysynth::SoundFont::new(&mut Cursor::new(&bound.bytes)).unwrap());
    for (program, audible) in [(1, false), (127, false), (0, true)] {
        let mut synth =
            rustysynth::Synthesizer::new(&font, &rustysynth::SynthesizerSettings::new(44100))
                .unwrap();
        synth.process_midi_message(0, 0xc0, program, 0);
        synth.note_on(0, 60, 127);
        let mut left = vec![0.0; 4096];
        let mut right = left.clone();
        synth.render(&mut left, &mut right);
        assert_eq!(left.iter().chain(&right).any(|&v| v != 0.0), audible);
    }
    let instrument = bound.report.empty_program_instrument.unwrap();
    let sample = bound.report.empty_program_sample.unwrap();
    for mutation in 0..4 {
        let mut edited = bound.soundfont.clone();
        match mutation {
            0 => edited.instruments[instrument].zones[0].sample = 0,
            1 => edited.instruments[instrument].zones[0].coarse_tune = 1,
            2 => edited.samples[sample].pcm[0] = 12000,
            _ => edited.presets[bound.report.empty_programs[0].sf2_preset].instruments = vec![0],
        }
        let parsed = Font::from_bytes(edited.encode().unwrap().bytes).unwrap();
        assert!(
            sample::pack_bank(&bound.report, &vh, &vb, &baseline, &parsed, None).is_err(),
            "mutation {mutation}"
        );
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn original_runtime_and_pc_both_produce_silence_for_a_zero_tone_program() {
    use bof3_audio::{
        machine::{executable::Executable, firmware::Image},
        pc_archive, pc_render, psx_render,
    };
    use emi_ex_v2::image::ArchiveImage;
    use std::{fs, path::PathBuf};
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let bios = fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    let original = ArchiveImage::from_bytes(
        fs::read(
            PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap())
                .join("BIN/BGM/BGM053.EMI"),
        )
        .unwrap(),
    )
    .unwrap();
    for (program, key, audible) in [(1, 40, false), (10, 10, true)] {
        let events = [
            0, 0xc0, program, 0, 0x90, key, 127, 96, 0x90, key, 0, 0, 0xff, 0x2f,
        ];
        let mut sep = original.entry(1).unwrap()[..19].to_vec();
        sep[8..10].copy_from_slice(&96u16.to_be_bytes());
        sep[10..13].copy_from_slice(&[7, 0xa1, 0x20]);
        sep[15..19].copy_from_slice(&(events.len() as u32).to_be_bytes());
        sep.extend(events);
        sep.resize(original.entry(1).unwrap().len(), 0);
        let archive = original.replace_entries(&[(1, &sep)]).unwrap();
        let psx = psx_render::render(
            &exe,
            Image::from_bytes(bios.clone()).unwrap(),
            &archive,
            &psx_render::Options {
                sequence: 0,
                layout: None,
                body: psx_render::Body::Duration(22050),
                release_frames: 0,
                safety_frames: 44100,
            },
        )
        .unwrap();
        let pc = pc_archive::render(
            &exe,
            "empty-program.emi",
            &archive,
            &pc_archive::Options {
                sequence: 0,
                loops: None,
                allow_approximations: true,
                playback: pc_render::Options {
                    duration_frames: Some(22050),
                    release_frames: 0,
                    ..Default::default()
                },
            },
        )
        .unwrap();
        assert_eq!(psx.wave.pcm.iter().any(|&v| v != 0), audible);
        assert_eq!(pc.wave.pcm.iter().any(|&v| v != 0), audible);
    }
}
