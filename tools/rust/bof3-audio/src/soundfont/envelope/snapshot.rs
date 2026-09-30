//! Explicit note-on-only SF2 approximation for known zero SPU steps.
use crate::{
    soundfont::Bank, soundfont::LoopMode, soundfont::Sample, soundfont::Zone,
    voice::tuning::KeyTuning, Result,
};

pub(crate) fn is_known_stop(row: &KeyTuning) -> bool {
    row.pitch_register == 0
        && row.effective_step == 0
        && row.sf2.is_none()
        && row
            .pitch_lookup
            .is_none_or(|lookup| lookup.region.is_pitch_table())
}

/// Phase zero with three cleared history samples and Gaussian coefficient -1.
/// This is not the value held when an already advancing voice changes to zero.
pub(crate) fn held_sample(first_decoded: i16) -> i16 {
    (-i32::from(first_decoded)).div_euclid(32768) as i16
}

pub(crate) fn silence(font: &mut Bank, slot: &mut Option<usize>, rate: u32) -> usize {
    *slot.get_or_insert_with(|| {
        let index = font.samples.len();
        font.samples.push(Sample {
            name: "BOF3 silence".into(),
            pcm: vec![0; 64],
            rate,
            root_key: 60,
            correction_cents: 0,
            loop_range: Some(8..56),
        });
        index
    })
}

pub(crate) fn zone(row: &KeyTuning, template: &Zone, first: i16, silence: usize) -> Result<Zone> {
    if !is_known_stop(row) || held_sample(first) != 0 {
        return Err(format!(
            "stopped-pitch key {}: a known zero step and zero held key-on sample are required",
            row.key
        )
        .into());
    }
    let mut zone = template.clone();
    zone.keys = (row.key, row.key);
    zone.sample = silence;
    zone.root_key = Some(row.key);
    zone.coarse_tune = 0;
    zone.fine_tune = 0;
    zone.scale_tuning = 100;
    zone.loop_mode = LoopMode::Continuous;
    Ok(zone)
}
