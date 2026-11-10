//! Encoded XA cue extents from the pinned US selector tables.
//! The selector's stop threshold is exposed separately; scheduler timing is unverified.

use crate::catalog::model::{Binding, CueMapping, XaCue, XaMap};
use crate::{
    catalog::model::Asset, catalog::model::AssetData, catalog::model::Catalog, machine::bus::Bus,
    machine::bus::Width, machine::cd_position::CdPosition, Result,
};
use std::collections::{BTreeMap, BTreeSet};

pub const SELECTOR: u32 = 0x8016_3744;
pub const FILE_SLOTS: u32 = 0x8018_3230;
pub const TABLE_POINTERS: u32 = 0x8018_323c;
pub const START: u32 = 0x8016_36a0;
pub const CALLBACK: u32 = 0x8016_3858;
pub const TICK: u32 = 0x8016_38b0;
pub const STATE_TABLE: u32 = 0x8018_3274;

/// Read complete reviewed selector data from the built owned image. Media LBAs
/// come from the supplied disc directory, independently of original EXE data.
pub fn read_owned_cues(
    image: &crate::machine::driver::Image,
    bases: [u32; 3],
) -> Result<Vec<XaCue>> {
    let mut ram = image.ram()?;
    let mut tables = [0; 3];
    for (index, (name, words, hash)) in [
        (
            "scene",
            15,
            "c3c98939bc16564f49a85f703b236ed9fca15804b657d6c265addde7f9fe573a",
        ),
        (
            "magic",
            896,
            "aa8abf2058a0e1f741ce583ba2045a57300ba469c9a6dbfc1a4e18bbda6a9f9f",
        ),
        (
            "voice",
            10,
            "ed182923777f469bb91bc7138c741aeff3aab09b3260bbc3369c23a759a52f93",
        ),
    ]
    .into_iter()
    .enumerate()
    {
        let start = image.symbol(&format!("audio_xa_{name}_table"))?;
        let end = image.symbol(&format!("audio_xa_{name}_table_end"))?;
        if end.checked_sub(start) != Some(words * 2)
            || ram.read(
                image.symbol("audio_xa_table_pointers")? + index as u32 * 4,
                Width::Word,
            )? != start
            || ram.read(
                image.symbol("audio_xa_file_slots")? + index as u32 * 4,
                Width::Word,
            )? != u32::from(streams()[index].5)
        {
            return Err("owned XA: selector metadata differs from the supported US profile".into());
        }
        let mut bytes = Vec::with_capacity(words as usize * 2);
        for offset in 0..words {
            bytes.extend_from_slice(
                &(ram.read(start + offset * 2, Width::Half)? as u16).to_le_bytes(),
            );
        }
        if crate::digest::sha256_hex(&bytes) != hash {
            return Err("owned XA: selector halfwords differ from the supported US profile".into());
        }
        tables[index] = start;
    }
    read_tables(&mut ram, tables, bases)
}

pub(crate) fn streams() -> [(u8, u32, u32, u16, &'static str, u16); 3] {
    [
        (0, 0x8018_39c4, 15, 11, "BIN/SCE_XA/S_XA00.STR", 681),
        (1, 0x8018_32c4, 896, 880, "BIN/BMAG_XA/MAGIC00.STR", 434),
        (2, 0x8018_3298, 10, 5, "BIN/SCE_XA/VOICE.STR", 682),
    ]
}

pub(crate) fn read_tables(
    ram: &mut impl Bus,
    tables: [u32; 3],
    bases: [u32; 3],
) -> Result<Vec<XaCue>> {
    let mut cues = Vec::new();
    for (stream, original_table, words, expected_count, path, file_slot) in streams() {
        let table = tables[usize::from(stream)];
        let base = bases[usize::from(stream)];
        let stride = if stream == 0 { 8 } else { 16 };
        let mut pos = 0;
        let mut channel = 0;
        let mut cue_index = 0;
        while pos + 1 < words {
            let address = table + pos * 2;
            let word = ram.read(address, Width::Half)?;
            let start = (word & 0x7fff) as usize;
            let end = (ram.read(address + 2, Width::Half)? & 0x7fff) as usize;
            if end <= start || usize::from(channel) >= stride {
                return Err("invalid US XA cue extent/channel".into());
            }
            let sector_start = start * stride + usize::from(channel);
            let last_sector = (end - 1) * stride + usize::from(channel);
            cues.push(XaCue {
                packed_id: u16::from(stream) * 0x1000 + cue_index,
                stream,
                cue_index,
                table_address: original_table + pos * 2,
                disc_path: path,
                disc_file_slot: file_slot,
                filter_file: 1,
                channel,
                sector_start,
                sector_stride: stride,
                sector_count: end - start,
                last_sector,
                runtime_start_lba: base + sector_start as u32,
                runtime_start_bcd: CdPosition::from_lba((base + sector_start as u32) as i32)?.bcd(),
                runtime_stop_threshold_lba: base + (end * stride + usize::from(channel)) as u32
                    - 150,
                runtime_stop_threshold_bcd: CdPosition::from_lba(
                    (base + (end * stride + usize::from(channel)) as u32) as i32 - 150,
                )?
                .bcd(),
                scheduler_endpoint: "not_verified",
            });
            cue_index += 1;
            if word & 0x8000 != 0 {
                channel += 1;
                pos += 2;
            } else {
                pos += 1;
            }
        }
        if cue_index != expected_count {
            return Err("US XA cue count differs from verified table walk".into());
        }
    }
    Ok(cues)
}

pub fn resolve(catalog: &mut Catalog, mapping: &crate::catalog::mapping::Mapping) -> Result<XaMap> {
    let cues = mapping.xa_cues.clone();
    catalog
        .assets
        .retain(|asset| !matches!(asset.data, AssetData::XaCue(_)));
    let mut sectors = BTreeMap::<(&str, u8, u8), BTreeSet<usize>>::new();
    for asset in &catalog.assets {
        if let AssetData::XaStream(stream) = &asset.data {
            let set = sectors
                .entry((&asset.source, stream.file, stream.channel))
                .or_default();
            for range in &stream.sector_ranges {
                set.extend(range.start..range.end_exclusive);
            }
        }
    }
    let mut mappings = Vec::new();
    let mut assets = Vec::new();
    for cue in cues {
        let mut bindings = Vec::new();
        for source in &catalog.sources {
            let Some(reference) = source
                .sha256
                .as_deref()
                .and_then(crate::xa::reference::identify)
            else {
                continue;
            };
            if reference.reference_source != cue.disc_path {
                continue;
            }
            let available = sectors.get(&(source.source.as_str(), cue.filter_file, cue.channel));
            let missing = (0..cue.sector_count)
                .filter(|index| {
                    !available.is_some_and(|set| {
                        set.contains(&(cue.sector_start + index * cue.sector_stride))
                    })
                })
                .count();
            let asset = if missing == 0 {
                let id = format!(
                    "{}#stream={}/channel={}/cue={}/sectors={}..{}",
                    crate::catalog::escape_source(&source.source),
                    cue.filter_file,
                    cue.channel,
                    cue.packed_id,
                    cue.sector_start,
                    cue.last_sector + 1
                );
                assets.push(Asset {
                    id: id.clone(),
                    source: source.source.clone(),
                    entry: None,
                    game_bank_id: None,
                    game_song_ids: Vec::new(),
                    content_type: "unresolved",
                    roles: Vec::new(),
                    data: AssetData::XaCue(cue.clone()),
                });
                Some(id)
            } else {
                None
            };
            bindings.push(Binding {
                source: source.source.clone(),
                source_completeness: reference.status,
                missing_selected_sectors: missing,
                asset,
            });
        }
        mappings.push(CueMapping { cue, bindings });
    }
    catalog.assets.extend(assets);
    Ok(XaMap { selector: SELECTOR, file_slots: FILE_SLOTS, table_pointers: TABLE_POINTERS,
        extent_evidence: "supported positive-range table prefixes; all selected sectors checked against supplied recognized media; scheduler endpoints and other XA channels remain unresolved",
        cues: mappings, scheduler: mapping.xa_scheduler.clone() })
}
