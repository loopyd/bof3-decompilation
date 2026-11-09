//! Qualified effects references and directly observed XA content.
//! Roles do not establish active context or accepted execution of a cue.

use super::model::{AssetData, Association, Catalog, Layout, Role, RoleEvidence, SampleReference};
use crate::machine::executable::Executable;
use std::collections::BTreeMap;

const DISPATCH: u32 = 0x8015_df18;
const HANDLERS: [u32; 7] = [
    0x8015_e994,
    0x8015_efac,
    0x8015_f5c8,
    0x8015_fbe4,
    0x8016_0200,
    0x8016_081c,
    0x8016_0e38,
];

pub(super) fn resolve(
    catalog: &mut Catalog,
    associations: &[Association],
    layouts: &[Layout],
    target: &'static str,
    executable: &Executable,
) {
    let load_address = executable.header().load_address;
    for asset in &mut catalog.assets {
        let AssetData::Bank {
            metadata, content, ..
        } = &asset.data
        else {
            continue;
        };
        asset.roles.clear();
        asset.content_type = "unresolved";
        let Some(association) = associations.iter().find(|item| item.bank == asset.id) else {
            continue;
        };
        let [auxiliary] = association.auxiliary_entries.as_slice() else {
            continue;
        };
        let Some(entry) = catalog
            .sources
            .iter()
            .flat_map(|source| &source.entries)
            .find(|entry| &entry.id == auxiliary)
        else {
            continue;
        };
        let Some(records) = &entry.auxiliary_records else {
            continue;
        };
        let slot = usize::from(association.game_bank_id);
        if content.is_none()
            || layouts.is_empty()
            || layouts.iter().any(|layout| {
                let start = layout.slots[slot].auxiliary_address;
                let end = layout
                    .slots
                    .get(slot + 1)
                    .map_or(0x8014_8a04, |next| next.auxiliary_address);
                start.checked_add(entry.bytes).is_none_or(|last| last > end)
            })
        {
            continue;
        }
        let reference_rows: Vec<_> = records
            .iter()
            .enumerate()
            .filter(|(_, record)| {
                record[0] == 0
                    && record[3] & 0x80 == 0
                    && record[3] & 31 < 24
                    && (record[2] >> 4) + ((record[3] >> 5) & 3) < 16
                    && metadata
                        .programs
                        .iter()
                        .any(|program| program.program == record[1] & 0x7f)
            })
            .map(|(row, _)| row)
            .collect();
        if reference_rows.is_empty() {
            continue;
        }
        asset.content_type = "sfx";
        asset.roles.push(Role {
            role: "sfx",
            scope: "local_dispatch_reference",
            evidence: RoleEvidence::LocalDispatch {
                auxiliary_entry: auxiliary.clone(),
                reference_rows,
                target,
                dispatcher: DISPATCH,
                dispatcher_file_offset: (DISPATCH - load_address) as usize + 0x800,
                handler: HANDLERS[slot],
                handler_file_offset: (HANDLERS[slot] - load_address) as usize + 0x800,
                sample_references: Vec::new(),
            },
            playback: "unverified: active layout, loaded state, physical tone/sample selection, histories and independent PCM remain unresolved",
        });
    }
    resolve_samples(catalog, executable);
}

fn resolve_samples(catalog: &mut Catalog, executable: &Executable) {
    let halfword = |address: u32| {
        let at = (address - executable.header().load_address) as usize;
        u16::from_le_bytes(executable.text()[at..at + 2].try_into().unwrap()) as usize
    };
    let mut samples = BTreeMap::<String, Role>::new();
    for bank in &catalog.assets {
        let AssetData::Bank {
            metadata,
            content: Some(content),
            physical_samples,
            ..
        } = &bank.data
        else {
            continue;
        };
        let Some(role) = bank.roles.first() else {
            continue;
        };
        let RoleEvidence::LocalDispatch {
            auxiliary_entry,
            reference_rows,
            ..
        } = &role.evidence
        else {
            continue;
        };
        let records = catalog
            .sources
            .iter()
            .flat_map(|source| &source.entries)
            .find(|entry| &entry.id == auxiliary_entry)
            .and_then(|entry| entry.auxiliary_records.as_ref())
            .expect("bank role requires a bounded auxiliary table");
        for &row in reference_rows {
            let record = records[row];
            let program_id = record[1] & 0x7f;
            let program = metadata
                .programs
                .iter()
                .find(|program| program.program == program_id)
                .expect("bank role requires a populated program");
            let first = usize::from(record[2] >> 4);
            let extra = usize::from((record[3] >> 5) & 3);
            // Cue attributes use fixed original tables, independently of the
            // SDK's packed program block used for the sample halfword below.
            let program_offset = halfword(0x8018_21e0 + u32::from(program_id) * 2);
            if (first..=first + extra).any(|tone| {
                program_offset + halfword(0x8018_21e8 + tone as u32 * 2) + 0x820 + 7
                    > content.header_bytes as usize
            }) {
                continue;
            }
            // Fallthrough retains the layer count: higher layers repeat the
            // same endpoint tone. BATL_SE row 2 therefore selects 6, 6, 4.
            for (layer, tone) in std::iter::repeat_n(first + extra, extra)
                .chain(std::iter::once(first))
                .enumerate()
            {
                let sample = physical_samples[program.tone_block][tone];
                // SsUtKeyOnV reads a halfword; zero rejects key-on. This is
                // distinct from the music selector's byte-sized reference.
                if sample == 0 || sample > metadata.declared_samples {
                    continue;
                }
                let id = format!("{}/sample={sample}", bank.id);
                let evidence = samples.entry(id).or_insert_with(|| {
                    let mut evidence = role.clone();
                    evidence.scope = "local_dispatch_sample_reference";
                    if let RoleEvidence::LocalDispatch { reference_rows, .. } = &mut evidence.evidence {
                        reference_rows.clear();
                    }
                    evidence.playback = "unverified: source-qualified physical sample reference only; active layout, loaded state, arbitration, audibility, histories and independent PCM remain unresolved";
                    evidence
                });
                let RoleEvidence::LocalDispatch {
                    reference_rows,
                    sample_references,
                    ..
                } = &mut evidence.evidence
                else {
                    unreachable!("sample references originate in the local dispatcher");
                };
                if reference_rows.last() != Some(&row) {
                    reference_rows.push(row);
                }
                sample_references.push(SampleReference {
                    row,
                    layer,
                    program: program_id,
                    tone,
                    header_offset: 0x820 + program.tone_block * 512 + tone * 32 + 22,
                    sample_reader: 0x8016_e5e8,
                    sample_reader_file_offset: (0x8016_e5e8 - executable.header().load_address)
                        as usize
                        + 0x800,
                });
            }
        }
    }
    for asset in &mut catalog.assets {
        if !matches!(asset.data, AssetData::Sample { .. }) {
            continue;
        }
        asset.roles.clear();
        asset.content_type = "unresolved";
        if let Some(role) = samples.remove(&asset.id) {
            asset.content_type = "sfx";
            asset.roles.push(role);
        }
    }
}

pub(super) fn resolve_vocals(catalog: &mut Catalog) {
    const SOURCE_HASH: &str = "f4f9d8ce9a2849501e8cd44dfcfee85aad4bc92cd9ff5113acde43c9362c65cc";
    for asset in &mut catalog.assets {
        let AssetData::XaCue(cue) = &asset.data else {
            continue;
        };
        let original = catalog.sources.iter().any(|source| {
            source.source == asset.source && source.sha256.as_deref() == Some(SOURCE_HASH)
        });
        if !original
            || (
                cue.packed_id,
                cue.filter_file,
                cue.channel,
                cue.sector_start,
                cue.sector_stride,
                cue.sector_count,
                cue.last_sector,
                cue.table_address,
            ) != (0x2000, 1, 0, 0, 16, 71, 1120, 0x8018_3298)
        {
            continue;
        }
        asset.content_type = "vocals";
        asset.roles.push(Role {
            role: "vocals",
            scope: "source_qualified_content_observation",
            evidence: RoleEvidence::XaContentObservation {
                source_sha256: SOURCE_HASH,
                reference_source: cue.disc_path,
                cue: cue.clone(),
                coding: 0,
                observer: "user",
                observation: "Japanese vocals confirmed by listening to this exact cue on 2026-11-08",
                language: "ja",
                wav_sha256: "8c4421937c0d5b9f84bd94c4b6cb217fe4fd6df3c866d123ded7da6b1079a766",
                pcm_frames: 286272,
                sample_rate: 37800,
                channels: 1,
                initial_history: "zero_per_exported_asset",
                arithmetic: "combined_rounded",
            },
            playback: "unverified: content observation only; active caller, game history, scheduler endpoint and independent playback remain unresolved",
        });
    }
}
