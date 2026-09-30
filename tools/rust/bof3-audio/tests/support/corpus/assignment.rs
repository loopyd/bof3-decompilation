use bof3_audio::{
    soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::Options,
    soundfont::packing::sample, soundfont::reader::Font, soundfont::LoopMode,
};

/// One existing-PCM reassignment per eligible bank; preserve all sample data.
pub fn check(bound: &Bound, header: &[u8], body: &[u8], options: &Options) -> bool {
    for tone in &bound.report.tones {
        if tone.silent {
            continue;
        }
        let Some(target) = bound
            .report
            .samples
            .iter()
            .find(|s| s.sample_id < 255 && s.sf2_sample.is_some_and(|i| i != tone.sf2_sample))
        else {
            continue;
        };
        let mut font = bound.soundfont.clone();
        for zone in &mut font.instruments[tone.sf2_instrument].zones {
            zone.sample = target.sf2_sample.unwrap();
            zone.loop_mode = if target.loop_range.is_some() {
                LoopMode::Continuous
            } else {
                LoopMode::None
            };
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
        assert_eq!(packed.assignments.tones.len(), 1);
        assert!(packed.report.samples.iter().all(|s| !s.content_changed));
        let contexts: Vec<_> = bound
            .report
            .tones
            .iter()
            .map(|t| t.context.clone())
            .collect();
        let rebuilt = binding::bind(
            bound.report.identity.clone(),
            &packed.header,
            body,
            &contexts,
            options,
        )
        .unwrap();
        assert_eq!(rebuilt.bytes, desired);
        let source = &bound
            .report
            .source_metadata
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .unwrap();
        let offset = 0x820 + source.tone_block * 512 + tone.source_tone.index * 32 + 22;
        for (i, (&a, &b)) in header.iter().zip(&packed.header).enumerate() {
            if i != offset {
                assert_eq!(a, b, "byte {i:x}");
            }
        }
        rustysynth::SoundFont::new(&mut std::io::Cursor::new(&rebuilt.bytes)).unwrap();
        return true;
    }
    false
}
