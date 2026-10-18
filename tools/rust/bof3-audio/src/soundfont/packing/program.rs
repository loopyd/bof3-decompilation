//! Rebuild VAB program layers from explicitly assigned, validated tone instruments.
use crate::{bank::Bank, soundfont::bank as binding, soundfont::reader::Font, Result};
use serde::Serialize;
use std::collections::BTreeMap;

#[derive(Debug, Serialize)]
/// One program's replacement layers; an empty list clears a populated slot.
pub struct Population {
    pub program: u8,
    pub tones: Vec<Tone>,
}
#[derive(Debug, Serialize)]
pub struct Tone {
    pub source_program: u8,
    pub source_tone: usize,
    pub sf2_instrument: usize,
}

pub(crate) fn derive(
    binding: &binding::Report,
    baseline: &Font,
    edited: &Font,
) -> Result<Vec<Population>> {
    if baseline.presets.len() != edited.presets.len() {
        return Err("SF2 program edit: preset insertion/removal changes program identities".into());
    }
    let programs: Vec<_> = binding
        .programs
        .iter()
        .map(|p| (p.source_program, p.sf2_preset, p.percussion_preset))
        .chain(
            binding
                .empty_programs
                .iter()
                .map(|p| (p.source_program, p.sf2_preset, p.percussion_preset)),
        )
        .collect();
    let mut changed = false;
    for (index, (before, after)) in baseline.presets.iter().zip(&edited.presets).enumerate() {
        let mut normalized = after.clone();
        normalized.zones = before.zones.clone();
        if normalized != *before {
            return Err(format!(
                "SF2 program edit: preset {index} identity/metadata edits are unsupported"
            )
            .into());
        }
        if before.zones != after.zones {
            changed = true;
            if !programs
                .iter()
                .any(|&(_, preset, alias)| preset == index || alias == Some(index))
            {
                return Err(
                    format!("SF2 program edit: preset {index} has no source program").into(),
                );
            }
        }
    }
    if !changed {
        return Ok(Vec::new());
    }
    // The parsed model covers every nonterminal row. Do not discard opaque
    // terminal fields when authorizing the three resized preset tables.
    for (id, width) in [(*b"phdr", 38), (*b"pbag", 4), (*b"pgen", 4)] {
        let terminal = |font: &Font| -> Result<Vec<u8>> {
            let chunk = font
                .chunks
                .iter()
                .find(|c| c.scope == *b"pdta" && c.id == id)
                .ok_or("SF2 program edit: missing table")?;
            let mut row = font.original_bytes()[chunk.data.end - width..chunk.data.end].to_vec();
            match &id {
                b"phdr" => row[24..26].fill(0),
                b"pbag" => row[..2].fill(0),
                _ => (),
            }
            Ok(row)
        };
        if terminal(baseline)? != terminal(edited)? {
            return Err("SF2 program edit: opaque preset terminal fields changed".into());
        }
    }
    let mut output = Vec::new();
    for (program, preset, alias) in programs {
        let before = &baseline.presets[preset];
        let after = &edited.presets[preset];
        if let Some(alias) = alias {
            if edited.presets[alias].zones != after.zones {
                let action = if binding.programs.iter().any(|p| p.source_program == program) {
                    "replacing populated"
                } else {
                    "populating empty"
                };
                return Err(format!("SF2 program edit: {action} preset {preset}: melodic and percussion alias assignments must agree").into());
            }
        }
        if before.zones == after.zones {
            continue;
        }
        if after.zones.is_empty() || after.zones.len() > 16 {
            return Err(format!("SF2 program {}: VAB requires 1..=16 tones", program).into());
        }
        let mut tones = Vec::new();
        for zone in &after.zones {
            let target = zone
                .target
                .ok_or("SF2 program edit: global preset zones are unsupported")?;
            if !zone.modulators.is_empty()
                || zone.generators.len() != 1
                || zone.generators[0].operator != 41
                || usize::from(zone.generators[0].amount) != target
            {
                return Err("SF2 program edit: use direct tone instrument assignments without preset generators/modulators".into());
            }
            if after.zones.len() == 1 && binding.empty_program_instrument == Some(target) {
                continue;
            }
            let source = binding.tones.iter().find(|t| t.sf2_instrument == target).ok_or("SF2 program edit: assignment requires an existing VAB tone instrument; generated silence and new instrument structures are not tone templates")?;
            tones.push(Tone {
                source_program: source.source_program,
                source_tone: source.source_tone.index,
                sf2_instrument: target,
            });
        }
        output.push(Population { program, tones });
    }
    Ok(output)
}

/// Existing tone edits are already inverted. Copy their resulting raw rows and
/// retain unused destination rows, unrelated blocks and the sample-size table/tail.
pub(crate) fn apply(header: &[u8], populations: &[Population]) -> Result<Vec<u8>> {
    if populations.is_empty() {
        return Ok(header.to_vec());
    }
    let bank = Bank::parse(header)?;
    if bank.programs.len() != usize::from(bank.declared_programs)
        || bank.programs.iter().map(|p| p.tones.len()).sum::<usize>()
            != usize::from(bank.declared_tones)
    {
        return Err("SF2 program edit: inconsistent original VAB program/tone totals".into());
    }
    let replaced: usize = populations
        .iter()
        .filter_map(|p| bank.programs.iter().find(|old| old.program == p.program))
        .map(|p| p.tones.len())
        .sum();
    let added = populations
        .iter()
        .filter(|p| !bank.programs.iter().any(|old| old.program == p.program))
        .count();
    let cleared = populations.iter().filter(|p| p.tones.is_empty()).count();
    let programs = bank.programs.len() + added - cleared;
    let tones = usize::from(bank.declared_tones) - replaced
        + populations.iter().map(|p| p.tones.len()).sum::<usize>();
    if programs > 128 || tones > 2048 {
        return Err("SF2 program edit: VAB program/tone capacity exceeded".into());
    }
    let mut blocks = BTreeMap::new();
    for program in &bank.programs {
        let offset = 0x820 + program.tone_block * 512;
        blocks.insert(program.program, header[offset..offset + 512].to_vec());
    }
    let mut output = header[..0x820].to_vec();
    for population in populations {
        if population.tones.is_empty() {
            blocks
                .remove(&population.program)
                .ok_or("SF2 program edit: cleared destination is not populated")?;
            output[0x20 + usize::from(population.program) * 16] = 0;
            continue;
        }
        let first = bank
            .programs
            .iter()
            .find(|p| p.program == population.tones[0].source_program)
            .ok_or("SF2 program edit: source program missing")?;
        let first_attr = &header[0x20 + usize::from(first.program) * 16..][..16];
        let mut block = blocks
            .get(&population.program)
            .cloned()
            .unwrap_or_else(|| vec![0; 512]);
        for (index, tone) in population.tones.iter().enumerate() {
            let source = bank
                .programs
                .iter()
                .find(|p| p.program == tone.source_program)
                .ok_or("SF2 program edit: source program missing")?;
            let attr = &header[0x20 + usize::from(source.program) * 16..][..16];
            if attr[1..5] != first_attr[1..5] {
                return Err(format!("SF2 program {}: layered templates must share VAB program volume, priority, mode and pan", population.program).into());
            }
            if tone.source_tone >= source.tones.len() {
                return Err("SF2 program edit: source tone missing".into());
            }
            let offset = 0x820 + source.tone_block * 512 + tone.source_tone * 32;
            block[index * 32..index * 32 + 32].copy_from_slice(&header[offset..offset + 32]);
            block[index * 32 + 20..index * 32 + 22]
                .copy_from_slice(&u16::from(population.program).to_le_bytes());
        }
        let attr = &mut output[0x20 + usize::from(population.program) * 16..][..16];
        attr[0] = population.tones.len() as u8;
        attr[1..5].copy_from_slice(&first_attr[1..5]);
        blocks.insert(population.program, block);
    }
    for block in blocks.values() {
        output.extend(block);
    }
    output.extend(&header[0x820 + bank.programs.len() * 512..]);
    output[0x12..0x14].copy_from_slice(&(programs as u16).to_le_bytes());
    output[0x14..0x16].copy_from_slice(&(tones as u16).to_le_bytes());
    let size = if output.len() >= header.len() {
        bank.declared_file_bytes
            .checked_add((output.len() - header.len()) as u32)
    } else {
        bank.declared_file_bytes
            .checked_sub((header.len() - output.len()) as u32)
    }
    .ok_or("SF2 program edit: declared file size outside supported range")?;
    output[0x0c..0x10].copy_from_slice(&size.to_le_bytes());
    let parsed = Bank::parse(&output)?;
    if parsed.programs.len() != programs
        || parsed.declared_tones as usize != tones
        || parsed.declared_samples != bank.declared_samples
    {
        return Err("SF2 program edit: rebuilt VAB validation failed".into());
    }
    Ok(output)
}
