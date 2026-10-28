//! Reconstruct supported SF2 PCM/loop edits and validated appended VAB allocations.
use crate::{
    codec::adpcm, codec::adpcm::encoder, codec::quantization, codec::quantization::Loss,
    digest::sha256_hex, soundfont::bank as binding, soundfont::reader::Font, Result,
};
use serde::Serialize;
use std::{
    collections::{BTreeMap, BTreeSet},
    ops::Range,
};

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub soundfont_byte_equal: bool,
    pub baseline_soundfont_sha256: String,
    pub input_soundfont_sha256: String,
    pub samples: Vec<SampleReport>,
    pub allocated_samples: Vec<AllocationReport>,
    pub limitations: Vec<&'static str>,
}
#[derive(Debug, Serialize)]
pub struct SampleReport {
    pub sample_id: u16,
    pub sf2_sample: Option<usize>,
    pub content_changed: bool,
    pub encoded_changed: bool,
    pub original_encoded_sha256: String,
    pub output_encoded_sha256: String,
    pub preserved_tail_bytes: usize,
    pub replaced_original_tail_bytes: usize,
    pub pcm24_rounding_loss: Option<Loss>,
    pub pcm24_total_loss: Option<Loss>,
    pub encoding: Option<encoder::Report>,
    pub loop_reconstruction: crate::sample::editing::LoopReport,
}
pub struct Packed {
    pub body: Vec<u8>,
    pub report: Report,
}

pub struct PackedBank {
    pub header: Vec<u8>,
    pub body: Vec<u8>,
    pub report: Report,
    pub tone_controls: crate::soundfont::packing::tone::Report,
    pub pitch_controls: crate::soundfont::packing::pitch::Report,
    pub envelope_controls: crate::soundfont::packing::envelope::Report,
    pub assignments: crate::soundfont::packing::assignment::Report,
    pub key_ranges: Vec<crate::soundfont::packing::program::KeyRangeReport>,
    pub populated_programs: Vec<crate::soundfont::packing::program::Population>,
    pub independent_instruments: Vec<crate::soundfont::packing::program::IndependentReport>,
}

#[derive(Debug, Serialize)]
pub struct AllocationReport {
    pub sample_id: u16,
    pub sf2_sample: usize,
    pub input_name: [u8; 20],
    pub generated_name: String,
    pub body_offset: usize,
    pub encoded_bytes: usize,
    pub encoded_sha256: String,
    pub overwritten_unused_size_units: u16,
    pub reused_encoding_from: Option<u16>,
    pub pcm24_rounding_loss: Loss,
    pub pcm24_total_loss: Loss,
    pub encoding: Option<encoder::Report>,
}

#[derive(Default)]
struct Allocations {
    references: BTreeMap<usize, u16>,
    bytes: Vec<u8>,
    reports: Vec<AllocationReport>,
}

#[derive(Default)]
struct Structures<'a> {
    instrument_prefix: Option<&'a Font>,
    range_prefix: Option<&'a Font>,
    allocations: Option<&'a Allocations>,
}

/// New samples occupy new size-table entries and are inserted before the
/// preserved body tail. Existing allocation offsets and bytes never move.
fn allocations(
    binding: &binding::Report,
    header: &[u8],
    body: &[u8],
    baseline: &Font,
    edited: &Font,
) -> Result<Allocations> {
    let added = edited
        .samples
        .len()
        .checked_sub(baseline.samples.len())
        .ok_or("SF2 allocation: original samples cannot be removed")?;
    let mut output = Allocations::default();
    if added == 0 {
        return Ok(output);
    }
    if binding.body_sha256 != sha256_hex(body) {
        return Err("SF2 allocation: body differs from regenerated binding".into());
    }
    let bank = crate::bank::Bank::parse(header)?;
    if usize::from(bank.declared_samples) + added > 254 {
        return Err(
            "SF2 allocation: appended PCM exceeds selectable sample IDs 1..254; 255 selects noise"
                .into(),
        );
    }
    let insertion = bank
        .samples
        .last()
        .map_or(bank.body_prefix_bytes, |s| s.body_offset + s.encoded_bytes);
    if insertion > body.len() {
        return Err("SF2 allocation: source allocations exceed body".into());
    }
    let table = 0x820 + usize::from(bank.declared_programs) * 512;
    let modes = modes(edited)?;
    let mut next = baseline.pcm24_pool().len();
    for (index, sample) in edited
        .samples
        .iter()
        .enumerate()
        .skip(baseline.samples.len())
    {
        let prefix = format!("SF2 allocation sample {index}");
        if sample.kind != 1 || sample.link != 0 || sample.start as usize != next {
            return Err(format!(
                "{prefix}: append contiguous, unlinked mono samples after the original PCM pool"
            )
            .into());
        }
        next = (sample.end as usize)
            .checked_add(46)
            .ok_or("SF2 allocation: guard range overflow")?;
        if !edited
            .pcm24_pool()
            .get(sample.end as usize..next)
            .is_some_and(|s| s.iter().all(|&v| v == 0))
        {
            return Err(format!("{prefix}: retain 46 zero guard points").into());
        }
        let used = binding.tones.iter().any(|tone| {
            edited
                .instruments
                .get(tone.sf2_instrument)
                .is_some_and(|instrument| instrument.zones.iter().any(|z| z.target == Some(index)))
        }) || edited
            .instruments
            .iter()
            .skip(baseline.instruments.len())
            .any(|i| i.zones.iter().any(|z| z.target == Some(index)));
        if !used {
            return Err(format!("{prefix}: new sample must be used by a validated tone").into());
        }
        let mode = modes
            .get(&index)
            .copied()
            .ok_or("SF2 allocation: missing playback mode")?;
        let sample_loop = loop_range(edited, index, mode)?;
        if mode == 0 && (sample.loop_start, sample.loop_end) != (sample.start, sample.start) {
            return Err(format!(
                "{prefix}: nonlooping samples require both unused loop points at sample start"
            )
            .into());
        }
        let input = edited.sample_pcm24(index)?;
        if input.is_empty() || !input.len().is_multiple_of(28) {
            return Err(format!(
                "{prefix}: nonempty whole 28-frame ADPCM blocks required; no implicit padding"
            )
            .into());
        }
        let required = (input.len() / 28)
            .checked_mul(16)
            .ok_or("SF2 allocation: encoded size overflow")?;
        if required / 8 > usize::from(u16::MAX)
            || body
                .len()
                .checked_add(output.bytes.len())
                .and_then(|n| n.checked_add(required))
                .is_none_or(|n| n > 512 * 1024)
        {
            return Err(format!(
                "{prefix}: encoded data exceeds VAB size-table or 512 KiB SPU capacity"
            )
            .into());
        }
        let pcm: Vec<i16> = input
            .iter()
            .map(|&v| ((v + 128).div_euclid(256)).clamp(-32768, 32767) as i16)
            .collect();
        let rounded: Vec<i32> = pcm.iter().map(|&v| i32::from(v) * 256).collect();
        let mut reused = None;
        for source in &binding.samples {
            let bytes = body
                .get(source.body_offset..source.body_offset + source.encoded_bytes)
                .ok_or("SF2 allocation: existing sample exceeds body")?;
            if sha256_hex(bytes) != source.sha256 {
                return Err("SF2 allocation: source sample hash differs".into());
            }
            let decoded = adpcm::decode_sample(bytes)?;
            if decoded
                .pcm
                .iter()
                .map(|&v| i32::from(v) * 256)
                .eq(input.iter().copied())
                && decoded
                    .sample_loop
                    .as_ref()
                    .map(|l| l.start_frame..l.end_frame_exclusive)
                    == sample_loop
                && decoded
                    .sample_loop
                    .as_ref()
                    .is_none_or(|l| l.pcm_repeat_is_stable)
            {
                reused = Some((source.sample_id, bytes.to_vec()));
                break;
            }
        }
        let (bytes, encoding, reused_encoding_from) = if let Some((id, bytes)) = reused {
            (bytes, None, Some(id))
        } else {
            let encoded = encoder::encode(
                &pcm,
                &encoder::Options {
                    sample_loop: sample_loop.clone(),
                    ..Default::default()
                },
            )?;
            (encoded.bytes, Some(encoded.report), None)
        };
        if bytes.is_empty()
            || !bytes.len().is_multiple_of(16)
            || bytes.len() / 8 > usize::from(u16::MAX)
        {
            return Err(
                format!("{prefix}: encoded allocation exceeds VAB size-table capacity").into(),
            );
        }
        let actual = adpcm::decode_sample(&bytes)?;
        if actual.pcm.len() != input.len()
            || actual
                .sample_loop
                .as_ref()
                .map(|l| l.start_frame..l.end_frame_exclusive)
                != sample_loop
        {
            return Err(format!("{prefix}: reconstructed duration or loop differs").into());
        }
        let actual: Vec<i32> = actual.pcm.iter().map(|&v| i32::from(v) * 256).collect();
        let sample_id = bank.declared_samples + u16::try_from(output.reports.len())? + 1;
        output.references.insert(index, sample_id);
        output.reports.push(AllocationReport {
            sample_id,
            sf2_sample: index,
            input_name: sample.name,
            generated_name: format!("S{sample_id:03}"),
            body_offset: insertion + output.bytes.len(),
            encoded_bytes: bytes.len(),
            encoded_sha256: sha256_hex(&bytes),
            overwritten_unused_size_units: u16::from_le_bytes(
                header[table + usize::from(sample_id) * 2..][..2].try_into()?,
            ),
            reused_encoding_from,
            pcm24_rounding_loss: quantization::measure(input, &rounded),
            pcm24_total_loss: quantization::measure(input, &actual),
            encoding,
        });
        output.bytes.extend(bytes);
    }
    if edited.pcm24_pool().len() != next {
        return Err("SF2 allocation: extra unowned PCM after new sample guards".into());
    }
    if body
        .len()
        .checked_add(output.bytes.len())
        .is_none_or(|n| n > 512 * 1024)
    {
        return Err(
            "SF2 allocation: body exceeds 512 KiB SPU capacity before game layout constraints"
                .into(),
        );
    }
    Ok(output)
}

fn apply_allocations(
    header: &mut [u8],
    body: &mut Vec<u8>,
    allocations: &Allocations,
) -> Result<()> {
    if allocations.references.is_empty() {
        return Ok(());
    }
    let bank = crate::bank::Bank::parse(header)?;
    let insertion = bank
        .samples
        .last()
        .map_or(bank.body_prefix_bytes, |s| s.body_offset + s.encoded_bytes);
    let table = 0x820 + usize::from(bank.declared_programs) * 512;
    for sample in &allocations.reports {
        let at = table + usize::from(sample.sample_id) * 2;
        header[at..at + 2].copy_from_slice(&u16::try_from(sample.encoded_bytes / 8)?.to_le_bytes());
    }
    let count = bank.declared_samples + u16::try_from(allocations.reports.len())?;
    header[0x16..0x18].copy_from_slice(&count.to_le_bytes());
    let size = bank
        .declared_file_bytes
        .checked_add(u32::try_from(allocations.bytes.len())?)
        .ok_or("SF2 allocation: declared file size overflow")?;
    header[0xc..0x10].copy_from_slice(&size.to_le_bytes());
    body.splice(insertion..insertion, allocations.bytes.iter().copied());
    let rebuilt = crate::bank::Bank::parse(header)?;
    for expected in &allocations.reports {
        let actual = &rebuilt.samples[usize::from(expected.sample_id) - 1];
        if (actual.body_offset, actual.encoded_bytes)
            != (expected.body_offset, expected.encoded_bytes)
            || sha256_hex(&body[actual.body_offset..actual.body_offset + actual.encoded_bytes])
                != expected.encoded_sha256
        {
            return Err(
                "SF2 allocation: rebuilt sample range/hash failed forward verification".into(),
            );
        }
    }
    Ok(())
}

/// Validate instrument controls, assignments and samples before accepting edits.
pub fn pack_bank(
    binding: &binding::Report,
    header: &[u8],
    body: &[u8],
    baseline: &Font,
    edited: &Font,
    mut reference: Option<&mut crate::voice::tuning::Reference>,
) -> Result<PackedBank> {
    let original_header_sha256 = sha256_hex(header);
    if crate::soundfont::packing::program::has_range_changes(baseline, edited) {
        validate_range_source(baseline)?;
    }
    let key_ranges = crate::soundfont::packing::program::ranges(
        binding,
        header,
        baseline,
        edited,
        reference.as_deref_mut(),
    )?;
    let range_prefix = key_ranges.as_ref().map(|_| baseline);
    let (binding, header, baseline) = key_ranges
        .as_ref()
        .map_or((binding, header, baseline), |ranges| {
            (&ranges.binding, ranges.header.as_slice(), &ranges.baseline)
        });
    let allocations = allocations(binding, header, body, baseline, edited)?;
    let independent = crate::soundfont::packing::program::independent(
        binding,
        header,
        baseline,
        edited,
        &allocations.references,
        reference.as_deref_mut(),
    )?;
    let fixed = if independent.tones.is_empty() {
        None
    } else {
        let mut font = Font::from_bytes(edited.original_bytes().to_vec())?;
        font.instruments.truncate(baseline.instruments.len());
        Some(font)
    };
    let existing = fixed.as_ref().unwrap_or(edited);
    let populated_programs =
        crate::soundfont::packing::program::derive(binding, baseline, edited, &independent)?;
    let mut tones = crate::soundfont::packing::tone::pack(binding, header, baseline, existing)?;
    let pitch =
        crate::soundfont::packing::pitch::pack(binding, header, baseline, existing, reference)?;
    let assignments = crate::soundfont::packing::assignment::pack(
        binding,
        &tones.header,
        baseline,
        existing,
        &allocations.references,
    )?;
    if range_prefix.is_some() {
        validate_range_destinations(binding, body, baseline, existing, &assignments.report)?;
    }
    let envelopes = crate::soundfont::packing::envelope::pack(
        binding,
        header,
        baseline,
        existing,
        &assignments.report,
    )?;
    if let Some(original) = range_prefix {
        let mut verified_header = tones.header.clone();
        for (offset, bytes) in &envelopes.patches {
            verified_header[*offset..*offset + 2].copy_from_slice(bytes);
        }
        crate::soundfont::packing::envelope::verify(&verified_header, &envelopes.report)?;
        validate_range_tables(
            original,
            baseline,
            existing,
            &tones.report,
            (binding, &pitch.report),
            &assignments.report,
            &envelopes.report,
        )?;
    }
    let mut samples = pack_inner(
        binding,
        body,
        baseline,
        edited,
        true,
        !populated_programs.is_empty(),
        Structures {
            instrument_prefix: (!independent.tones.is_empty()).then_some(baseline),
            range_prefix,
            allocations: Some(&allocations),
        },
    )?;
    if range_prefix.is_some() && samples.body != body {
        return Err(
            "SF2 key range: sample assignment must preserve the entire original body".into(),
        );
    }
    for (offset, bytes) in pitch
        .patches
        .into_iter()
        .chain(assignments.patches)
        .chain(envelopes.patches)
    {
        tones.header[offset..offset + 2].copy_from_slice(&bytes);
    }
    crate::soundfont::packing::envelope::verify(&tones.header, &envelopes.report)?;
    tones.header = crate::soundfont::packing::program::apply(
        &tones.header,
        &populated_programs,
        &independent,
    )?;
    apply_allocations(&mut tones.header, &mut samples.body, &allocations)?;
    samples.report.allocated_samples = allocations.reports;
    crate::bank::Bank::parse(&tones.header)?;
    tones.report.original_header_sha256 = original_header_sha256;
    tones.report.output_header_sha256 = sha256_hex(&tones.header);
    Ok(PackedBank {
        header: tones.header,
        body: samples.body,
        report: samples.report,
        tone_controls: tones.report,
        pitch_controls: pitch.report,
        envelope_controls: envelopes.report,
        assignments: assignments.report,
        key_ranges: key_ranges.map_or_else(Vec::new, |ranges| ranges.report),
        populated_programs,
        independent_instruments: independent.report,
    })
}

/// `binding` and `baseline` must be freshly generated from the preserved bank.
/// XML mapping claims are not accepted as reconstruction authority.
pub fn pack(
    binding: &binding::Report,
    original: &[u8],
    baseline: &Font,
    edited: &Font,
) -> Result<Packed> {
    pack_inner(
        binding,
        original,
        baseline,
        edited,
        false,
        false,
        Structures::default(),
    )
}

fn pack_inner(
    binding: &binding::Report,
    original: &[u8],
    baseline: &Font,
    edited: &Font,
    tone_controls: bool,
    program_edits: bool,
    structures: Structures<'_>,
) -> Result<Packed> {
    if binding.body_sha256 != sha256_hex(original) {
        return Err("SF2 edit: source body does not match regenerated binding".into());
    }
    let added = structures.allocations.map_or(0, |a| a.references.len());
    if baseline.samples.len() + added != edited.samples.len()
        || (added == 0 && baseline.pcm24_pool().len() != edited.pcm24_pool().len())
    {
        return Err(
            "SF2 edit: sample count/storage resizing requires allocation/layout reconstruction"
                .into(),
        );
    }
    let sample_prefix = (added > 0).then_some(baseline);
    let before = controls(
        baseline,
        tone_controls,
        program_edits,
        structures.instrument_prefix,
        sample_prefix,
        structures.range_prefix,
    )?;
    let after = controls(
        edited,
        tone_controls,
        program_edits,
        structures.instrument_prefix,
        sample_prefix,
        structures.range_prefix,
    )?;
    if before.len() != after.len() {
        return Err("SF2 edit: added/removed metadata chunks unsupported".into());
    }
    for (a, b) in before.iter().zip(&after) {
        if a != b {
            return Err(format!(
                "SF2 edit: unsupported metadata or instrument edit in {}/{}",
                String::from_utf8_lossy(&b.0),
                String::from_utf8_lossy(&b.1)
            )
            .into());
        }
    }
    let before_modes = modes(baseline)?;
    let after_modes = modes(edited)?;
    let mut owned = BTreeSet::new();
    let mut editable = vec![false; edited.pcm24_pool().len()];
    let mut body = original.to_vec();
    let mut samples = Vec::new();
    for sample in &binding.samples {
        let end = sample
            .body_offset
            .checked_add(sample.encoded_bytes)
            .ok_or("SF2 edit: source sample range overflow")?;
        let source = original
            .get(sample.body_offset..end)
            .ok_or("SF2 edit: source sample exceeds body")?;
        if sha256_hex(source) != sample.sha256 {
            return Err("SF2 edit: source sample hash disagrees with binding".into());
        }
        let decoded = adpcm::decode_sample(source)?;
        let mut report = SampleReport {
            sample_id: sample.sample_id,
            sf2_sample: sample.sf2_sample,
            content_changed: false,
            encoded_changed: false,
            original_encoded_sha256: sample.sha256.clone(),
            output_encoded_sha256: sample.sha256.clone(),
            preserved_tail_bytes: decoded.trailing_bytes,
            replaced_original_tail_bytes: 0,
            pcm24_rounding_loss: None,
            pcm24_total_loss: None,
            encoding: None,
            loop_reconstruction: crate::sample::editing::LoopReport::preserved(decoded.sample_loop),
        };
        if let Some(index) = sample.sf2_sample {
            if !owned.insert(index) {
                return Err("SF2 edit: conflicting regenerated sample identities".into());
            }
            let header = edited
                .samples
                .get(index)
                .ok_or("SF2 edit: mapped sample index missing")?;
            let input = edited.sample_pcm24(index)?;
            let reference = baseline.sample_pcm24(index)?;
            if reference.len() != decoded.pcm.len()
                || reference
                    .iter()
                    .zip(&decoded.pcm)
                    .any(|(&a, &b)| a != i32::from(b) * 256)
            {
                return Err("SF2 edit: baseline PCM differs from preserved sample decode".into());
            }
            for point in &mut editable[header.start as usize..header.end as usize] {
                *point = true;
            }
            let mode = after_modes
                .get(&index)
                .copied()
                .unwrap_or(u16::from(header.loop_start != header.loop_end));
            let sample_loop = loop_range(edited, index, mode)?;
            if mode == 0
                && (header.loop_start, header.loop_end)
                    != (
                        baseline.samples[index].loop_start,
                        baseline.samples[index].loop_end,
                    )
            {
                return Err(format!("SF2 edit: sample {index}: unused loop-point edits are unrepresentable; preserve points when disabling looping").into());
            }
            let source_loop = decoded
                .sample_loop
                .map(|r| r.start_frame..r.end_frame_exclusive);
            report.content_changed = input != reference || sample_loop != source_loop;
            if report.content_changed {
                // Round to nearest PCM16, ties toward positive infinity, saturating
                // the positive endpoint. Report both this loss and final ADPCM loss.
                let pcm: Vec<i16> = input
                    .iter()
                    .map(|&v| ((v + 128).div_euclid(256)).clamp(-32768, 32767) as i16)
                    .collect();
                let rounded: Vec<i32> = pcm.iter().map(|&v| i32::from(v) * 256).collect();
                let rebuilt =
                    crate::sample::editing::rebuild(source, &pcm, sample_loop).map_err(|e| {
                        format!(
                            "SF2 edit sample {index} / VAB sample {}: {e}",
                            sample.sample_id
                        )
                    })?;
                let actual: Vec<i32> = adpcm::decode_sample(&rebuilt.bytes)?
                    .pcm
                    .into_iter()
                    .map(|v| i32::from(v) * 256)
                    .collect();
                if binding
                    .tones
                    .iter()
                    .any(|tone| tone.source_sf2_sample == index && !tone.stopped_keys.is_empty())
                    && actual.first().is_none_or(|first| {
                        crate::soundfont::envelope::snapshot::held_sample((first / 256) as i16) != 0
                    })
                {
                    return Err(format!("SF2 edit sample {index}: encoded first sample changes the held value of stopped-pitch keys; unsupported").into());
                }
                if actual.len() != input.len() {
                    return Err("SF2 edit: reconstructed duration changed".into());
                }
                report.encoded_changed = rebuilt.bytes != source;
                report.output_encoded_sha256 = sha256_hex(&rebuilt.bytes);
                report.preserved_tail_bytes = rebuilt.preserved_tail_bytes;
                report.replaced_original_tail_bytes = rebuilt.replaced_original_tail_bytes;
                report.pcm24_rounding_loss = Some(quantization::measure(input, &rounded));
                report.pcm24_total_loss = Some(quantization::measure(input, &actual));
                report.encoding = rebuilt.encoding;
                report.loop_reconstruction = rebuilt.loop_reconstruction;
                body[sample.body_offset..end].copy_from_slice(&rebuilt.bytes);
            }
        }
        samples.push(report);
    }
    // Guards, generated silence and unowned points must never be silently dropped.
    if baseline
        .pcm24_pool()
        .iter()
        .zip(edited.pcm24_pool())
        .zip(&editable)
        .any(|((&a, &b), &can_edit)| !can_edit && a != b)
    {
        return Err(
            "SF2 edit: generated silence, guard or unowned PCM edits are unsupported".into(),
        );
    }
    for (index, (a, b)) in baseline.samples.iter().zip(&edited.samples).enumerate() {
        let removed_silence = tone_controls
            && binding.silence_sample == Some(index)
            && !after_modes.contains_key(&index);
        if !owned.contains(&index)
            && ((a.loop_start, a.loop_end) != (b.loop_start, b.loop_end)
                || (!removed_silence && before_modes.get(&index) != after_modes.get(&index)))
        {
            return Err(
                "SF2 edit: generated playback-only sample loop edits are unsupported".into(),
            );
        }
    }
    Ok(Packed {
        body,
        report: Report {
            schema: "bof3.audio.sf2-sample-pack/v1",
            soundfont_byte_equal: baseline.original_bytes() == edited.original_bytes(),
            baseline_soundfont_sha256: sha256_hex(baseline.original_bytes()),
            input_soundfont_sha256: sha256_hex(edited.original_bytes()),
            samples,
            allocated_samples: Vec::new(),
            limitations: vec![
                "Original sample identities, PCM length/storage offsets and sample-header rate/pitch/names remain fixed. The enclosing bank inverse may validate appended mono samples and source-shaped instruments separately; new sample labels re-export as generated VAB allocation names.",
                "Only shared-consistent nonlooping/continuous loop modes are representable; release-only loops are unsupported. Loops must align to 28 frames and end at the sample end.",
                "PCM24 loss fields use signed 24-bit integer units; PCM16 conversion rounds nearest with ties toward positive infinity and saturation. Encoder loss fields use PCM16 units.",
                "ADPCM encoding uses the existing zero-history model and stabilizes edited loops; original runtime interpolation/mixing and PC/PSX fidelity remain unverified.",
            ],
        },
    })
}

fn loop_range(font: &Font, index: usize, mode: u16) -> Result<Option<Range<usize>>> {
    let sample = &font.samples[index];
    match mode {
        0 => Ok(None),
        1 => {
            let start = sample
                .loop_start
                .checked_sub(sample.start)
                .ok_or("SF2 edit: loop starts before sample")? as usize;
            let end = sample
                .loop_end
                .checked_sub(sample.start)
                .ok_or("SF2 edit: loop ends before sample")? as usize;
            if start >= end || sample.loop_end > sample.end {
                return Err("SF2 edit: loop outside sample or empty".into());
            }
            Ok(Some(start..end))
        }
        _ => Err(format!(
            "SF2 edit: sample {index}: loop mode {mode} is unsupported; release-only loops cannot be represented by SPU sample flags"
        ).into()),
    }
}
fn modes(font: &Font) -> Result<BTreeMap<usize, u16>> {
    let mut result = BTreeMap::new();
    for instrument in &font.instruments {
        let mut global = 0;
        for zone in &instrument.zones {
            let mode = zone
                .generators
                .iter()
                .find(|g| g.operator == 54)
                .map_or(global, |g| g.amount);
            if let Some(index) = zone.target {
                if let Some(previous) = result.insert(index, mode) {
                    if previous != mode {
                        return Err(format!(
                            "SF2 edit: conflicting loop modes for shared sample {index}"
                        )
                        .into());
                    }
                }
            } else {
                global = mode;
            }
        }
    }
    Ok(result)
}

type Control = ([u8; 4], [u8; 4], Vec<u8>, Option<u8>);

fn instrument_table(font: &Font, id: [u8; 4]) -> Result<&[u8]> {
    let chunk = font
        .chunks
        .iter()
        .find(|c| c.scope == *b"pdta" && c.id == id)
        .ok_or("SF2 key range: instrument table missing")?;
    Ok(&font.original_bytes()[chunk.data.clone()])
}

/// Rebuild only indices implied by the already validated instrument zones.
/// Names, ordered generator records and opaque terminal bytes stay exact.
fn instrument_tables(font: &Font, original: &Font) -> Result<[([u8; 4], Vec<u8>); 3]> {
    let mut inst = Vec::new();
    let mut ibag = Vec::new();
    let mut igen = Vec::new();
    let mut modulators = 0usize;
    for instrument in &font.instruments {
        inst.extend(instrument.name);
        inst.extend(u16::try_from(ibag.len() / 4)?.to_le_bytes());
        for zone in &instrument.zones {
            ibag.extend(u16::try_from(igen.len() / 4)?.to_le_bytes());
            ibag.extend(u16::try_from(modulators)?.to_le_bytes());
            modulators += zone.modulators.len();
            for generator in &zone.generators {
                igen.extend(generator.operator.to_le_bytes());
                igen.extend(generator.amount.to_le_bytes());
            }
        }
    }
    let raw = instrument_table(original, *b"inst")?;
    let mut terminal = raw[raw.len() - 22..].to_vec();
    terminal[20..22].copy_from_slice(&u16::try_from(ibag.len() / 4)?.to_le_bytes());
    inst.extend(terminal);
    let raw = instrument_table(original, *b"ibag")?;
    let mut terminal = raw[raw.len() - 4..].to_vec();
    if terminal[2..4] != u16::try_from(modulators)?.to_le_bytes() {
        return Err("SF2 key range: changed modulator layout unsupported".into());
    }
    terminal[..2].copy_from_slice(&u16::try_from(igen.len() / 4)?.to_le_bytes());
    ibag.extend(terminal);
    let raw = instrument_table(original, *b"igen")?;
    igen.extend(&raw[raw.len() - 4..]);
    Ok([(*b"inst", inst), (*b"ibag", ibag), (*b"igen", igen)])
}

fn validate_range_source(baseline: &Font) -> Result<()> {
    for (id, bytes) in instrument_tables(baseline, baseline)? {
        if instrument_table(baseline, id)? != bytes {
            return Err("SF2 key range: original raw instrument ownership differs".into());
        }
    }
    Ok(())
}

fn validate_range_destinations(
    binding: &binding::Report,
    body: &[u8],
    baseline: &Font,
    edited: &Font,
    assignments: &crate::soundfont::packing::assignment::Report,
) -> Result<()> {
    let source_modes = modes(baseline)?;
    for tone in &assignments.tones {
        let mode = *source_modes
            .get(&tone.original_sf2_sample)
            .ok_or("SF2 key range: original sample mode missing")?;
        let destination = binding
            .samples
            .iter()
            .find(|sample| sample.sf2_sample == Some(tone.output_sf2_sample))
            .ok_or("SF2 key range: destination is not an existing physical allocation")?;
        let end = destination
            .body_offset
            .checked_add(destination.encoded_bytes)
            .ok_or("SF2 key range: destination allocation overflow")?;
        let encoded = body
            .get(destination.body_offset..end)
            .ok_or("SF2 key range: destination allocation exceeds body")?;
        if encoded.is_empty() || sha256_hex(encoded) != destination.sha256 {
            return Err(
                "SF2 key range: destination allocation is empty or differs from binding".into(),
            );
        }
        let decoded = adpcm::decode_sample(encoded)?;
        let original_loop = decoded
            .sample_loop
            .map(|range| range.start_frame..range.end_frame_exclusive);
        if loop_range(edited, tone.output_sf2_sample, mode)? != original_loop {
            return Err(
                "SF2 key range: destination loop is incompatible with fixed source tone mode"
                    .into(),
            );
        }
    }
    Ok(())
}

fn range_pitch_raw(zone: &crate::soundfont::reader::model::Zone) -> [Option<u16>; 3] {
    [58, 51, 52].map(|operator| {
        zone.generators
            .iter()
            .find(|generator| generator.operator == operator)
            .map(|generator| generator.amount)
    })
}

fn range_pitch_values(zone: &crate::soundfont::reader::model::Zone) -> Result<(u8, [u16; 3], i32)> {
    let mut keys = zone
        .generators
        .iter()
        .filter(|generator| generator.operator == 43);
    let key = keys
        .next()
        .ok_or("SF2 key range: pitch key missing")?
        .amount;
    let [low, high] = key.to_le_bytes();
    if low != high || high > 127 || keys.next().is_some() {
        return Err("SF2 key range: pitch requires one singleton key".into());
    }
    let raw = range_pitch_raw(zone);
    let root = raw[0].ok_or("SF2 key range: explicit pitch root missing")?;
    let coarse = raw[1].ok_or("SF2 key range: explicit coarse pitch missing")?;
    let fine = raw[2].ok_or("SF2 key range: explicit fine pitch missing")?;
    if root > 127
        || !(-120..=120).contains(&(coarse as i16))
        || !(-99..=99).contains(&(fine as i16))
    {
        return Err("SF2 key range: pitch generators outside supported bounds".into());
    }
    let cents = -100 * i32::from(root) + 100 * i32::from(coarse as i16) + i32::from(fine as i16);
    Ok((low, [root, coarse, fine], cents))
}

fn validate_range_pitch(
    binding: &binding::Report,
    expected: &Font,
    edited: &Font,
    validated: &mut Font,
    pitches: &crate::soundfont::packing::pitch::Report,
) -> Result<()> {
    let mut seen = BTreeSet::new();
    for tone in &pitches.tones {
        let mut sources = binding.tones.iter().filter(|source| {
            source.sf2_instrument == tone.sf2_instrument
                && source.source_program == tone.program
                && source.source_tone.index == tone.tone
        });
        let source = sources
            .next()
            .ok_or("SF2 key range: unrelated pitch report")?;
        if sources.next().is_some() || !seen.insert(tone.sf2_instrument) {
            return Err("SF2 key range: duplicate pitch report identity".into());
        }
        if tone.original_center != source.source_tone.center
            || tone.original_shift != source.source_tone.shift
        {
            return Err("SF2 key range: original pitch provenance differs".into());
        }
        let before = expected
            .instruments
            .get(tone.sf2_instrument)
            .ok_or("SF2 key range: original pitch instrument missing")?;
        let after = edited
            .instruments
            .get(tone.sf2_instrument)
            .ok_or("SF2 key range: requested pitch instrument missing")?;
        let count = before.zones.len();
        let key_count = usize::from(source.source_tone.key_max)
            .checked_sub(usize::from(source.source_tone.key_min))
            .ok_or("SF2 key range: invalid pitch source range")?
            + 1;
        if count == 0
            || count != key_count
            || count != after.zones.len()
            || count != source.context.tuning.len()
            || count != tone.requested_cents.len()
            || count != tone.selected.len()
        {
            return Err("SF2 key range: pitch report/key lengths differ".into());
        }
        let rate = expected
            .samples
            .get(source.source_sf2_sample)
            .ok_or("SF2 key range: source pitch sample missing")?
            .rate;
        let mut changed = false;
        for index in 0..count {
            let (before_key, _, _) = range_pitch_values(&before.zones[index])?;
            let (key, raw, cents) = range_pitch_values(&after.zones[index])?;
            let row = &tone.selected[index];
            let source_row = &source.context.tuning[index];
            let expected_key = usize::from(source.source_tone.key_min) + index;
            let parameters = row
                .sf2
                .as_ref()
                .ok_or("SF2 key range: selected pitch is unrepresentable")?;
            let selected_cents = -100 * i32::from(parameters.root_key)
                + 100 * i32::from(parameters.coarse_tune)
                + i32::from(parameters.fine_tune);
            if usize::from(key) != expected_key
                || before_key != key
                || source_row.key != key
                || row.key != key
                || source_row.center != tone.original_center
                || source_row.shift != tone.original_shift
                || source_row.sf2_sample_rate != rate
                || row.sf2_sample_rate != rate
                || row.center != tone.output_center
                || row.shift != tone.output_shift
                || row.unsupported.is_some()
                || row.effective_step == 0
                || row.pitch_table_index.is_none()
                || !row.pitch_lookup.is_some_and(|lookup| {
                    matches!(
                        lookup.region,
                        crate::voice::tuning::LookupRegion::NoteOnPitchTable
                            | crate::voice::tuning::LookupRegion::EarlierSpuPitchTable
                    )
                })
                || parameters.root_key > 127
                || !(-120..=120).contains(&parameters.coarse_tune)
                || !(-99..=99).contains(&parameters.fine_tune)
                || cents != selected_cents
                || cents != tone.requested_cents[index]
            {
                return Err("SF2 key range: selected pitch/report context differs".into());
            }
            changed |= range_pitch_raw(&before.zones[index]) != raw.map(Some);
            for (operator, amount) in [58, 51, 52].into_iter().zip(raw) {
                validated.instruments[tone.sf2_instrument].zones[index]
                    .generators
                    .iter_mut()
                    .find(|generator| generator.operator == operator)
                    .ok_or("SF2 key range: original pitch generator missing")?
                    .amount = amount;
            }
        }
        if !changed {
            return Err("SF2 key range: unrelated unchanged pitch report".into());
        }
    }
    for source in &binding.tones {
        let before = &expected.instruments[source.sf2_instrument].zones;
        let after = &edited.instruments[source.sf2_instrument].zones;
        if before.len() != after.len() {
            return Err("SF2 key range: pitch instrument lengths differ".into());
        }
        if !seen.contains(&source.sf2_instrument)
            && before
                .iter()
                .zip(after)
                .any(|(a, b)| range_pitch_raw(a) != range_pitch_raw(b))
        {
            return Err("SF2 key range: changed pitch has no validated report".into());
        }
    }
    Ok(())
}

fn validate_range_tables(
    baseline: &Font,
    expected: &Font,
    edited: &Font,
    gains: &crate::soundfont::packing::tone::Report,
    pitch: (&binding::Report, &crate::soundfont::packing::pitch::Report),
    assignments: &crate::soundfont::packing::assignment::Report,
    envelopes: &crate::soundfont::packing::envelope::Report,
) -> Result<()> {
    let mut validated = Font::from_bytes(expected.original_bytes().to_vec())?;
    validated.instruments = expected.instruments.clone();
    for tone in gains.tones.iter().filter(|tone| tone.changed) {
        let parameters = tone
            .fit
            .as_ref()
            .and_then(|fit| fit.parameters)
            .ok_or("SF2 key range: validated audible gain fit missing")?;
        for zone in &mut validated.instruments[tone.sf2_instrument].zones {
            for generator in &mut zone.generators {
                match generator.operator {
                    17 => generator.amount = parameters.pan as u16,
                    48 => generator.amount = parameters.attenuation_cb,
                    _ => (),
                }
            }
        }
    }
    validate_range_pitch(pitch.0, expected, edited, &mut validated, pitch.1)?;
    for tone in &assignments.tones {
        let sample = u16::try_from(tone.output_sf2_sample)?;
        for zone in &mut validated.instruments[tone.sf2_instrument].zones {
            zone.target = Some(tone.output_sf2_sample);
            for generator in &mut zone.generators {
                if generator.operator == 53 {
                    generator.amount = sample;
                }
            }
        }
    }
    for tone in &envelopes.tones {
        let parameters = tone.fit.parameters.envelope();
        for zone in &mut validated.instruments[tone.sf2_instrument].zones {
            for generator in &mut zone.generators {
                generator.amount = match generator.operator {
                    34 => parameters.attack as u16,
                    36 => parameters.decay as u16,
                    37 => parameters.sustain_cb,
                    38 => parameters.release as u16,
                    _ => generator.amount,
                };
            }
        }
    }
    if validated
        .instruments
        .iter()
        .zip(&edited.instruments)
        .any(|(before, after)| {
            before
                .zones
                .iter()
                .zip(&after.zones)
                .any(|(before, after)| before.target != after.target)
        })
    {
        return Err("SF2 key range: sample target differs from validated assignment".into());
    }
    for (id, bytes) in instrument_tables(&validated, baseline)? {
        if instrument_table(edited, id)? != bytes {
            return Err(format!(
                "SF2 key range: instrument table {} differs from validated geometry or source bytes",
                String::from_utf8_lossy(&id)
            ).into());
        }
    }
    Ok(())
}

fn controls(
    font: &Font,
    tone_controls: bool,
    program_edits: bool,
    instrument_prefix: Option<&Font>,
    sample_prefix: Option<&Font>,
    range_prefix: Option<&Font>,
) -> Result<Vec<Control>> {
    if !matches!(font.version, (2, 1) | (2, 4)) {
        return Err("SF2 edit: only 2.01/2.04 PCM interchange is supported".into());
    }
    let mut result = Vec::new();
    for chunk in &font.chunks {
        let raw = &font.original_bytes()[chunk.data.clone()];
        if program_edits
            && chunk.scope == *b"pdta"
            && matches!(&chunk.id, b"phdr" | b"pbag" | b"pgen")
        {
            continue;
        }
        if chunk.scope == *b"sfbk"
            && chunk.id == *b"LIST"
            && raw.len() >= 4
            && [b"INFO", b"sdta", b"pdta"].contains(&raw[..4].try_into().unwrap())
        {
            continue;
        }
        if chunk.scope == *b"sdta" && matches!(&chunk.id, b"smpl" | b"sm24") {
            if chunk.id == *b"sm24"
                && !font.pcm24_pool().len().is_multiple_of(2)
                && raw.last() != Some(&0)
            {
                return Err("SF2 edit: nonzero sm24 alignment byte unsupported".into());
            }
            continue;
        }
        let mut data = raw.to_vec();
        if let Some(prefix) = range_prefix {
            if chunk.scope == *b"pdta" && matches!(&chunk.id, b"inst" | b"ibag" | b"igen") {
                data = instrument_table(prefix, chunk.id)?.to_vec();
            }
        }
        if let Some(prefix) = instrument_prefix {
            let width = match &chunk.id {
                b"inst" => Some(22),
                b"ibag" | b"igen" => Some(4),
                _ => None,
            };
            if chunk.scope == *b"pdta" {
                if let Some(width) = width {
                    let original = prefix
                        .chunks
                        .iter()
                        .find(|c| c.scope == chunk.scope && c.id == chunk.id)
                        .ok_or("SF2 independent tone: original instrument table missing")?;
                    let end = original.data.len() - width;
                    data = raw
                        .get(..end)
                        .ok_or("SF2 independent tone: original instrument table truncated")?
                        .to_vec();
                    let mut terminal = raw[raw.len() - width..].to_vec();
                    match &chunk.id {
                        b"inst" => terminal[20..22].fill(0),
                        b"ibag" => terminal[..2].fill(0),
                        _ => (),
                    }
                    data.extend(terminal);
                }
            }
        }
        if chunk.scope == *b"INFO" && chunk.id == *b"ifil" {
            data = vec![2, 0, 1, 0];
        }
        if chunk.scope == *b"pdta" && chunk.id == *b"shdr" {
            if let Some(prefix) = sample_prefix {
                let end = prefix.samples.len() * 46;
                data = raw
                    .get(..end)
                    .ok_or("SF2 allocation: original sample table truncated")?
                    .to_vec();
                data.extend(&raw[raw.len() - 46..]);
            }
            for row in data
                .as_chunks_mut::<46>()
                .0
                .iter_mut()
                .take(sample_prefix.map_or(font.samples.len(), |p| p.samples.len()))
            {
                row[28..36].fill(0);
            }
        }
        if chunk.scope == *b"pdta" && chunk.id == *b"igen" {
            let count = data.len() / 4 - 1;
            for row in data.as_chunks_mut::<4>().0.iter_mut().take(count) {
                let operator = u16::from_le_bytes([row[0], row[1]]);
                if operator == 54
                    || (tone_controls && matches!(operator, 17 | 33..=38 | 48 | 51 | 52 | 53 | 58))
                {
                    row[2..].fill(0);
                }
            }
        }
        let padding = if raw.len().is_multiple_of(2) {
            None
        } else {
            Some(font.original_bytes()[chunk.data.end])
        };
        result.push((chunk.scope, chunk.id, data, padding));
    }
    Ok(result)
}
