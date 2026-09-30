//! Invert uniform SF2 tone gain/pan edits through the recorded game reference.
use crate::{
    bank::Bank, digest::sha256_hex, soundfont::bank as binding, soundfont::bank::Gain,
    soundfont::gain, soundfont::gain::Fit, soundfont::gain::Parameters,
    soundfont::reader::model::Zone, soundfont::reader::Font, Result,
};
use serde::Serialize;
use std::collections::BTreeSet;

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub original_header_sha256: String,
    pub output_header_sha256: String,
    pub tones: Vec<ToneReport>,
    pub limitations: Vec<&'static str>,
}
#[derive(Debug, Serialize)]
pub struct ToneReport {
    pub program: u8,
    pub tone: usize,
    pub sf2_instrument: usize,
    pub changed: bool,
    pub original_volume: u8,
    pub original_pan: u8,
    pub output_volume: u8,
    pub output_pan: u8,
    pub matching_candidates: Option<usize>,
    pub fit: Option<Fit>,
}
pub struct Packed {
    pub header: Vec<u8>,
    pub report: Report,
}

/// Only validates gain/pan fields. The enclosing bank packer must validate all
/// remaining SF2 content before these bytes can be published.
pub(crate) fn pack(
    binding: &binding::Report,
    original: &[u8],
    baseline: &Font,
    edited: &Font,
) -> Result<Packed> {
    if binding.header_sha256 != sha256_hex(original) {
        return Err("SF2 tone edit: header differs from regenerated binding".into());
    }
    if baseline.instruments.len() != edited.instruments.len() {
        return Err("SF2 tone edit: instrument insertion/removal unsupported".into());
    }
    let bank = Bank::parse(original)?;
    let mut header = original.to_vec();
    let mut tones = Vec::new();
    let mut owned = BTreeSet::new();
    for tone in &binding.tones {
        let index = tone.sf2_instrument;
        if !owned.insert(index) {
            return Err("SF2 tone edit: duplicate regenerated instrument identity".into());
        }
        let before = baseline
            .instruments
            .get(index)
            .ok_or("SF2 tone edit: missing baseline instrument")?;
        let after = edited
            .instruments
            .get(index)
            .ok_or("SF2 tone edit: missing edited instrument")?;
        if before.zones.len() != after.zones.len() {
            return Err("SF2 tone edit: zone insertion/removal unsupported".into());
        }
        let activating = tone.silent
            && before
                .zones
                .iter()
                .zip(&after.zones)
                .any(|(a, b)| a.target != b.target);
        let changed = activating
            || before
                .zones
                .iter()
                .zip(&after.zones)
                .any(|(a, b)| parameters(a) != parameters(b));
        let program = bank
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .ok_or("SF2 tone edit: source program missing")?;
        let source = program
            .tones
            .get(tone.source_tone.index)
            .ok_or("SF2 tone edit: source tone missing")?;
        let mut report = ToneReport {
            program: program.program,
            tone: source.index,
            sf2_instrument: index,
            changed,
            original_volume: source.volume,
            original_pan: source.pan,
            output_volume: source.volume,
            output_pan: source.pan,
            matching_candidates: None,
            fit: None,
        };
        if changed {
            let prefix = format!(
                "SF2 tone edit: program {} tone {}",
                program.program, source.index
            );
            let original_fit = tone
                .context
                .gain
                .fit
                .as_ref()
                .ok_or_else(|| format!("{prefix}: game gain reference required"))?;
            let verified = Gain::from_bank(&bank, program, source, original_fit.model)?;
            let fit = verified
                .fit
                .ok_or("SF2 tone edit: missing regenerated gain fit")?;
            if fit.source != original_fit.source || fit.parameters != original_fit.parameters {
                return Err(format!(
                    "{prefix}: gain context differs from regenerated game reference"
                )
                .into());
            }
            let expected = match fit.parameters {
                Some(parameters) => parameters,
                None if activating => Parameters { attenuation_cb: 0, pan: 0 },
                None => return Err(format!("{prefix}: editing silent tone controls requires playback-sample reconstruction").into()),
            };
            let requested = after
                .zones
                .first()
                .map(parameters)
                .ok_or("SF2 tone edit: empty instrument")?;
            if before
                .zones
                .iter()
                .any(|z| z.target.is_none() || parameters(z) != expected)
                || after
                    .zones
                    .iter()
                    .any(|z| z.target.is_none() || parameters(z) != requested)
            {
                return Err(format!("{prefix}: all key zones of a tone must agree on gain/pan; per-key or global edits unsupported").into());
            }
            if requested.attenuation_cb > 1440 || !(-500..=500).contains(&requested.pan) {
                return Err(
                    format!("{prefix}: attenuation/pan outside supported SF2 range").into(),
                );
            }
            let (selected, count) =
                inverse(&fit, requested).map_err(|e| format!("{prefix}: {e}"))?;
            let offset = 0x820 + program.tone_block * 512 + source.index * 32;
            header[offset + 2] = selected.source.tone_volume;
            header[offset + 3] = selected.source.tone_pan;
            report.output_volume = selected.source.tone_volume;
            report.output_pan = selected.source.tone_pan;
            report.matching_candidates = Some(count);
            report.fit = Some(selected);
        }
        tones.push(report);
    }
    if let Some(index) = binding.empty_program_instrument {
        let before = baseline
            .instruments
            .get(index)
            .ok_or("SF2 empty-program instrument missing")?;
        let after = edited
            .instruments
            .get(index)
            .ok_or("SF2 empty-program instrument missing")?;
        if before != after || !owned.insert(index) {
            return Err(
                "SF2 edit: generated empty-program instrument edits are unsupported".into(),
            );
        }
    }
    if owned.len() != edited.instruments.len() {
        return Err("SF2 tone edit: unmapped instruments cannot authorize gain edits".into());
    }
    let rebuilt = Bank::parse(&header)?;
    for report in tones.iter().filter(|t| t.changed) {
        let program = rebuilt
            .programs
            .iter()
            .find(|p| p.program == report.program)
            .ok_or("SF2 tone edit: rebuilt program missing")?;
        let tone = &program.tones[report.tone];
        let expected = report
            .fit
            .as_ref()
            .ok_or("SF2 tone edit: rebuilt fit missing")?;
        let actual = Gain::from_bank(&rebuilt, program, tone, expected.model)?;
        if actual.fit.as_ref().and_then(|f| f.parameters) != expected.parameters {
            return Err("SF2 tone edit: rebuilt gain/pan failed forward verification".into());
        }
    }
    Ok(Packed {
        report: Report {
            schema: "bof3.audio.sf2-tone-pack/v1",
            original_header_sha256: sha256_hex(original), output_header_sha256: sha256_hex(&header), tones,
            limitations: vec![
                "This inverse changes only uniform per-tone attenuation/pan with an exact forward fit; separate inverses validate pitch and sample assignments. Header hashes describe the enclosing bank packer's final header.",
                "Search covers all seven-bit tone volume/pan pairs at the recorded ordinary-sequence note-on context. Equivalent pairs minimize total distance from the original, then volume and pan; candidate count is reported.",
                "Equality is against quantized SF2 generators in the recorded gain model, not dynamic-controller or PSX/PC PCM equivalence. The selected fit reports its existing approximation error.",
                    "Silent tones require explicit relinking to existing PCM and a representable audible gain; the assignment inverse validates relinking. Arbitrary envelopes, key ranges and instrument topology reconstruction remain unsupported.",
            ],
        },
        header,
    })
}

fn parameters(zone: &Zone) -> Parameters {
    let amount = |operator| {
        zone.generators
            .iter()
            .find(|g| g.operator == operator)
            .map_or(0, |g| g.amount)
    };
    Parameters {
        attenuation_cb: amount(48),
        pan: amount(17) as i16,
    }
}

fn inverse(original: &Fit, requested: Parameters) -> Result<(Fit, usize)> {
    let mut best = None;
    let mut rank = (u16::MAX, u8::MAX, u8::MAX);
    let mut count = 0;
    for volume in 0u8..=127 {
        for pan in 0u8..=127 {
            let mut context = original.source;
            context.tone_volume = volume;
            context.tone_pan = pan;
            let candidate = gain::fit(context, original.model)?;
            if candidate.parameters != Some(requested) {
                continue;
            }
            count += 1;
            let distance = u16::from(volume.abs_diff(original.source.tone_volume))
                + u16::from(pan.abs_diff(original.source.tone_pan));
            let next = (distance, volume, pan);
            if next < rank {
                rank = next;
                best = Some(candidate);
            }
        }
    }
    best.map(|fit| (fit, count)).ok_or_else(|| format!(
        "attenuation {} / pan {} has no exact seven-bit tone volume/pan inverse at the recorded game context",
        requested.attenuation_cb, requested.pan
    ).into())
}
