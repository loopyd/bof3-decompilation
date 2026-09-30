//! Preserve the no-voice behavior of zero-tone VAB program slots in SF2.
use crate::soundfont::{Bank, Instrument, Preset, Sample, Zone};
use serde::Serialize;
use std::collections::BTreeSet;

#[derive(Clone, Debug, Serialize)]
pub struct Program {
    pub source_program: u8,
    pub sf2_bank: u16,
    pub sf2_preset: usize,
    pub percussion_preset: Option<usize>,
}

pub(crate) struct Addition {
    pub sample: usize,
    pub instrument: usize,
    pub programs: Vec<Program>,
}

pub(crate) fn add(
    font: &mut Bank,
    present: &BTreeSet<u8>,
    sample_rate: u32,
    sf2_bank: u16,
    percussion_alias: bool,
) -> Option<Addition> {
    if present.len() == 128 {
        return None;
    }
    let sample = font.samples.len();
    font.samples.push(Sample {
        name: "Empty VAB programs".into(),
        pcm: vec![0; 64],
        rate: sample_rate,
        root_key: 60,
        correction_cents: 0,
        loop_range: None,
    });
    let instrument = font.instruments.len();
    let mut zone = Zone::new(sample);
    zone.scale_tuning = 0;
    font.instruments.push(Instrument {
        name: "Empty VAB programs".into(),
        zones: vec![zone],
    });
    let mut programs = Vec::new();
    for program in (0..128).filter(|p| !present.contains(p)) {
        let preset = font.presets.len();
        font.presets.push(Preset {
            name: format!("Empty P{program:03}"),
            bank: sf2_bank,
            program,
            instruments: vec![instrument],
        });
        let percussion_preset = percussion_alias.then(|| {
            let index = font.presets.len();
            font.presets.push(Preset {
                name: format!("Empty drum P{program:03}"),
                bank: sf2_bank + 128,
                program,
                instruments: vec![instrument],
            });
            index
        });
        programs.push(Program {
            source_program: program,
            sf2_bank,
            sf2_preset: preset,
            percussion_preset,
        });
    }
    Some(Addition {
        sample,
        instrument,
        programs,
    })
}
