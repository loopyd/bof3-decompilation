use crate::fixture as common;
use bof3_audio::{
    bank::Bank, soundfont::bank as binding, soundfont::packing::sample, soundfont::reader::Font,
};
use std::{io::Cursor, sync::Arc};

fn assign(
    font: &mut bof3_audio::soundfont::Bank,
    binding: &binding::Report,
    program: u8,
    targets: &[usize],
) {
    let empty = binding
        .empty_programs
        .iter()
        .find(|p| p.source_program == program)
        .unwrap();
    font.presets[empty.sf2_preset].instruments = targets.to_vec();
    font.presets[empty.percussion_preset.unwrap()].instruments = targets.to_vec();
}
fn pcm(bytes: &[u8], program: i32) -> Vec<f32> {
    let font = Arc::new(rustysynth::SoundFont::new(&mut Cursor::new(bytes)).unwrap());
    let mut synth =
        rustysynth::Synthesizer::new(&font, &rustysynth::SynthesizerSettings::new(44100)).unwrap();
    synth.process_midi_message(0, 0xc0, program, 0);
    synth.note_on(0, 60, 127);
    let mut left = vec![0.0; 8192];
    let mut right = left.clone();
    synth.render(&mut left, &mut right);
    left.extend(right);
    left
}
#[test]
fn layered_population_preserves_existing_blocks_samples_and_playback() {
    let (mut vh, vb) = common::fixture();
    vh.extend(b"opaque VAB trailer");
    let total = (vh.len() + vb.len()) as u32;
    vh[0x0c..0x10].copy_from_slice(&total.to_le_bytes());
    let bound = binding::bind(
        common::identity(),
        &vh,
        &vb,
        &common::contexts(&vh),
        &common::options(),
    )
    .unwrap();
    let baseline = Font::from_bytes(bound.bytes.clone()).unwrap();
    let mut font = bound.soundfont.clone();
    assign(&mut font, &bound.report, 1, &[0, 1]);
    let edited_bytes = font.encode().unwrap().bytes;
    let edited = Font::from_bytes(edited_bytes.clone()).unwrap();
    let packed = sample::pack_bank(&bound.report, &vh, &vb, &baseline, &edited, None).unwrap();
    assert_eq!(packed.body, vb);
    assert_eq!(packed.populated_programs.len(), 1);
    assert_eq!(packed.populated_programs[0].program, 1);
    assert_eq!(packed.populated_programs[0].tones.len(), 2);
    assert_eq!(packed.header.len(), vh.len() + 512);
    assert_eq!(&packed.header[0x820..0xa20], &vh[0x820..0xa20]);
    assert_eq!(&packed.header[0xc20..], &vh[0xa20..]);
    let bank = Bank::parse(&packed.header).unwrap();
    assert_eq!(bank.declared_file_bytes, total + 512);
    assert_eq!(bank.declared_tones, 5);
    assert_eq!(
        bank.programs.iter().map(|p| p.program).collect::<Vec<_>>(),
        [0, 1, 5]
    );
    assert!(bank.programs[1]
        .tones
        .iter()
        .all(|t| t.program_reference == 1 && t.sample_reference == 2));
    let regenerated = binding::bind(
        common::identity(),
        &packed.header,
        &vb,
        &common::contexts(&packed.header),
        &common::options(),
    )
    .unwrap();
    let desired = pcm(&edited_bytes, 1);
    assert!(desired.iter().any(|&v| v != 0.0));
    assert_eq!(desired, pcm(&regenerated.bytes, 1));
    assert_eq!(pcm(&bound.bytes, 0), pcm(&regenerated.bytes, 0));
    assert_eq!(pcm(&bound.bytes, 5), pcm(&regenerated.bytes, 5));
    let regenerated_font = Font::from_bytes(regenerated.bytes.clone()).unwrap();
    let unchanged = sample::pack_bank(
        &regenerated.report,
        &packed.header,
        &vb,
        &regenerated_font,
        &regenerated_font,
        None,
    )
    .unwrap();
    assert_eq!(unchanged.header, packed.header);
    assert_eq!(unchanged.body, vb);
}
#[test]
fn population_rejects_alias_conflicts_tone_overflow_and_unsupported_preset_edits() {
    let (vh, vb) = common::fixture();
    let bound = binding::bind(
        common::identity(),
        &vh,
        &vb,
        &common::contexts(&vh),
        &common::options(),
    )
    .unwrap();
    let baseline = Font::from_bytes(bound.bytes.clone()).unwrap();
    for case in 0..4 {
        let mut font = bound.soundfont.clone();
        assign(&mut font, &bound.report, 1, &[0]);
        let p = bound
            .report
            .empty_programs
            .iter()
            .find(|p| p.source_program == 1)
            .unwrap();
        match case {
            0 => font.presets[p.percussion_preset.unwrap()].instruments = vec![1],
            1 => assign(&mut font, &bound.report, 1, &[0; 17]),
            2 => font.presets[p.sf2_preset].name = "Changed identity".into(),
            _ => font.presets[bound.report.programs[0].sf2_preset].instruments = vec![1],
        }
        let edited = Font::from_bytes(font.encode().unwrap().bytes).unwrap();
        let error = sample::pack_bank(&bound.report, &vh, &vb, &baseline, &edited, None)
            .err()
            .unwrap()
            .to_string();
        let expected = [
            "alias assignments must agree",
            "1..=16",
            "identity/metadata",
            "replacing populated",
        ][case];
        assert!(error.contains(expected), "{error}");
    }
    let mut font = bound.soundfont.clone();
    assign(&mut font, &bound.report, 1, &[0]);
    let mut bytes = font.encode().unwrap().bytes;
    let parsed = Font::from_bytes(bytes.clone()).unwrap();
    let terminal = parsed
        .chunks
        .iter()
        .find(|c| c.id == *b"pgen")
        .unwrap()
        .data
        .end
        - 2;
    bytes[terminal..terminal + 2].copy_from_slice(&1u16.to_le_bytes());
    let edited = Font::from_bytes(bytes).unwrap();
    let error = sample::pack_bank(&bound.report, &vh, &vb, &baseline, &edited, None)
        .err()
        .unwrap()
        .to_string();
    assert!(
        error.contains("opaque preset terminal fields changed"),
        "{error}"
    );
}
