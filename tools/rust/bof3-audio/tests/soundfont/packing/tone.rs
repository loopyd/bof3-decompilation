use crate::fixture as bank_fixture;
#[path = "../../support/soundfont/controls.rs"]
mod controls;
use bof3_audio::{
    bank::Bank, soundfont::bank as binding, soundfont::bank::Gain, soundfont::gain::Model,
    soundfont::gain::Parameters, soundfont::packing::sample, soundfont::reader::Font,
};

fn bind(header: &[u8], body: &[u8], model: Model) -> binding::Bound {
    let bank = Bank::parse(header).unwrap();
    let mut contexts = bank_fixture::contexts(header);
    for context in &mut contexts {
        let program = bank
            .programs
            .iter()
            .find(|p| p.program == context.program)
            .unwrap();
        context.gain =
            Gain::from_bank(&bank, program, &program.tones[context.tone], model).unwrap();
    }
    binding::bind(
        bank_fixture::identity(),
        header,
        body,
        &contexts,
        &bank_fixture::options(),
    )
    .unwrap()
}
fn pack(
    header: &[u8],
    body: &[u8],
    bound: &binding::Bound,
    edited: Vec<u8>,
) -> bof3_audio::Result<sample::PackedBank> {
    sample::pack_bank(
        &bound.report,
        header,
        body,
        &Font::from_bytes(bound.bytes.clone())?,
        &Font::from_bytes(edited)?,
        None,
    )
}

#[test]
fn tone_controls_reconstruct_forward_export_for_both_gain_models() {
    for model in [Model::SpecificationScale, Model::RustySynth136] {
        let (mut header, body) = bank_fixture::fixture();
        header.extend(b"opaque VAB header tail");
        let original = bind(&header, &body, model);
        let same = pack(&header, &body, &original, original.bytes.clone()).unwrap();
        assert_eq!(same.header, header);
        assert_eq!(same.body, body);
        assert!(same
            .tone_controls
            .tones
            .iter()
            .all(|t| !t.changed && t.matching_candidates.is_none()));
        let mut changed = header.clone();
        changed[0x822..0x824].copy_from_slice(&[93, 44]);
        changed[0x842..0x844].copy_from_slice(&[110, 80]);
        let desired = bind(&changed, &body, model);
        let packed = pack(&header, &body, &original, desired.bytes.clone()).unwrap();
        assert_eq!(packed.body, body);
        assert!(!packed.report.soundfont_byte_equal);
        assert_eq!(
            packed
                .tone_controls
                .tones
                .iter()
                .filter(|t| t.changed)
                .count(),
            2
        );
        for report in packed.tone_controls.tones.iter().filter(|t| t.changed) {
            assert!(report.matching_candidates.unwrap() > 0);
            assert!(report.fit.as_ref().unwrap().max_absolute_error.is_finite());
        }
        for (i, (&before, &after)) in header.iter().zip(&packed.header).enumerate() {
            if ![0x822, 0x823, 0x842, 0x843].contains(&i) {
                assert_eq!(before, after, "offset {i:x}");
            }
        }
        let round_trip = bind(&packed.header, &packed.body, model);
        assert_eq!(round_trip.bytes, desired.bytes);
        let font =
            rustysynth::SoundFont::new(&mut std::io::Cursor::new(&round_trip.bytes)).unwrap();
        assert_eq!(font.get_instruments().len(), 4);
        // The sample-only entry point must not bypass tone validation.
        assert!(sample::pack(
            &original.report,
            &body,
            &Font::from_bytes(original.bytes.clone()).unwrap(),
            &Font::from_bytes(desired.bytes).unwrap()
        )
        .is_err());
    }
}

#[test]
fn partial_key_controls_unrepresentable_gain_and_silent_edits_fail() {
    let (header, body) = bank_fixture::fixture();
    let bound = bind(&header, &body, Model::RustySynth136);
    let mut edited = bound.bytes.clone();
    controls::set(
        &mut edited,
        0,
        Parameters {
            attenuation_cb: 100,
            pan: 50,
        },
        false,
    );
    assert!(pack(&header, &body, &bound, edited)
        .err()
        .unwrap()
        .to_string()
        .contains("all key zones"));
    let mut edited = bound.bytes.clone();
    controls::set(
        &mut edited,
        0,
        Parameters {
            attenuation_cb: 0,
            pan: 500,
        },
        true,
    );
    assert!(pack(&header, &body, &bound, edited)
        .err()
        .unwrap()
        .to_string()
        .contains("no exact seven-bit"));
    let mut edited = bound.bytes.clone();
    controls::set(
        &mut edited,
        0,
        Parameters {
            attenuation_cb: 1441,
            pan: 0,
        },
        true,
    );
    assert!(pack(&header, &body, &bound, edited)
        .err()
        .unwrap()
        .to_string()
        .contains("outside supported"));
    let mut silent = header.clone();
    silent[0x822] = 0;
    let bound = bind(&silent, &body, Model::RustySynth136);
    let mut edited = bound.bytes.clone();
    controls::set(
        &mut edited,
        0,
        Parameters {
            attenuation_cb: 100,
            pan: 0,
        },
        true,
    );
    assert!(pack(&silent, &body, &bound, edited)
        .err()
        .unwrap()
        .to_string()
        .contains("silent tone"));
}

#[test]
fn combined_tone_and_pcm_edits_validate_all_content_and_source_headers() {
    let (header, body) = bank_fixture::fixture();
    let bound = bind(&header, &body, Model::RustySynth136);
    let mut changed = header.clone();
    changed[0x822] = 80;
    let desired = bind(&changed, &body, Model::RustySynth136);
    let mut edited = desired.bytes.clone();
    let font = Font::from_bytes(edited.clone()).unwrap();
    let pcm = font
        .chunks
        .iter()
        .find(|c| c.id == *b"smpl")
        .unwrap()
        .data
        .start;
    edited[pcm..pcm + 2].copy_from_slice(&(-12000i16).to_le_bytes());
    let result = pack(&header, &body, &bound, edited.clone()).unwrap();
    assert!(result.tone_controls.tones[0].changed);
    assert!(result.report.samples[1].content_changed);
    assert_ne!(result.header, header);
    assert_ne!(result.body, body);
    assert_eq!(
        result.report.input_soundfont_sha256,
        bof3_audio::digest::sha256_hex(&edited)
    );
    let info = font
        .chunks
        .iter()
        .find(|c| c.id == *b"INAM")
        .unwrap()
        .data
        .start;
    edited[info] ^= 1;
    assert!(pack(&header, &body, &bound, edited).is_err());
    assert!(pack(&changed, &body, &bound, desired.bytes)
        .err()
        .unwrap()
        .to_string()
        .contains("header differs"));
}

#[test]
fn ambiguous_center_pan_keeps_nearest_original_controls() {
    let (header, body) = bank_fixture::fixture();
    let bound = bind(&header, &body, Model::RustySynth136);
    let mut changed = header.clone();
    changed[0x822..0x824].copy_from_slice(&[93, 63]);
    let desired = bind(&changed, &body, Model::RustySynth136);
    let packed = pack(&header, &body, &bound, desired.bytes.clone()).unwrap();
    let report = &packed.tone_controls.tones[0];
    // The game's 63 and 64 center pans both leave left/right gains unchanged.
    assert!(report.matching_candidates.unwrap() >= 2);
    assert_eq!((report.output_volume, report.output_pan), (93, 64));
    assert_eq!(
        bind(&packed.header, &body, Model::RustySynth136).bytes,
        desired.bytes
    );
}
