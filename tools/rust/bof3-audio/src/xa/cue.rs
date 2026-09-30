//! Encoded XA cue extents from the pinned US selector tables.
//! The selector's stop threshold is exposed separately; scheduler timing is unverified.

use crate::catalog::model::{Binding, CueMapping, SchedulerEvidence, XaCue, XaMap};
use crate::{
    catalog::model::Asset, catalog::model::AssetData, catalog::model::Catalog, machine::bus::Bus,
    machine::bus::Ram, machine::bus::Width, machine::cd_position::CdPosition,
    machine::executable::Executable, machine::profile::Profile, Result,
};
use std::collections::{BTreeMap, BTreeSet};

pub const SELECTOR: u32 = 0x8016_3744;
pub const FILE_SLOTS: u32 = 0x8018_3230;
pub const TABLE_POINTERS: u32 = 0x8018_323c;
pub const START: u32 = 0x8016_36a0;
pub const CALLBACK: u32 = 0x8016_3858;
pub const TICK: u32 = 0x8016_38b0;
pub const STATE_TABLE: u32 = 0x8018_3274;

fn scheduler(executable: &Executable) -> Result<SchedulerEvidence> {
    let mut ram = Ram::from_executable(executable);
    let mut entries = [0; 9];
    for (state, entry) in entries.iter_mut().enumerate() {
        *entry = ram.read(STATE_TABLE + state as u32 * 4, Width::Word)?;
    }
    Ok(SchedulerEvidence {
        start_entry: START,
        completion_callback: CALLBACK,
        tick_entry: TICK,
        state_table: STATE_TABLE,
        state_entries: entries,
        position_poll_period_ticks: 2,
        watchdog_threshold_ticks: 181,
        stop_condition: "state 5 enters state 6 when a completed GetlocL response converts to LBA >= runtime_stop_threshold_lba; state 6 reduces the CD mix to zero and requests Pause",
        timing_evidence: "original initialization, callback, position comparison, cancellation and watchdog paths tested with injected responses; drive/IRQ latency, tick frequency, buffered PCM and audible endpoints remain unverified",
    })
}

pub fn read_cues(executable: &Executable) -> Result<Vec<XaCue>> {
    Profile::identify(executable)?;
    let mut ram = Ram::from_executable(executable);
    let mut cues = Vec::new();
    for (stream, table, words, expected_count, path, file_slot) in [
        (0u8, 0x8018_39c4, 15, 11, "BIN/SCE_XA/S_XA00.STR", 681u16),
        (1, 0x8018_32c4, 896, 880, "BIN/BMAG_XA/MAGIC00.STR", 434),
        (2, 0x8018_3298, 10, 5, "BIN/SCE_XA/VOICE.STR", 682),
    ] {
        if ram.read(TABLE_POINTERS + u32::from(stream) * 4, Width::Word)? != table
            || ram.read(FILE_SLOTS + u32::from(stream) * 4, Width::Word)? != u32::from(file_slot)
        {
            return Err("US XA stream tables disagree with supported profile".into());
        }
        let base = ram.read(
            crate::catalog::music::FILE_TABLE + u32::from(file_slot) * 4,
            Width::Word,
        )?;
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
                table_address: address,
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

pub fn resolve(catalog: &mut Catalog, executable: &Executable) -> Result<XaMap> {
    let cues = read_cues(executable)?;
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
        cues: mappings, scheduler: scheduler(executable)? })
}
