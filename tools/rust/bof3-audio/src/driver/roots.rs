//! Supported-US root candidates and bounded dispatch-table snapshot evidence.
use super::closure::{location, Location, Root};
use crate::{
    machine::cues, machine::executable::Executable, machine::profile::Profile, xa::cue, Result,
};
use serde::Serialize;
#[derive(Debug, Serialize)]
pub struct TableEntry {
    pub selector: u8,
    pub pointer: Location,
    pub target: Location,
}
#[derive(Debug, Serialize)]
pub struct Inventory {
    pub roots: Vec<Root>,
    pub cue_table: Vec<TableEntry>,
    pub limitations: Vec<&'static str>,
}
pub fn inventory(executable: &Executable) -> Result<Inventory> {
    let profile = Profile::identify(executable)?;
    let mut roots = Vec::new();
    for (address,reason) in [
        (profile.sound_initialize,"verified sound initialization"),
        (profile.cue_dispatch,"verified music cue selection wrapper"),
        (profile.sequence_table_setup,"verified sequence table initialization"),
        (profile.set_tick_mode,"verified tick mode initialization"),
        (profile.pitch_entry,"verified note-on pitch"),
        (profile.spu_pitch_entry,"verified SPU pitch"),
        (cues::DISPATCH,"dispatchSoundCue entry; original mask/table/JALR instructions corroborate partial recovered C"),
        (cues::REFRESH,"original 24-voice status snapshot used by cue arbitration"),
        (0x80175534,"original US CD reset dispatcher; modes 1/2 observed with booted ROM and explicit device response/timing inputs"),
        (cue::SELECTOR,"verified XA cue selector"),
        (cue::START,"verified XA cue start"),
        (cue::CALLBACK,"verified XA callback"),
        (cue::TICK,"verified XA tick"),
    ] { roots.push(Root {address,reason:reason.into()}); }
    // 0x8015DF1C/20 mask and shift to a four-bit selector; 0x8015DF44
    // loads base + selector*4, then JALR at 0x8015DF50. This bounds the
    // possible reads, not the valid gameplay selector domain or table extent.
    let mut cue_table = Vec::new();
    for selector in 0..16 {
        let pointer = location(executable, 0x8018232c + u32::from(selector) * 4);
        let offset = pointer
            .file_offset
            .ok_or("driver roots: cue pointer outside executable")?;
        let address = u32::from_le_bytes(executable.bytes()[offset..offset + 4].try_into()?);
        let target = location(executable, address);
        if address & 3 == 0 && target.file_offset.is_some() {
            roots.push(Root {address,reason:format!("original cue table snapshot selector {selector}; legal selector domain and runtime table mutation remain unresolved")});
        }
        cue_table.push(TableEntry {
            selector,
            pointer,
            target,
        });
    }
    Ok(Inventory {roots,cue_table,limitations:vec!["The cue selector mask permits 16 reads, but the original bytes contain only seven executable pointer candidates; adjacent data is not accepted as a callable handler.","Candidate roots and table snapshots do not establish complete legal cue domains, initialization writes, SFX/voice coverage or pruning safety."]})
}
