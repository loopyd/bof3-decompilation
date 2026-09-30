use crate::fixture;
use bof3_audio::{
    bank::Bank, soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::Gain,
    soundfont::gain::Model, soundfont::packing::sample, soundfont::reader::Font,
    soundfont::Bank as FontBank, soundfont::LoopMode,
};

fn bind(header: &[u8], body: &[u8], model: Model) -> Bound {
    let bank = Bank::parse(header).unwrap();
    let mut contexts = fixture::contexts(header);
    for c in &mut contexts {
        let p = bank
            .programs
            .iter()
            .find(|p| p.program == c.program)
            .unwrap();
        c.gain = Gain::from_bank(&bank, p, &p.tones[c.tone], model).unwrap();
    }
    binding::bind(
        fixture::identity(),
        header,
        body,
        &contexts,
        &fixture::options(),
    )
    .unwrap()
}
fn pack(
    header: &[u8],
    body: &[u8],
    bound: &Bound,
    font: &FontBank,
) -> bof3_audio::Result<sample::PackedBank> {
    sample::pack_bank(
        &bound.report,
        header,
        body,
        &Font::from_bytes(bound.bytes.clone())?,
        &Font::from_bytes(font.encode()?.bytes)?,
        None,
    )
}
fn activate(font: &mut FontBank, bound: &Bound, index: usize) {
    let target = bound.report.tones[index].source_sf2_sample;
    let mode = if font.samples[target].loop_range.is_some() {
        LoopMode::Continuous
    } else {
        LoopMode::None
    };
    for z in &mut font.instruments[index].zones {
        z.sample = target;
        z.loop_mode = mode;
    }
}

#[test]
fn unmute_one_layer_preserves_alias_and_reexports_exactly_for_both_models() {
    for model in [Model::SpecificationScale, Model::RustySynth136] {
        let (mut header, body) = fixture::fixture();
        header[0x822] = 0;
        header[0x842] = 0;
        header[0x836..0x838].copy_from_slice(&0xa500u16.to_le_bytes());
        header.extend(b"opaque header tail");
        let original = bind(&header, &body, model);
        let same = pack(&header, &body, &original, &original.soundfont).unwrap();
        assert_eq!(same.header, header);
        assert_eq!(same.body, body);
        let mut changed = header.clone();
        changed[0x822..0x824].copy_from_slice(&[93, 44]);
        let desired = bind(&changed, &body, model);
        let result = pack(&header, &body, &original, &desired.soundfont).unwrap();
        assert_eq!(result.body, body);
        assert!(result.assignments.tones[0].unmuted);
        assert!(result.tone_controls.tones[0].changed);
        assert!(result.tone_controls.tones[0].matching_candidates.unwrap() > 0);
        for (i, (&a, &b)) in header.iter().zip(&result.header).enumerate() {
            if ![0x822, 0x823].contains(&i) {
                assert_eq!(a, b, "byte {i:x}");
            }
        }
        let rebuilt = bind(&result.header, &body, model);
        assert_eq!(rebuilt.bytes, desired.bytes);
        rustysynth::SoundFont::new(&mut std::io::Cursor::new(rebuilt.bytes)).unwrap();
        assert!(sample::pack(
            &original.report,
            &body,
            &Font::from_bytes(original.bytes).unwrap(),
            &Font::from_bytes(desired.bytes).unwrap()
        )
        .is_err());
    }
}

#[test]
fn relink_alone_activates_unity_gain_and_reexport_omits_unused_silence() {
    for model in [Model::SpecificationScale, Model::RustySynth136] {
        let (mut header, body) = fixture::fixture();
        header[0x822] = 0;
        let original = bind(&header, &body, model);
        let mut edited = original.soundfont.clone();
        activate(&mut edited, &original, 0);
        let result = pack(&header, &body, &original, &edited).unwrap();
        assert!(result.tone_controls.tones[0].changed);
        assert!(result.assignments.tones[0].unmuted);
        assert_eq!(result.body, body);
        let rebuilt = bind(&result.header, &body, model);
        assert!(!rebuilt.report.tones[0].silent);
        assert_eq!(rebuilt.report.silence_sample, None);
        let silence = original.report.silence_sample.unwrap();
        edited.samples.remove(silence);
        for instrument in &mut edited.instruments {
            for zone in &mut instrument.zones {
                assert_ne!(zone.sample, silence);
                if zone.sample > silence {
                    zone.sample -= 1;
                }
            }
        }
        assert_eq!(rebuilt.bytes, edited.encode().unwrap().bytes);
        rustysynth::SoundFont::new(&mut std::io::Cursor::new(rebuilt.bytes)).unwrap();
    }
}

#[test]
fn unmute_rejects_incomplete_controls_partial_links_and_generated_sample_edits() {
    let (mut header, body) = fixture::fixture();
    header[0x822] = 0;
    let original = bind(&header, &body, Model::RustySynth136);
    let error = |font: &FontBank| {
        pack(&header, &body, &original, font)
            .err()
            .unwrap()
            .to_string()
    };
    let mut edited = original.soundfont.clone();
    for z in &mut edited.instruments[0].zones {
        z.attenuation_cb = 100;
    }
    assert!(error(&edited).contains("silent tone"));
    edited = original.soundfont.clone();
    activate(&mut edited, &original, 0);
    edited.instruments[0].zones[1].sample = original.report.silence_sample.unwrap();
    assert!(error(&edited).contains("all key zones must select"));
    activate(&mut edited, &original, 0);
    edited.instruments[0].zones[1].pan = 100;
    assert!(error(&edited).contains("all key zones of a tone"));
    let mut edited = original.soundfont.clone();
    activate(&mut edited, &original, 0);
    let silence = original.report.silence_sample.unwrap();
    edited.samples[silence].pcm[0] = 1;
    assert!(error(&edited).contains("generated silence"));
    edited.samples[silence].pcm[0] = 0;
    edited.samples[silence].loop_range = Some(9..56);
    assert!(error(&edited).contains("playback-only sample loop"));
}

#[test]
fn tone_controls_cannot_override_program_or_bank_mutes() {
    for offset in [0x18, 0x21] {
        let (mut header, body) = fixture::fixture();
        header[offset] = 0;
        let original = bind(&header, &body, Model::RustySynth136);
        let mut edited = original.soundfont.clone();
        activate(&mut edited, &original, 0);
        let error = pack(&header, &body, &original, &edited)
            .err()
            .unwrap()
            .to_string();
        assert!(error.contains("no exact seven-bit"), "{error}");
    }
}
