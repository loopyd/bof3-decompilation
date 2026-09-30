//! Bounded US effects cue staging and original game dispatch.
//! Local-bank records use the selected original handler; redirection is explicit.
use super::{
    bank::Prepared,
    bus::{Bus, Width},
    executable::ram_offset,
    execution::{Call, Execution},
};
use crate::{catalog::loader, digest::sha256_hex, Result};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;

pub const DISPATCH: u32 = 0x8015_df18;
pub const REFRESH: u32 = 0x8015_d044;
// Original initialization writes the sound-control halfwords at 0x80148A04/06.
// They bound the final auxiliary buffer and must not be overwritten.
const AUXILIARY_END: u32 = 0x8014_8a04;

#[derive(Debug, Serialize)]
pub struct State {
    pub dispatch_gate: i8,
    pub previous_voice: i16,
    pub previous_arbitration: i16,
    pub current_voice: i16,
    pub current_arbitration: i16,
    pub voice_status: [u32; 24],
}

/// Read the game's snapshot, which is distinct from current SPU hardware state.
pub fn state(prepared: &Prepared) -> Result<State> {
    let ram = prepared.execution.bus.ram().bytes();
    let half = |address| -> Result<i16> {
        let at = ram_offset(address, 2)?;
        Ok(i16::from_le_bytes(ram[at..at + 2].try_into()?))
    };
    let at = ram_offset(0x8018_e140, 24 * 4)?;
    Ok(State {
        dispatch_gate: ram[ram_offset(0x8018_232a, 1)?] as i8,
        previous_voice: half(0x8018_b3f0)?,
        previous_arbitration: half(0x8018_b3ec)?,
        current_voice: half(0x8018_b318)?,
        current_arbitration: half(0x8018_b3e8)?,
        voice_status: std::array::from_fn(|i| {
            u32::from_le_bytes(ram[at + i * 4..at + i * 4 + 4].try_into().unwrap())
        }),
    })
}

/// Execute the game's separate 24-voice status poll. Do not implicitly poll
/// during dispatch: callers can submit multiple cues against one snapshot.
pub fn refresh(
    prepared: &mut Prepared,
    observer: &mut impl FnMut(&Execution) -> Result<()>,
) -> Result<Call> {
    prepared
        .execution
        .call_observed(REFRESH, [0; 4], 100_000, observer)
}

#[derive(Debug, Serialize)]
pub struct Table {
    pub archive_sha256: String,
    pub header_entry: usize,
    pub auxiliary_entry: usize,
    pub game_bank_id: u16,
    pub address: u32,
    pub capacity: usize,
    pub records: Vec<[u8; 4]>,
}

/// Host staging of the type-8 payload, as with VAB/SEP preparation. This does
/// not execute the CD loader or establish its scheduling and transport behavior.
pub fn stage(prepared: &mut Prepared, archive: &ArchiveImage, index: usize) -> Result<Table> {
    if sha256_hex(archive.bytes()) != prepared.identity.archive_sha256 {
        return Err("PSX cues: auxiliary archive differs from prepared bank".into());
    }
    if usize::from(prepared.identity.game_bank_id) >= loader::LOADER_SLOTS {
        return Err("PSX cues: bank does not select one of the seven game handlers".into());
    }
    if archive
        .entries()
        .get(index)
        .is_none_or(|e| e.file_type != 8)
    {
        return Err("PSX cues: auxiliary entry must have type 8".into());
    }
    let header = prepared.identity.header_entry;
    if index <= header
        || archive.entries()[header + 1..index]
            .iter()
            .any(|e| e.file_type == 6)
    {
        return Err("PSX cues: auxiliary entry is not owned by the prepared header".into());
    }
    let bytes = archive.entry(index)?;
    if bytes.is_empty() || bytes.len() % 4 != 0 || bytes.len() > 256 * 4 {
        return Err("PSX cues: expected 1..256 complete four-byte records".into());
    }
    let address = prepared.identity.slot.auxiliary_address;
    if address >= AUXILIARY_END {
        return Err("PSX cues: auxiliary destination overlaps sound-control state".into());
    }
    let mut next = AUXILIARY_END;
    for slot in 0..loader::LOADER_SLOTS {
        let target = prepared
            .execution
            .bus
            .read(loader::LOADER_TABLE + slot as u32 * 20 + 8, Width::Word)?;
        if target > address {
            next = next.min(target);
        }
    }
    let capacity = (next - address) as usize;
    if bytes.len() > capacity {
        return Err(format!(
            "PSX cues: {} auxiliary bytes exceed capacity {capacity}",
            bytes.len()
        )
        .into());
    }
    ram_offset(address, bytes.len())?;
    let table = Table {
        archive_sha256: prepared.identity.archive_sha256.clone(),
        header_entry: header,
        auxiliary_entry: index,
        game_bank_id: prepared.identity.game_bank_id,
        address,
        capacity,
        records: bytes.as_chunks::<4>().0.to_vec(),
    };
    for (i, &byte) in bytes.iter().enumerate() {
        prepared
            .execution
            .bus
            .write(address + i as u32, Width::Byte, byte.into())?;
    }
    Ok(table)
}

/// Run the original dispatcher with a source-qualified auxiliary row. Validation
/// covers the supported local-bank record shape; allocation/volume decisions
/// remain guest instructions, not host reconstruction.
/// Runtime tone guards may fail after guest state changes; stop or reinitialize
/// the prepared machine after an error rather than continuing that history.
pub fn dispatch(
    prepared: &mut Prepared,
    table: &Table,
    row: usize,
    observer: &mut impl FnMut(&Execution) -> Result<()>,
) -> Result<Call> {
    if table.archive_sha256 != prepared.identity.archive_sha256
        || table.header_entry != prepared.identity.header_entry
        || table.game_bank_id != prepared.identity.game_bank_id
        || usize::from(table.game_bank_id) >= loader::LOADER_SLOTS
        || table.address != prepared.identity.slot.auxiliary_address
    {
        return Err("PSX cues: table does not identify the prepared bank".into());
    }
    let record = table
        .records
        .get(row)
        .ok_or("PSX cues: row outside auxiliary table")?;
    if record[0] != 0 || record[3] & 0x80 != 0 {
        return Err(
            "PSX cues: cross-bank or unverified record flags require runtime context".into(),
        );
    }
    let program_id = record[1] & 0x7f;
    let tone = usize::from(record[2] >> 4);
    let count = usize::from((record[3] >> 5) & 3) + 1;
    if record[3] & 31 >= 24 {
        return Err("PSX cues: first voice exceeds the hardware voice range".into());
    }
    prepared
        .bank
        .programs
        .iter()
        .find(|p| p.program == program_id)
        .ok_or("PSX cues: referenced program is empty")?;
    if tone + count > 16 {
        return Err("PSX cues: layered tones exceed the physical program block".into());
    }
    let e = &mut prepared.execution;
    let offset = e
        .bus
        .read(0x8018_21e0 + u32::from(program_id) * 2, Width::Half)?;
    // Cue attributes and SDK playback selection are separate original paths.
    // In particular the fixed cue offset table need not select the program's
    // packed VAB block. Preserve the guest lookup, but bound all attribute reads.
    for index in tone..tone + count {
        let tone_offset = e.bus.read(0x8018_21e8 + index as u32 * 2, Width::Half)?;
        let end = 0x820 + offset as usize + tone_offset as usize + 7;
        if end > prepared.identity.header_bytes {
            return Err("PSX cues: original attribute lookup exceeds the loaded VH payload".into());
        }
    }
    let at = ram_offset(table.address, table.records.len() * 4)?;
    if e.bus.ram().bytes()[at..at + table.records.len() * 4]
        .as_chunks::<4>()
        .0
        != table.records
    {
        return Err("PSX cues: staged auxiliary data changed".into());
    }
    if row > 255 {
        return Err("PSX cues: row cannot be encoded in the cue selector".into());
    }
    let tones_start = prepared.identity.slot.header_address + 0x820;
    let tones_end = tones_start + u32::from(prepared.bank.declared_programs) * 512;
    let samples = prepared.bank.declared_samples;
    e.call_observed(
        DISPATCH,
        [(u32::from(table.game_bank_id) << 8) | row as u32, 0, 0, 0],
        100_000,
        &mut |execution| {
            // SsUtKeyOnV reads a physical tone slot, even beyond the declared
            // tone count. Observe its computed address before the first load;
            // game handlers do not always select consecutive tone indices.
            if execution.cpu.pc() == 0x8016_e5dc {
                let address = execution.cpu.register(2);
                if address < tones_start
                    || address.checked_add(32).is_none_or(|end| end > tones_end)
                    || !(address - tones_start).is_multiple_of(32)
                {
                    return Err(
                        "PSX cues: original SDK tone read exceeds the loaded tone blocks".into(),
                    );
                }
                let at = ram_offset(address + 22, 2)?;
                let sample =
                    u16::from_le_bytes(execution.bus.ram().bytes()[at..at + 2].try_into()?);
                // Sample zero follows the SDK's explicit -1 return without
                // key-on. Other noncanonical references still need context.
                if sample > samples {
                    return Err(format!(
                        "PSX cues: original SDK sample reference {sample} at 0x{address:08X} exceeds the loaded bank's {samples} samples; requires runtime context"
                    ).into());
                }
            }
            observer(execution)
        },
    )
}
