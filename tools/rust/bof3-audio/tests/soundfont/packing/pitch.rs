use crate::fixture;
use bof3_audio::{
    bank::Bank, machine::executable::Executable, soundfont::bank as binding,
    soundfont::bank::Bound, soundfont::bank::Gain, soundfont::gain::Model,
    soundfont::packing::sample, soundfont::reader::Font, voice::tuning::Reference,
};

fn bind(header: &[u8], body: &[u8], reference: Option<&mut Reference>) -> Bound {
    let metadata = Bank::parse(header).unwrap();
    let mut contexts = fixture::contexts(header);
    let mut reference = reference;
    for context in &mut contexts {
        let p = metadata
            .programs
            .iter()
            .find(|p| p.program == context.program)
            .unwrap();
        let tone = &p.tones[context.tone];
        context.gain = Gain::from_bank(&metadata, p, tone, Model::RustySynth136).unwrap();
        if let Some(reference) = reference.as_deref_mut() {
            context.tuning = reference.tone(tone, 44100).unwrap();
            context.pitch_provenance = "original executable pitch routine".into();
        }
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
    bytes: Vec<u8>,
    reference: Option<&mut Reference>,
) -> bof3_audio::Result<sample::PackedBank> {
    sample::pack_bank(
        &bound.report,
        header,
        body,
        &Font::from_bytes(bound.bytes.clone())?,
        &Font::from_bytes(bytes)?,
        reference,
    )
}
fn cents(zone: &bof3_audio::soundfont::Zone) -> i32 {
    -100 * i32::from(zone.root_key.unwrap())
        + 100 * i32::from(zone.coarse_tune)
        + i32::from(zone.fine_tune)
}

#[test]
fn pitch_changes_need_executable_evidence_and_do_not_bypass_sample_only_validation() {
    let (header, body) = fixture::fixture();
    let bound = bind(&header, &body, None);
    let same = pack(&header, &body, &bound, bound.bytes.clone(), None).unwrap();
    assert_eq!(same.header, header);
    assert!(same.pitch_controls.tones.is_empty());
    let mut changed = bound.soundfont.clone();
    for z in &mut changed.instruments[0].zones {
        z.coarse_tune += 1;
    }
    let bytes = changed.encode().unwrap().bytes;
    assert!(pack(&header, &body, &bound, bytes.clone(), None)
        .err()
        .unwrap()
        .to_string()
        .contains("original game pitch reference required"));
    assert!(sample::pack(
        &bound.report,
        &body,
        &Font::from_bytes(bound.bytes.clone()).unwrap(),
        &Font::from_bytes(bytes).unwrap()
    )
    .is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; inverse candidates execute original US pitch routine"]
fn original_pitch_inverts_layers_equivalent_encodings_and_rejects_nonrepresentable_edits() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut reference = Reference::from_executable(&exe).unwrap();
    let (mut header, body) = fixture::fixture();
    header.extend(b"opaque tail survives tuning edits");
    header[0x825] = 5;
    header[0x826..0x828].copy_from_slice(&[48, 84]);
    let original = bind(&header, &body, Some(&mut reference));
    // Independently execute every ignored shift-bit combination over all keys.
    for center in [0, 59, 60, 127, 128, 255] {
        for group in [0, 1, 15, 16, 31] {
            for key in [0, 48, 60, 84, 127] {
                let expected = reference.pitch_register(key, center, group * 8).unwrap();
                for low in 1..8 {
                    assert_eq!(
                        reference
                            .pitch_register(key, center, group * 8 + low)
                            .unwrap(),
                        expected
                    );
                }
            }
        }
    }
    let mut target = header.clone();
    target[0x822..0x826].copy_from_slice(&[93, 44, 59, 77]);
    target[0x844..0x846].copy_from_slice(&[61, 133]);
    let desired = bind(&target, &body, Some(&mut reference));
    let mut aliases = desired.soundfont.clone();
    // Reserved silent-program instruments have no root key; edit only VAB tones.
    let instruments: std::collections::BTreeSet<_> = desired
        .report
        .tones
        .iter()
        .map(|tone| tone.sf2_instrument)
        .collect();
    for &index in &instruments {
        let instrument = &mut aliases.instruments[index];
        for zone in &mut instrument.zones {
            zone.root_key = Some(zone.root_key.unwrap() + 1);
            zone.coarse_tune += 1;
        }
    }
    let packed = pack(
        &header,
        &body,
        &original,
        aliases.encode().unwrap().bytes,
        Some(&mut reference),
    )
    .unwrap();
    assert_eq!(packed.body, body);
    assert_eq!(packed.pitch_controls.tones.len(), 3);
    assert!(packed.tone_controls.tones[0].changed);
    assert_eq!(
        packed.tone_controls.output_header_sha256,
        bof3_audio::digest::sha256_hex(&packed.header)
    );
    for (i, (&a, &b)) in header.iter().zip(&packed.header).enumerate() {
        if ![0x822, 0x823, 0x824, 0x825, 0x844, 0x845, 0xa24, 0xa25].contains(&i) {
            assert_eq!(a, b, "header offset {i:x}");
        }
    }
    for report in &packed.pitch_controls.tones {
        assert!(report.matching_candidates >= 8);
        assert!(report
            .selected
            .iter()
            .all(|r| r.sf2.as_ref().unwrap().error_cents.abs() <= 0.50000001));
    }
    let rebuilt = bind(&packed.header, &body, Some(&mut reference));
    for index in instruments {
        let a = &desired.soundfont.instruments[index];
        let b = &rebuilt.soundfont.instruments[index];
        assert_eq!(a.zones.len(), b.zones.len());
        for (a, b) in a.zones.iter().zip(&b.zones) {
            assert_eq!(cents(a), cents(b));
        }
    }
    rustysynth::SoundFont::new(&mut std::io::Cursor::new(rebuilt.bytes)).unwrap();
    // A representation-only root/coarse change leaves original VH/VB intact,
    // including the ignored low three shift bits in original shift=5.
    let mut equivalent = original.soundfont.clone();
    for z in &mut equivalent.instruments[0].zones {
        z.root_key = Some(61);
        z.coarse_tune += 1;
    }
    let same = pack(
        &header,
        &body,
        &original,
        equivalent.encode().unwrap().bytes,
        Some(&mut reference),
    )
    .unwrap();
    assert_eq!(same.header, header);
    assert_eq!(same.body, body);
    let mut impossible = original.soundfont.clone();
    impossible.instruments[0].zones[0].fine_tune += 1;
    assert!(pack(
        &header,
        &body,
        &original,
        impossible.encode().unwrap().bytes,
        Some(&mut reference)
    )
    .err()
    .unwrap()
    .to_string()
    .contains("no exact VAB"));
    let mut unsupported = desired.soundfont.clone();
    unsupported.instruments[0].zones[0].scale_tuning = 99;
    assert!(pack(
        &header,
        &body,
        &original,
        unsupported.encode().unwrap().bytes,
        Some(&mut reference)
    )
    .is_err());
    eprintln!("Original pitch inverse: 3 layered tone requests, 2 changed tunings, equivalent representation, 1050 low-bit checks, rejection of per-key cent and scale edits");
}
