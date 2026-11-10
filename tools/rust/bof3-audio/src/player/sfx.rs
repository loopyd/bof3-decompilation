//! Source-qualified embedded effects banks for the existing development player.
use crate::{
    machine::{bank::Plan, image::executable::Executable, reference::mapping},
    publication, Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use serde_json::{json, Value};
use std::{fmt::Write, path::Path};

#[derive(Serialize)]
struct ToneBinding {
    program: u8,
    tone: usize,
    key: u8,
    program_reference: i16,
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
            writeln!(assembly, ".balign 64\n.globl player_bank{index}_{extension}\nplayer_bank{index}_{extension}: .incbin \"{filename}\"")?;
            writeln!(
                declarations,
                "extern const u8 player_bank{index}_{extension}[];"
            )?;
        }
        let mut samples = Vec::new();
        let mut selections = Vec::new();
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
            selections.push(selected.map_or_else(
                || "{0, 0, 0, 0}".to_owned(),
                |candidate| {
                    format!(
                        "{{1, {}, {}, {}}}",
                        candidate.program, candidate.tone, candidate.key
                    )
                },
            ));
            samples.push(json!({
                "sample": sample, "tone_candidates": candidates,
                "playback": if selected.is_some() { "explicit_sdk_note" } else { "unresolved" },
                "cue_binding": "unresolved", "vocal_role": "unresolved",
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
        reports.push(json!({
            "source": name, "bank": plan.identity, "auxiliary_entry": auxiliary_entry,
            "auxiliary_bytes": auxiliary.len(), "samples": samples,
            "diagnostics": plan.bank.diagnostics,
            "auxiliary_records": auxiliary.as_chunks::<4>().0,
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
