use bof3_audio::{
    bank::Bank, soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::Gain,
    soundfont::bank::Options, soundfont::packing::sample, soundfont::reader::Font,
    soundfont::LoopMode,
};

/// Activate one tone-volume-muted layer per eligible bank; retain all source PCM.
pub fn check(bound: &Bound, header: &[u8], body: &[u8], options: &Options) -> bool {
    for tone in &bound.report.tones {
        if !tone.silent {
            continue;
        }
        let Some(fit) = &tone.context.gain.fit else {
            continue;
        };
        let p = bound
            .report
            .source_metadata
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .unwrap();
        let offset = 0x820 + p.tone_block * 512 + tone.source_tone.index * 32;
        let mut proposed = header.to_vec();
        proposed[offset + 2] = 127;
        proposed[offset + 3] = 64;
        let bank = Bank::parse(&proposed).unwrap();
        let program = bank
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .unwrap();
        let gain = Gain::from_bank(
            &bank,
            program,
            &program.tones[tone.source_tone.index],
            fit.model,
        )
        .unwrap();
        if gain.is_silent() {
            continue;
        }
        let mut edited = bound.soundfont.clone();
        let target = tone.source_sf2_sample;
        let mode = if edited.samples[target].loop_range.is_some() {
            LoopMode::Continuous
        } else {
            LoopMode::None
        };
        for zone in &mut edited.instruments[tone.sf2_instrument].zones {
            zone.sample = target;
            zone.loop_mode = mode;
            zone.attenuation_cb = gain.attenuation_cb;
            zone.pan = gain.pan;
        }
        let desired = edited.encode().unwrap().bytes;
        let packed = sample::pack_bank(
            &bound.report,
            header,
            body,
            &Font::from_bytes(bound.bytes.clone()).unwrap(),
            &Font::from_bytes(desired).unwrap(),
            None,
        )
        .unwrap();
        assert_eq!(packed.body, body);
        assert_eq!(packed.assignments.tones.len(), 1);
        assert!(packed.assignments.tones[0].unmuted);
        assert!(packed.report.samples.iter().all(|s| !s.content_changed));
        for (i, (&a, &b)) in header.iter().zip(&packed.header).enumerate() {
            if i != offset + 2 && i != offset + 3 {
                assert_eq!(a, b, "byte {i:x}");
            }
        }
        let rebuilt_bank = Bank::parse(&packed.header).unwrap();
        let mut contexts: Vec<_> = bound
            .report
            .tones
            .iter()
            .map(|t| t.context.clone())
            .collect();
        for c in &mut contexts {
            let p = rebuilt_bank
                .programs
                .iter()
                .find(|p| p.program == c.program)
                .unwrap();
            c.gain = Gain::from_bank(&rebuilt_bank, p, &p.tones[c.tone], fit.model).unwrap();
        }
        let rebuilt = binding::bind(
            bound.report.identity.clone(),
            &packed.header,
            body,
            &contexts,
            options,
        )
        .unwrap();
        if rebuilt.report.silence_sample.is_none() {
            let silence = bound.report.silence_sample.unwrap();
            edited.samples.remove(silence);
            for instrument in &mut edited.instruments {
                for zone in &mut instrument.zones {
                    assert_ne!(zone.sample, silence);
                    if zone.sample > silence {
                        zone.sample -= 1;
                    }
                }
            }
        }
        assert_eq!(rebuilt.bytes, edited.encode().unwrap().bytes);
        rustysynth::SoundFont::new(&mut std::io::Cursor::new(rebuilt.bytes)).unwrap();
        return true;
    }
    false
}
