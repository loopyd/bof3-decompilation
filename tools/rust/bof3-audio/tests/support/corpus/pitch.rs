use bof3_audio::{
    bank::Bank, soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::Options,
    soundfont::packing::sample, soundfont::reader::Font, voice::tuning::Reference,
};

/// One representable tuning edit per constructible bank; unchanged banks are
/// checked by the caller. Do not turn an ineligible probe into coverage.
pub fn check(
    bound: &Bound,
    header: &[u8],
    body: &[u8],
    options: &Options,
    reference: &mut Reference,
) -> bool {
    for tone in &bound.report.tones {
        let source = &tone.source_tone;
        for center in [
            source.center.saturating_sub(1),
            source.center.saturating_add(1),
        ] {
            if center == source.center {
                continue;
            }
            let mut changed = source.clone();
            changed.center = center;
            let rows = reference.tone(&changed, options.sample_rate).unwrap();
            if rows.iter().any(|r| r.sf2.is_none()) {
                continue;
            }
            let effect = |p: &bof3_audio::voice::tuning::Parameters| {
                -100 * i32::from(p.root_key)
                    + 100 * i32::from(p.coarse_tune)
                    + i32::from(p.fine_tune)
            };
            if rows
                .iter()
                .zip(&tone.context.tuning)
                .all(|(a, b)| effect(a.sf2.as_ref().unwrap()) == effect(b.sf2.as_ref().unwrap()))
            {
                continue;
            }
            let mut font = bound.soundfont.clone();
            for (zone, row) in font.instruments[tone.sf2_instrument]
                .zones
                .iter_mut()
                .zip(&rows)
            {
                *zone = row.zone(zone).unwrap();
            }
            let bytes = font.encode().unwrap().bytes;
            let packed = sample::pack_bank(
                &bound.report,
                header,
                body,
                &Font::from_bytes(bound.bytes.clone()).unwrap(),
                &Font::from_bytes(bytes).unwrap(),
                Some(&mut *reference),
            )
            .unwrap();
            assert_eq!(packed.body, body);
            assert_eq!(packed.pitch_controls.tones.len(), 1);
            let metadata = Bank::parse(&packed.header).unwrap();
            let program = metadata
                .programs
                .iter()
                .find(|p| p.program == tone.source_program)
                .unwrap();
            let offset = 0x820 + program.tone_block * 512 + source.index * 32 + 4;
            for (i, (&a, &b)) in header.iter().zip(&packed.header).enumerate() {
                if i != offset && i != offset + 1 {
                    assert_eq!(a, b);
                }
            }
            let mut contexts: Vec<_> = bound
                .report
                .tones
                .iter()
                .map(|t| t.context.clone())
                .collect();
            for c in &mut contexts {
                let p = metadata
                    .programs
                    .iter()
                    .find(|p| p.program == c.program)
                    .unwrap();
                c.tuning = reference
                    .tone(&p.tones[c.tone], options.sample_rate)
                    .unwrap();
            }
            let rebuilt = binding::bind(
                bound.report.identity.clone(),
                &packed.header,
                body,
                &contexts,
                options,
            )
            .unwrap();
            for (a, b) in rebuilt.report.tones[tone.sf2_instrument]
                .context
                .tuning
                .iter()
                .zip(&rows)
            {
                assert_eq!(
                    effect(a.sf2.as_ref().unwrap()),
                    effect(b.sf2.as_ref().unwrap())
                );
            }
            rustysynth::SoundFont::new(&mut std::io::Cursor::new(&rebuilt.bytes)).unwrap();
            return true;
        }
    }
    false
}
