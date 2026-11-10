//! Original-file inputs for development comparisons and pending production migrations.
use crate::{
    Result,
    catalog::{
        loader::{LAYOUT_INITIALIZE, LOADER_SLOTS, LOADER_TABLE},
        mapping::Mapping,
        model::{Layout, Slot, XaCue},
    },
    machine::{
        bus::{Bus, Ram, Width},
        cpu::Cpu,
        image::executable::Executable,
        image::profile::Profile,
    },
    xa::cue,
};

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

pub fn read_cues(executable: &Executable) -> Result<Vec<XaCue>> {
    Profile::identify(executable)?;
    let mut ram = Ram::from_executable(executable);
    let mut bases = [0; 3];
    for (stream, table, _, _, _, file_slot) in cue::streams() {
        if ram.read(cue::TABLE_POINTERS + u32::from(stream) * 4, Width::Word)? != table
            || ram.read(cue::FILE_SLOTS + u32::from(stream) * 4, Width::Word)?
                != u32::from(file_slot)
        {
            return Err("US XA stream tables disagree with supported profile".into());
        }
        bases[usize::from(stream)] = ram.read(
            crate::catalog::music::FILE_TABLE + u32::from(file_slot) * 4,
            Width::Word,
        )?;
    }
    cue::read_tables(&mut ram, [0x8018_39c4, 0x8018_32c4, 0x8018_3298], bases)
}

pub(crate) fn read_cue_lookup(ram: &mut Ram) -> Result<[u8; 256]> {
    let mut lookup = [0; 256];
    for (index, byte) in lookup.iter_mut().enumerate() {
        *byte = ram.read(0x8018_21e0 + index as u32, Width::Byte)? as u8;
    }
    Ok(lookup)
}

pub fn read(executable: &Executable) -> Result<Mapping> {
    let profile = Profile::identify(executable)?;
    let mut ram = Ram::from_executable(executable);
    for file in crate::catalog::music::disc_evidence()?.files {
        if ram.read(
            crate::catalog::music::FILE_TABLE + u32::from(file.slot) * 4,
            Width::Word,
        )? != file.lba
        {
            return Err(format!(
                "music file slot {} disagrees with verified ISO extent",
                file.slot
            )
            .into());
        }
    }
    let mut music = [0; 662];
    for (index, byte) in music.iter_mut().enumerate() {
        *byte = ram.read(crate::catalog::music::CUE_TABLE + index as u32, Width::Byte)? as u8;
    }
    let lookup = read_cue_lookup(&mut ram)?;
    let mut handlers = [0; 9];
    for (state, entry) in handlers.iter_mut().enumerate() {
        *entry = ram.read(cue::STATE_TABLE + state as u32 * 4, Width::Word)?;
    }
    if handlers != crate::catalog::mapping::XA_SOURCE_HANDLERS {
        return Err("original XA scheduler handlers disagree with source provenance".into());
    }
    Ok(Mapping {
        profile,
        source_load_address: executable.header().load_address,
        layouts: (0..3)
            .map(|selector| initialize_layout(executable, selector))
            .collect::<Result<_>>()?,
        auxiliary_end: 0x80148a04,
        lookup,
        music,
        xa_cues: read_cues(executable)?,
        xa_scheduler: crate::catalog::mapping::scheduler(),
    })
}
