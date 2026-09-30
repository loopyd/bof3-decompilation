//! Rebind fixed tone zones to existing, source-qualified VAB PCM allocations.
use crate::{bank::Bank, soundfont::bank as binding, soundfont::reader::Font, Result};
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub tones: Vec<ToneReport>,
    pub limitations: Vec<&'static str>,
}
#[derive(Debug, Serialize)]
pub struct ToneReport {
    pub program: u8,
    pub tone: usize,
    pub sf2_instrument: usize,
    pub original_reference: i16,
    pub output_reference: i16,
    pub original_sf2_sample: usize,
    pub output_sf2_sample: usize,
    pub original_sample_id: u16,
    pub output_sample_id: u16,
    pub unmuted: bool,
}
pub(crate) struct Packed {
    pub patches: Vec<(usize, [u8; 2])>,
    pub report: Report,
}

/// The enclosing gain inverse has already checked instrument/zone identities;
/// the sample inverse must subsequently validate loop semantics and all PCM.
pub(crate) fn pack(
    binding: &binding::Report,
    header: &[u8],
    baseline: &Font,
    edited: &Font,
) -> Result<Packed> {
    let bank = Bank::parse(header)?;
    let mut patches = Vec::new();
    let mut tones = Vec::new();
    for tone in &binding.tones {
        let index = tone.sf2_instrument;
        let before = &baseline.instruments[index];
        let after = &edited.instruments[index];
        let prefix = format!(
            "SF2 sample assignment: program {} tone {}",
            tone.source_program, tone.source_tone.index
        );
        if before.zones.len() != tone.context.tuning.len() {
            return Err(
                format!("{prefix}: baseline zones differ from regenerated key contexts").into(),
            );
        }
        if before
            .zones
            .iter()
            .zip(&tone.context.tuning)
            .any(|(z, row)| {
                let expected = if tone.stopped_keys.contains(&row.key) {
                    binding.silence_sample
                } else {
                    Some(tone.sf2_sample)
                };
                z.target != expected
            })
        {
            return Err(
                format!("{prefix}: baseline sample differs from regenerated binding").into(),
            );
        }
        if before
            .zones
            .iter()
            .zip(&after.zones)
            .all(|(a, b)| a.target == b.target)
        {
            continue;
        }
        if !tone.stopped_keys.is_empty() {
            return Err(format!(
                "{prefix}: sample reassignment for stopped-pitch tones is unsupported"
            )
            .into());
        }
        let target = after
            .zones
            .first()
            .and_then(|z| z.target)
            .ok_or_else(|| format!("{prefix}: every tone zone requires a PCM sample"))?;
        if after.zones.iter().any(|z| z.target != Some(target)) {
            return Err(format!("{prefix}: all key zones must select the same sample; per-key assignment requires tone splitting").into());
        }
        let source = binding
            .samples
            .iter()
            .find(|s| s.sf2_sample == Some(target))
            .ok_or_else(|| {
                format!("{prefix}: target sample {target} is generated or has no VAB allocation")
            })?;
        let program = bank
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .ok_or("SF2 sample assignment: source program missing")?;
        let original = program
            .tones
            .get(tone.source_tone.index)
            .ok_or("SF2 sample assignment: source tone missing")?;
        if tone.silent {
            let model = tone
                .context
                .gain
                .fit
                .as_ref()
                .ok_or_else(|| format!("{prefix}: silent tone requires a game gain reference"))?
                .model;
            if binding::Gain::from_bank(&bank, program, original, model)?.is_silent() {
                return Err(
                    format!("{prefix}: silent tone requires a validated audible gain").into(),
                );
            }
        }
        let old = crate::sample::reference::resolve_us_pcm(
            original.sample_reference,
            bank.declared_samples,
        )?;
        if old.sample_id != tone.sample_resolution.sample_id {
            return Err(
                format!("{prefix}: source reference differs from regenerated binding").into(),
            );
        }
        if source.sample_id == 0 || source.sample_id >= 255 {
            return Err(format!(
                "{prefix}: allocation {} cannot be selected as PCM; runtime byte 255 selects noise",
                source.sample_id
            )
            .into());
        }
        let current_sample = baseline
            .samples
            .get(tone.source_sf2_sample)
            .ok_or("SF2 sample assignment: missing source sample")?;
        let new_sample = baseline
            .samples
            .get(target)
            .ok_or("SF2 sample assignment: missing target sample")?;
        if current_sample.rate != new_sample.rate
            || current_sample.root_key != new_sample.root_key
            || current_sample.correction_cents != new_sample.correction_cents
        {
            return Err(format!("{prefix}: sample-header tuning contexts differ; explicit pitch reconstruction required").into());
        }
        // The original US tone selector truncates the signed word to its low
        // byte. Keep the otherwise opaque high byte and untouched zero aliases.
        let output_reference = if old.sample_id == source.sample_id {
            original.sample_reference
        } else {
            ((original.sample_reference as u16 & 0xff00) | source.sample_id) as i16
        };
        let resolved =
            crate::sample::reference::resolve_us_pcm(output_reference, bank.declared_samples)?;
        if resolved.sample_id != source.sample_id {
            return Err(format!("{prefix}: selected reference failed forward validation").into());
        }
        patches.push((
            0x820 + program.tone_block * 512 + original.index * 32 + 22,
            output_reference.to_le_bytes(),
        ));
        tones.push(ToneReport {
            program: program.program,
            tone: original.index,
            sf2_instrument: index,
            original_reference: original.sample_reference,
            output_reference,
            original_sf2_sample: tone.sf2_sample,
            output_sf2_sample: target,
            original_sample_id: old.sample_id,
            output_sample_id: resolved.sample_id,
            unmuted: tone.silent,
        });
    }
    Ok(Packed {patches,report:Report {
        schema:"bof3.audio.sf2-assignment-pack/v1",tones,
        limitations:vec![
            "Only existing PCM allocations may be selected, uniformly across each fixed tone's key zones. Unmuting additionally requires the gain inverse's validated audible controls. Sample insertion, tone splitting and generated-silence targets are unsupported.",
            "The verified US selector uses the low byte; unchanged zero aliases and the original high byte are preserved. Runtime byte 255 selects noise and cannot name PCM allocation 255.",
            "All edited users of a shared sample must agree on loop mode; the sample inverse separately validates loop endpoints, PCM and allocation capacity before publication.",
            "Pitch-header contexts remain fixed. These reference edits do not establish complete game playback, voice allocation or independent PSX/PC fidelity.",
        ],
    }})
}
