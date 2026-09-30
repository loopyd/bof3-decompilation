use bof3_audio::{
    bank::Bank, soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::Gain,
    soundfont::bank::Options, soundfont::gain, soundfont::packing::sample, soundfont::reader::Font,
};

/// Exercise one changed, nonsilent tone per eligible constructible bank.
pub fn check(bound: &Bound, header: &[u8], body: &[u8], options: &Options) -> bool {
    for tone in &bound.report.tones {
        let fit = tone.context.gain.fit.as_ref().unwrap();
        let Some(original) = fit.parameters else {
            continue;
        };
        let mut context = fit.source;
        context.tone_volume = if context.tone_volume == 127 { 96 } else { 127 };
        context.tone_pan = if context.tone_pan == 64 { 32 } else { 64 };
        let Some(requested) = gain::fit(context, fit.model).unwrap().parameters else {
            continue;
        };
        if requested == original {
            continue;
        }
        let mut font = bound.soundfont.clone();
        for zone in &mut font.instruments[tone.sf2_instrument].zones {
            zone.pan = requested.pan;
            zone.attenuation_cb = requested.attenuation_cb;
        }
        let desired = font.encode().unwrap().bytes;
        let packed = sample::pack_bank(
            &bound.report,
            header,
            body,
            &Font::from_bytes(bound.bytes.clone()).unwrap(),
            &Font::from_bytes(desired.clone()).unwrap(),
            None,
        )
        .unwrap();
        assert_eq!(packed.body, body);
        assert_eq!(
            packed
                .tone_controls
                .tones
                .iter()
                .filter(|t| t.changed)
                .count(),
            1
        );
        let metadata = Bank::parse(&packed.header).unwrap();
        let program = metadata
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .unwrap();
        let offset = 0x820 + program.tone_block * 512 + tone.source_tone.index * 32;
        for (i, (&before, &after)) in header.iter().zip(&packed.header).enumerate() {
            if i != offset + 2 && i != offset + 3 {
                assert_eq!(before, after);
            }
        }
        let contexts: Vec<_> = bound
            .report
            .tones
            .iter()
            .map(|t| {
                let mut c = t.context.clone();
                let p = metadata
                    .programs
                    .iter()
                    .find(|p| p.program == t.source_program)
                    .unwrap();
                c.gain = Gain::from_bank(&metadata, p, &p.tones[t.source_tone.index], fit.model)
                    .unwrap();
                c
            })
            .collect();
        let reconstructed = binding::bind(
            bound.report.identity.clone(),
            &packed.header,
            body,
            &contexts,
            options,
        )
        .unwrap();
        assert_eq!(reconstructed.bytes, desired);
        rustysynth::SoundFont::new(&mut std::io::Cursor::new(&reconstructed.bytes)).unwrap();
        return true;
    }
    false
}
