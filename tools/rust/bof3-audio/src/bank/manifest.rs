//! Validate bank extraction metadata against its preserved EMI and linked runtime.
use crate::{
    bank::extraction::reference_pitch, bank::Bank, catalog::model::Asset,
    catalog::model::AssetData, catalog::model::Catalog, document::manifest::Element, Result,
};
use serde_json::json;
use std::collections::BTreeSet;

pub(crate) struct Validated<'a> {
    pub asset: &'a Asset,
    pub bank: &'a Bank,
    pub body_entry: usize,
    pub rate: u32,
}

pub(crate) fn validate<'a>(
    node: &Element,
    catalog: &'a Catalog,
    mapping: &crate::catalog::model::Report,
) -> Result<Validated<'a>> {
    if node.name != "bank" {
        return Err("pack: expected <bank> manifest".into());
    }
    node.shape(
        &[
            "schema",
            "id",
            "source",
            "header_entry",
            "body_entry",
            "game_bank_id",
            "content_type",
        ],
        &[
            "runtime",
            "export",
            "loader",
            "metadata",
            "samples",
            "preservation",
        ],
        false,
    )?;
    let asset = catalog.select("audio", Some("bank"), node.attribute("id")?)?;
    let AssetData::Bank {
        metadata: bank,
        content: Some(content),
        ..
    } = &asset.data
    else {
        return Err("pack: bank requires exactly one verified, sufficiently sized body".into());
    };
    let body_entry = catalog.sources[0]
        .entries
        .iter()
        .find(|e| e.id == content.body_entry)
        .ok_or("pack: mapped body missing")?
        .entry;
    node.scalars(
        &json!({
            "schema": "bof3.audio-bank/v1", "id": asset.id, "source": asset.source,
            "header_entry": asset.entry, "body_entry": body_entry,
            "game_bank_id": asset.game_bank_id, "content_type": "unresolved",
        }),
        &[],
        &[],
    )?;
    let profile = &mapping.profile;
    let runtime = node.child("runtime")?;
    runtime.shape(
        &[
            "profile",
            "executable_sha256",
            "pitch_entry",
            "pitch_table",
            "playback_context",
        ],
        &[],
        false,
    )?;
    runtime.scalars(
        &json!({
            "profile": profile.id, "executable_sha256": profile.exe_sha256,
            "pitch_entry": profile.pitch_entry, "pitch_table": profile.pitch_table,
            "playback_context": "unresolved",
        }),
        &[],
        &[],
    )?;
    let export = node.child("export")?;
    let rate = export.number::<u32>("reference_rate")?;
    export.shape(
        &[
            "reference_rate",
            "reference_pitch_register",
            "midi_unity_note",
            "channels",
            "rate_context",
        ],
        &[],
        false,
    )?;
    export.scalars(
        &json!({
            "reference_rate": rate, "reference_pitch_register": reference_pitch(rate)?,
            "midi_unity_note": 60, "channels": 1, "rate_context": "export_reference_only",
        }),
        &[],
        &[],
    )?;
    validate_loader(node.child("loader")?, asset, catalog, mapping)?;
    validate_metadata(node.child("metadata")?, bank)?;
    let samples = node.child("samples")?;
    samples.shape(&[], &["sample"], false)?;
    if samples.children.len() != bank.samples.len() {
        return Err("pack: missing or additional samples are unsupported".into());
    }
    let mut seen = BTreeSet::new();
    for sample in &samples.children {
        let id = sample.number::<u16>("sample_id")?;
        if !seen.insert(id) || !bank.samples.iter().any(|s| s.sample_id == id) {
            return Err(format!("pack: duplicate or unknown sample {id}").into());
        }
    }
    node.child("preservation")?;
    Ok(Validated {
        asset,
        bank,
        body_entry,
        rate,
    })
}

fn validate_metadata(node: &Element, bank: &Bank) -> Result<()> {
    node.scalars(bank, &["programs", "samples", "diagnostics"], &[])?;
    node.shape(
        &node
            .attributes
            .keys()
            .map(String::as_str)
            .collect::<Vec<_>>(),
        &["program", "diagnostic"],
        false,
    )?;
    let programs = node
        .children
        .iter()
        .filter(|n| n.name == "program")
        .collect::<Vec<_>>();
    if programs.len() != bank.programs.len() {
        return Err("pack: edited program count unsupported".into());
    }
    let mut seen = BTreeSet::new();
    for program in programs {
        let id = program.number::<u8>("program")?;
        let expected = bank
            .programs
            .iter()
            .find(|p| p.program == id)
            .ok_or("pack: unknown program")?;
        if !seen.insert(id) {
            return Err("pack: duplicate program".into());
        }
        program.scalars(expected, &["tones"], &[])?;
        program.shape(
            &program
                .attributes
                .keys()
                .map(String::as_str)
                .collect::<Vec<_>>(),
            &["tone"],
            false,
        )?;
        if program.children.len() != expected.tones.len() {
            return Err("pack: edited tone count unsupported".into());
        }
        let mut tones = BTreeSet::new();
        for tone in &program.children {
            let index = tone.number::<usize>("index")?;
            let original = expected
                .tones
                .iter()
                .find(|t| t.index == index)
                .ok_or("pack: unknown tone")?;
            if !tones.insert(index) {
                return Err("pack: duplicate tone".into());
            }
            tone.scalars(original, &[], &[])?;
            tone.shape(
                &tone
                    .attributes
                    .keys()
                    .map(String::as_str)
                    .collect::<Vec<_>>(),
                &[],
                false,
            )?;
        }
    }
    let mut diagnostics = Vec::new();
    for diagnostic in node.children.iter().filter(|n| n.name == "diagnostic") {
        diagnostic.shape(&["message"], &[], false)?;
        diagnostics.push(diagnostic.attribute("message")?);
    }
    let mut expected = bank
        .diagnostics
        .iter()
        .map(String::as_str)
        .collect::<Vec<_>>();
    diagnostics.sort();
    expected.sort();
    if diagnostics != expected {
        return Err("pack: edited bank diagnostics unsupported".into());
    }
    Ok(())
}

fn validate_loader(
    node: &Element,
    asset: &Asset,
    catalog: &Catalog,
    mapping: &crate::catalog::model::Report,
) -> Result<()> {
    let association = mapping
        .associations
        .iter()
        .find(|a| a.bank == asset.id)
        .ok_or("pack: loader association missing")?;
    node.shape(
        &["load_slot", "active_layout"],
        &["possible_layout", "auxiliary", "song"],
        false,
    )?;
    node.scalars(
        &json!({"load_slot": association.load_slot, "active_layout": "unresolved"}),
        &[],
        &[],
    )?;
    let mut layouts = BTreeSet::new();
    let mut auxiliary = BTreeSet::new();
    let mut songs = BTreeSet::new();
    for child in &node.children {
        match child.name.as_str() {
            "possible_layout" => {
                let selector = child.number::<u8>("selector")?;
                let layout = mapping
                    .layouts
                    .iter()
                    .find(|l| l.selector == selector)
                    .ok_or("pack: unknown layout")?;
                if !layouts.insert(selector) {
                    return Err("pack: duplicate possible layout".into());
                }
                child.scalars(
                    &layout.slots[association.load_slot as usize],
                    &[],
                    &["selector"],
                )?;
                child.shape(
                    &child
                        .attributes
                        .keys()
                        .map(String::as_str)
                        .collect::<Vec<_>>(),
                    &[],
                    false,
                )?;
            }
            "auxiliary" => {
                child.shape(&["id"], &[], false)?;
                let id = child.attribute("id")?;
                if !auxiliary.insert(id) || !association.auxiliary_entries.iter().any(|a| a == id) {
                    return Err("pack: duplicate or unknown auxiliary identity".into());
                }
            }
            "song" => {
                child.shape(&["id"], &["game_song"], false)?;
                let id = child.attribute("id")?;
                if !songs.insert(id) || !association.songs.iter().any(|s| s == id) {
                    return Err("pack: duplicate or unknown song identity".into());
                }
                let song = catalog
                    .assets
                    .iter()
                    .find(|a| a.id == id)
                    .ok_or("pack: song missing")?;
                let mut cues = BTreeSet::new();
                for cue in &child.children {
                    cue.shape(&["id"], &[], false)?;
                    if !cues.insert(cue.number::<u16>("id")?) {
                        return Err("pack: duplicate game song ID".into());
                    }
                }
                if cues != song.game_song_ids.iter().copied().collect() {
                    return Err("pack: edited game song IDs unsupported".into());
                }
            }
            _ => unreachable!(),
        }
    }
    if layouts.len() != mapping.layouts.len()
        || auxiliary.len() != association.auxiliary_entries.len()
        || songs.len() != association.songs.len()
    {
        return Err("pack: incomplete loader metadata".into());
    }
    Ok(())
}
