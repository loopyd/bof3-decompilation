//! Source-qualified embedded effects banks for the existing development player.
use crate::{
    Result,
    machine::{bank::Plan, bus::Ram, cues, image::executable::Executable, reference::mapping},
    publication,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use serde_json::{Value, json};
use std::{fmt::Write, path::Path};

#[derive(Serialize)]
struct ToneBinding {
    program: u8,
    tone: usize,
    key: u8,
    program_reference: i16,
}

#[derive(Serialize)]
struct CueBinding {
    cue: usize,
    program: u8,
    sample: u16,
    layers: Vec<CueLayer>,
}

#[derive(Serialize)]
struct CueLayer {
    voice: usize,
    tone: usize,
}

fn bind_cue(
    plan: &Plan<'_>,
    lookup: &[u8; 256],
    row: usize,
    record: &[u8; 4],
) -> std::result::Result<CueBinding, &'static str> {
    let (cue, program) = cues::local_cue(record, &plan.bank)?;
    let half = |bytes: &[u8], at| u16::from_le_bytes([bytes[at], bytes[at + 1]]);
    let program_offset = usize::from(half(lookup, usize::from(program.program) * 2));
    let top_tone = cue.first_tone + cue.extra_layers;
    for tone in cue.first_tone..=top_tone {
        let attribute = 0x820 + program_offset + usize::from(half(lookup, 8 + tone * 2));
        if attribute + 7 > plan.header.len() {
            return Err("original attribute lookup exceeds the loaded VH payload");
        }
    }

    let mut sample = None;
    let mut layers = Vec::new();
    let mut silent_layer = false;
    // The original handler repeats the highest tone on every added voice.
    for layer in (0..=cue.extra_layers).rev() {
        let tone = if layer == 0 { cue.first_tone } else { top_tone };
        let offset = 0x820 + program.tone_block * 512 + tone * 32;
        let id = half(plan.header, offset + 22);
        if id > plan.bank.declared_samples {
            return Err("physical tone references a sample outside the loaded bank");
        }
        if id == 0 {
            silent_layer = true;
            continue;
        }
        if half(plan.header, offset + 20) != u16::from(program.program) {
            return Err("physical tone has a different program reference");
        }
        if sample.is_some_and(|previous| previous != id) {
            return Err("cue layers select different physical samples");
        }
        sample = Some(id);
        layers.push(CueLayer {
            voice: (usize::from(cue.first_voice) + layer) % 24,
            tone,
        });
    }
    if silent_layer {
        return Err("cue contains a silent SDK layer");
    }
    let sample = sample.ok_or("cue contains no physical sample")?;
    let allocation = &plan.bank.samples[usize::from(sample - 1)];
    if allocation.encoded_bytes == 0 || !allocation.encoded_bytes.is_multiple_of(16) {
        return Err("cue sample lacks nonempty whole ADPCM blocks");
    }
    Ok(CueBinding {
        cue: (usize::from(plan.identity.game_bank_id) << 8) | row,
        program: program.program,
        sample,
        layers,
    })
}

pub fn stage(
    output: &Path,
    executable: &Executable,
    archives: &[ArchiveImage],
    plans: &[Plan<'_>],
    names: &[String],
) -> Result<Value> {
    let mut assembly = String::new();
    let mut declarations = String::new();
    let mut descriptors = Vec::new();
    let mut reports = Vec::new();
    let lookup = mapping::read_cue_lookup(&mut Ram::from_executable(executable))?;
    for (index, ((archive, plan), name)) in archives.iter().zip(plans).zip(names).enumerate() {
        if plan.bank.samples.is_empty() {
            return Err("compile SFX requires at least one physical sample per bank".into());
        }
        if archive.entries()[plan.identity.body_entry].ram_ptr
            != archive.entries()[plan.identity.header_entry].ram_ptr
        {
            return Err("compile SFX body entry selects a different bank slot".into());
        }
        let auxiliary_entry = super::unique(archive, &[8])?;
        if auxiliary_entry <= plan.identity.header_entry
            || archive.entries()[plan.identity.header_entry + 1..auxiliary_entry]
                .iter()
                .any(|entry| entry.file_type == 6)
            || archive.entries()[auxiliary_entry].ram_ptr
                != archive.entries()[plan.identity.header_entry].ram_ptr
        {
            return Err("compile SFX auxiliary entry is not owned by the selected bank".into());
        }
        let auxiliary = archive.entry(auxiliary_entry)?;
        let slot = &plan.identity.slot;
        let layout = mapping::initialize_layout(executable, plan.identity.layout)?;
        let next = layout
            .slots
            .iter()
            .map(|slot| slot.auxiliary_address)
            .filter(|&address| address > slot.auxiliary_address)
            .min()
            .unwrap_or(0x80148a04)
            .min(0x80148a04);
        if slot.auxiliary_address >= next
            || auxiliary.is_empty()
            || auxiliary.len() % 4 != 0
            || auxiliary.len() > 1024
            || auxiliary.len() > (next - slot.auxiliary_address) as usize
        {
            return Err("compile SFX auxiliary data exceeds its original layout capacity".into());
        }
        for (extension, bytes) in [
            ("vh", plan.header),
            ("vb", plan.transfer),
            ("aux", auxiliary),
        ] {
            let filename = format!("bank{index}.{extension}");
            publication::write_new(&output.join(&filename), bytes)?;
            writeln!(
                assembly,
                ".balign 64\n.globl player_bank{index}_{extension}\nplayer_bank{index}_{extension}: .incbin \"{filename}\""
            )?;
            writeln!(
                declarations,
                "extern const u8 player_bank{index}_{extension}[];"
            )?;
        }
        let mut samples = Vec::new();
        let mut selections = Vec::new();
        let records = auxiliary.as_chunks::<4>().0;
        let rows: Vec<_> = records
            .iter()
            .enumerate()
            .map(|(row, record)| bind_cue(plan, &lookup, row, record))
            .collect();
        for sample in &plan.bank.samples {
            let candidates: Vec<_> = plan
                .bank
                .programs
                .iter()
                .flat_map(|program| {
                    program.tones.iter().filter_map(move |tone| {
                        (tone.sample_reference == sample.sample_id as i16).then_some(ToneBinding {
                            program: program.program,
                            tone: tone.index,
                            key: tone.center,
                            program_reference: tone.program_reference,
                        })
                    })
                })
                .collect();
            let selected = match candidates.as_slice() {
                [candidate]
                    if sample.encoded_bytes > 0
                        && sample.encoded_bytes % 16 == 0
                        && candidate.key <= 127
                        && candidate.program_reference == i16::from(candidate.program) =>
                {
                    Some(candidate)
                }
                _ => None,
            };
            let cue_candidates: Vec<_> = rows
                .iter()
                .filter_map(|row| row.as_ref().ok())
                .filter(|binding| binding.sample == sample.sample_id)
                .collect();
            let (selection, playback, binding) = match cue_candidates.as_slice() {
                [cue] => {
                    let voices = cue
                        .layers
                        .iter()
                        .fold(0u32, |mask, layer| mask | 1 << layer.voice);
                    (
                        format!("{{SFX_GAME_CUE, 0, 0, 0, {}, {voices}}}", cue.cue),
                        "original_game_cue",
                        "qualified",
                    )
                }
                [] => match selected {
                    Some(candidate) => (
                        format!(
                            "{{SFX_SDK_NOTE, {}, {}, {}, 0, 1}}",
                            candidate.program, candidate.tone, candidate.key
                        ),
                        "explicit_sdk_note",
                        "unresolved",
                    ),
                    None => (
                        "{SFX_UNRESOLVED, 0, 0, 0, 0, 0}".into(),
                        "unresolved",
                        "unresolved",
                    ),
                },
                _ => (
                    "{SFX_AMBIGUOUS, 0, 0, 0, 0, 0}".into(),
                    "unresolved",
                    "ambiguous",
                ),
            };
            selections.push(selection);
            samples.push(json!({
                "sample": sample, "tone_candidates": candidates,
                "playback": playback, "cue_binding": binding, "vocal_role": "unresolved",
                "cue_candidates": cue_candidates,
            }));
        }
        writeln!(
            declarations,
            "static const struct SfxSample player_samples{index}[] = {{{}}};",
            selections.join(",")
        )?;
        descriptors.push(format!(
            "{{{{{}u, {}u, {}u, {}u, {}u, {}u, {}u, player_bank{index}_vh, player_bank{index}_vb, player_bank{index}_aux, {}u}}, {}u, player_samples{index}, {}}}",
            plan.identity.layout, slot.vab_id, slot.spu_base, slot.header_address,
            plan.header.len(), plan.identity.body_bytes, slot.auxiliary_address,
            auxiliary.len(), plan.bank.samples.len(), super::literal(name),
        ));
        let cue_rows: Vec<_> = rows
            .iter()
            .enumerate()
            .map(|(row, binding)| match binding {
                Ok(binding) => json!({"row": row, "binding": binding}),
                Err(reason) => json!({"row": row, "unresolved": reason}),
            })
            .collect();
        reports.push(json!({
            "source": name, "bank": plan.identity, "auxiliary_entry": auxiliary_entry,
            "auxiliary_bytes": auxiliary.len(), "samples": samples,
            "diagnostics": plan.bank.diagnostics,
            "auxiliary_records": records, "cue_rows": cue_rows,
            "audition_context": {"voice": 0, "key": "tone_center", "fine": 0, "left": 127, "right": 127},
        }));
    }
    publication::write_new(&output.join("banks.s"), assembly.as_bytes())?;
    writeln!(
        declarations,
        "static const struct SfxBank player_banks[] = {{{}}};",
        descriptors.join(",")
    )?;
    publication::write_new(&output.join("banks.h"), declarations.as_bytes())?;
    Ok(json!(reports))
}
