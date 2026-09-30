//! Executable-backed loader associations, not playback fidelity evidence.

use crate::catalog::model::{Association, Layout, Reference, Report, Slot};
use crate::{
    catalog::model::AssetData, catalog::model::Catalog, machine::bus::Bus, machine::bus::Ram,
    machine::bus::Width, machine::cpu::Cpu, machine::executable::Executable,
    machine::profile::Profile, Result,
};
use std::collections::BTreeMap;

pub const LAYOUT_INITIALIZE: u32 = 0x8016_1808;
pub const LOADER_TABLE: u32 = 0x8014_677c;
pub const LOADER_SLOTS: usize = 7;

/// Execute the original leaf routine without substituting game/SDK calls.
/// These rows describe possible layouts, not the active gameplay mode.
pub fn initialize_layout(executable: &Executable, selector: u8) -> Result<Layout> {
    Profile::identify(executable)?;
    if selector > 2 {
        return Err("US audio layout selector must be 0, 1, or 2".into());
    }
    let mut ram = Ram::from_executable(executable);
    let mut cpu = Cpu::new(LAYOUT_INITIALIZE);
    cpu.set_register(4, u32::from(selector));
    cpu.set_register(31, 0x8000_1000);
    let instructions = cpu.run_until(&mut ram, 0x8000_1000, 1000)?;
    let mut slots = Vec::new();
    for index in 0..LOADER_SLOTS {
        let address = LOADER_TABLE + index as u32 * 20;
        let table_index = u32::from(selector) * 28 + index as u32 * 4;
        slots.push(Slot {
            index,
            spu_base: ram.read(address, Width::Word)?,
            header_address: ram.read(address + 4, Width::Word)?,
            auxiliary_address: ram.read(address + 8, Width::Word)?,
            sequence_address: ram.read(address + 12, Width::Word)?,
            vab_id: ram.read(address + 16, Width::Half)? as u16,
            flags: ram.read(address + 18, Width::Half)? as u16,
            header_capacity: ram.read(0x8018_239c + table_index, Width::Word)?,
            sequence_capacity: ram.read(0x8018_23f0 + table_index, Width::Word)?,
        });
    }
    Ok(Layout {
        selector,
        instructions,
        slots,
    })
}

pub fn resolve(catalog: &mut Catalog, executable: &Executable) -> Result<Report> {
    let profile = Profile::identify(executable)?;
    let layouts = (0..3)
        .map(|selector| initialize_layout(executable, selector))
        .collect::<Result<Vec<_>>>()?;
    for layout in &layouts {
        if layout
            .slots
            .iter()
            .any(|slot| usize::from(slot.vab_id) != slot.index)
        {
            return Err(
                "US loader table redirects a slot unexpectedly; association model rejected".into(),
            );
        }
    }
    let mut associations = Vec::<Association>::new();
    let mut entry_banks = BTreeMap::new();
    let mut unresolved = vec![
        "layout tables are initialized independently; active gameplay layout and load history are not established".into(),
        "music cues are limited to the verified media-backed prefix; caller bounds, XA scheduler endpoints, and SFX/vocal classification remain unresolved".into(),
        "the loader routines are mapped; CD transfer, bank opening, scheduler, and playback are not executed by this report".into(),
    ];
    for source in &catalog.sources {
        let mut selected = None;
        for entry in &source.entries {
            match entry.file_type {
                6 => {
                    if entry.load_argument as usize >= LOADER_SLOTS {
                        unresolved.push(format!(
                            "{}: loader argument {} exceeds seven supported slots",
                            entry.id, entry.load_argument
                        ));
                        selected = None;
                        continue;
                    }
                    let index = associations.len();
                    associations.push(Association {
                        bank: entry.id.clone(),
                        load_slot: entry.load_argument,
                        game_bank_id: layouts[0].slots[entry.load_argument as usize].vab_id,
                        body_entries: Vec::new(),
                        auxiliary_entries: Vec::new(),
                        songs: Vec::new(),
                    });
                    selected = Some(index);
                    entry_banks.insert(entry.id.clone(), index);
                }
                7..=10 => {
                    if let Some(index) = selected {
                        let association = &mut associations[index];
                        match entry.file_type {
                            7 => association.body_entries.push(entry.id.clone()),
                            8 => association.auxiliary_entries.push(entry.id.clone()),
                            _ => association.songs.push(entry.id.clone()),
                        }
                        entry_banks.insert(entry.id.clone(), index);
                    } else {
                        unresolved.push(format!("{}: audio payload precedes a verified slot-selecting VH; prior game load context required", entry.id));
                    }
                }
                _ => {}
            }
        }
    }
    for asset in &mut catalog.assets {
        let parent = match &asset.data {
            AssetData::Sample { bank, .. } => bank,
            AssetData::Sequence { song, .. } => song,
            _ => &asset.id,
        };
        if let Some(&index) = entry_banks.get(parent) {
            let association = &associations[index];
            asset.game_bank_id = Some(association.game_bank_id);
            match &mut asset.data {
                AssetData::Bank {
                    body_candidates, ..
                } => body_candidates.clone_from(&association.body_entries),
                AssetData::Song {
                    bank_candidates, ..
                } => *bank_candidates = vec![association.bank.clone()],
                _ => {}
            }
        }
    }
    let shared_banks = crate::catalog::content::resolve(catalog, &associations, &mut unresolved);
    let mut references = Vec::new();
    for (address, symbol, role) in [
        (LAYOUT_INITIALIZE, None, "initializes seven audio slots from one of three layout rows"),
        (0x8016_2b08, Some("stageEmiTransferSlot"), "copies EMI TOC load argument into active loader state"),
        (0x8016_2790, Some("selectNextEmiEntry"), "type 6 selects the runtime resource slot and VH destination"),
        (0x8016_2898, Some("startEmiEntryTransfer"), "type 7 opens the selected slot's VH with its VAB ID and SPU base"),
        (0x8016_29f0, Some("selectPrimaryEmiDestination"), "type 8 uses the selected slot's auxiliary destination"),
        (0x8016_2a6c, Some("selectAlternateEmiDestination"), "types 9/10 use the selected slot's sequence destination and set its sequence flag"),
        (0x8016_35d0, None, "callsite passes sequence pointer, VAB ID, and four sequences to the linked open routine"),
        (0x8018_3248, None, "entry-type dispatch table"),
        (0x8016_1bbc, None, "cue file selection reads the first halfword and conditionally starts its disc file"),
        (0x8016_1c20, None, "cue dispatch reads separate game bank and sequence-index bytes"),
        (0x8016_2160, Some("emiLoaderSlotLba"), "translates a disc file slot to its original LBA"),
        (crate::xa::cue::START, None, "initializes XA cue state, BCD seek location and completion callback"),
        (crate::xa::cue::CALLBACK, None, "records CD completion/error and copies the eight-byte response"),
        (crate::xa::cue::TICK, None, "dispatches XA scheduler states with cancellation and watchdog handling"),
        (crate::xa::cue::STATE_TABLE, None, "nine XA scheduler handler addresses"),
        (crate::xa::cue::SELECTOR, None, "selects XA stream, file/channel filter, start and raw stop threshold"),
        (crate::xa::cue::FILE_SLOTS, None, "XA stream disc file-slot table"),
        (crate::xa::cue::TABLE_POINTERS, None, "XA stream cue-table pointers; supported positive-range prefixes only"),
        (crate::catalog::music::CUE_TABLE, None, "supported music cue record prefix"),
        (crate::catalog::music::FILE_TABLE, None, "original file-slot LBA table checked against ISO directory evidence"),
    ] {
        references.push(Reference { target: profile.target, address,
            executable_file_offset: (address - executable.header().load_address) as usize + 0x800,
            symbol, role });
    }
    let music = crate::catalog::music::resolve(catalog, executable)?;
    let xa = crate::xa::cue::resolve(catalog, executable)?;
    Ok(Report {
        schema: "bof3.audio-mapping/v1",
        profile,
        layouts,
        associations,
        shared_banks,
        references,
        unresolved,
        music,
        xa,
    })
}
