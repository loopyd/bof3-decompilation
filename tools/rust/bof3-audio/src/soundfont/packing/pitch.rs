//! Invert SF2 note-on pitch through the original, hash-identified US routine.
use crate::{
    bank::Bank, soundfont::bank as binding, soundfont::reader::model::Zone,
    soundfont::reader::Font, voice::tuning::KeyTuning, voice::tuning::Parameters,
    voice::tuning::Reference, Result,
};
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
    pub original_center: u8,
    pub original_shift: u8,
    pub output_center: u8,
    pub output_shift: u8,
    pub matching_candidates: usize,
    pub requested_cents: Vec<i32>,
    pub selected: Vec<KeyTuning>,
    pub executable_sha256: String,
}
pub(crate) struct Packed {
    pub patches: Vec<(usize, [u8; 2])>,
    pub report: Report,
}

/// Other generators, topology and sample metadata are checked by the enclosing
/// packer. Patches remain private until all components have been validated.
pub(crate) fn pack(
    binding: &binding::Report,
    header: &[u8],
    baseline: &Font,
    edited: &Font,
    mut reference: Option<&mut Reference>,
) -> Result<Packed> {
    let bank = Bank::parse(header)?;
    let mut tones = Vec::new();
    let mut patches = Vec::new();
    for tone in &binding.tones {
        let before = &baseline.instruments[tone.sf2_instrument];
        let after = &edited.instruments[tone.sf2_instrument];
        if before
            .zones
            .iter()
            .zip(&after.zones)
            .all(|(a, b)| raw(a) == raw(b))
        {
            continue;
        }
        let prefix = format!(
            "SF2 tuning edit: program {} tone {}",
            tone.source_program, tone.source_tone.index
        );
        if !tone.stopped_keys.is_empty() {
            return Err(
                format!("{prefix}: pitch edits for stopped-pitch tones are unsupported").into(),
            );
        }
        let reference = reference
            .as_deref_mut()
            .ok_or_else(|| format!("{prefix}: original game pitch reference required"))?;
        let program = bank
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .ok_or("SF2 tuning edit: missing program")?;
        let source = program
            .tones
            .get(tone.source_tone.index)
            .ok_or("SF2 tuning edit: missing tone")?;
        let sample = baseline
            .samples
            .get(tone.sf2_sample)
            .ok_or("SF2 tuning edit: missing sample")?;
        let original = reference.tone(source, sample.rate)?;
        if original.len() != before.zones.len() || original.len() != after.zones.len() {
            return Err(format!("{prefix}: every original key zone must be retained").into());
        }
        let mut requested = Vec::new();
        for ((original, before), after) in original.iter().zip(&before.zones).zip(&after.zones) {
            let expected = original
                .sf2
                .as_ref()
                .ok_or_else(|| format!("{prefix}: {}", original.unsupported_message()))?;
            if cents(before)? != effective_cents(expected)
                || key(before)? != original.key
                || key(after)? != original.key
                || before.target.is_none()
                || after.target.is_none()
            {
                return Err(format!(
                    "{prefix}: baseline/key zones differ from original pitch reference"
                )
                .into());
            }
            requested.push(cents(after).map_err(|e| format!("{prefix}: {e}"))?);
        }
        let (center, shift, matching_candidates, selected) = inverse(
            reference,
            source.center,
            source.shift,
            &original,
            &requested,
        )
        .map_err(|e| format!("{prefix}: {e}"))?;
        patches.push((
            0x820 + program.tone_block * 512 + source.index * 32 + 4,
            [center, shift],
        ));
        tones.push(ToneReport {
            program: program.program,
            tone: source.index,
            sf2_instrument: tone.sf2_instrument,
            original_center: source.center,
            original_shift: source.shift,
            output_center: center,
            output_shift: shift,
            matching_candidates,
            requested_cents: requested,
            selected,
            executable_sha256: reference.profile().exe_sha256.clone(),
        });
    }
    Ok(Packed {patches, report: Report {
        schema: "bof3.audio.sf2-pitch-pack/v1", tones,
        limitations: vec![
            "Fixed key zones, sample rates, sample pitch headers and scale tuning; only root/coarse/fine instrument generators are inverted.",
            "Equality compares combined note-on cents after integer SF2 quantization; equivalent root/coarse/fine encodings may re-export differently. Every key must match exactly, without nearest-pitch substitution.",
            "The verified US zero-fine-argument routine ignores the low three shift bits. All 256 centers and 32 shift groups are searched; equivalent bytes minimize distance to original center/shift, then center and shift.",
            "Candidates reading outside the verified pitch table or requiring zero/unrepresentable SF2 rates are rejected. Game bends, controller changes, initialized runtime and PC/PSX PCM equivalence remain separate obligations.",
        ],
    }})
}

fn raw(zone: &Zone) -> [Option<u16>; 3] {
    [58, 51, 52].map(|op| {
        zone.generators
            .iter()
            .find(|g| g.operator == op)
            .map(|g| g.amount)
    })
}
fn key(zone: &Zone) -> Result<u8> {
    let amount = zone
        .generators
        .iter()
        .find(|g| g.operator == 43)
        .ok_or("missing single-key range")?
        .amount;
    let [lo, hi] = amount.to_le_bytes();
    if lo != hi || hi > 127 {
        return Err("single-key range required".into());
    }
    Ok(lo)
}
fn cents(zone: &Zone) -> Result<i32> {
    let [root, coarse, fine] = raw(zone);
    let root = root.ok_or("explicit root key required")?;
    let coarse = coarse.unwrap_or(0) as i16;
    let fine = fine.unwrap_or(0) as i16;
    if root > 127 || !(-120..=120).contains(&coarse) || !(-99..=99).contains(&fine) {
        return Err("root/coarse/fine outside supported SF2 range".into());
    }
    Ok(-100 * i32::from(root) + 100 * i32::from(coarse) + i32::from(fine))
}
fn effective_cents(p: &Parameters) -> i32 {
    -100 * i32::from(p.root_key) + 100 * i32::from(p.coarse_tune) + i32::from(p.fine_tune)
}
fn inverse(
    reference: &mut Reference,
    original_center: u8,
    original_shift: u8,
    keys: &[KeyTuning],
    requested: &[i32],
) -> Result<(u8, u8, usize, Vec<KeyTuning>)> {
    let mut selected = None;
    let mut best = (u16::MAX, u8::MAX, u8::MAX);
    let mut count = 0;
    for center in 0u8..=255 {
        for group in 0u8..32 {
            let first = group * 8;
            let shift = original_shift.clamp(first, first + 7);
            let mut rows = Vec::new();
            for (original, &wanted) in keys.iter().zip(requested) {
                let row =
                    reference.probe_key(original.key, center, shift, original.sf2_sample_rate)?;
                if row
                    .sf2
                    .as_ref()
                    .is_none_or(|p| effective_cents(p) != wanted)
                {
                    break;
                }
                rows.push(row);
            }
            if rows.len() != keys.len() {
                continue;
            }
            count += 8;
            let rank = (
                u16::from(center.abs_diff(original_center))
                    + u16::from(shift.abs_diff(original_shift)),
                center,
                shift,
            );
            if rank < best {
                best = rank;
                selected = Some(rows);
            }
        }
    }
    selected.map(|rows|(best.1,best.2,count,rows)).ok_or_else(|| "no exact VAB center/shift reproduces every requested key through the original pitch routine".into())
}
