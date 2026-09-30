//! Explicit-tone playback through the verified US `SsUtKeyOnV` routine.
//! This is an SDK entry, not a recovered game cue-to-tone mapping.

use super::{
    bank::Prepared,
    bus::{Bus, Width},
    executable::ram_offset,
    execution::{Call, Execution},
};
use crate::Result;
use serde::Serialize;

pub const KEY_ON: u32 = 0x8016_e400;
pub const KEY_OFF: u32 = 0x8016_e794;

#[derive(Clone, Copy, Debug, Serialize)]
pub struct Note {
    pub voice: u8,
    pub program: u8,
    pub tone: u8,
    pub key: u8,
    pub fine: u8,
    pub left: u8,
    pub right: u8,
}

/// All selection and pitch/volume context is explicit. The original routine
/// rejects voices outside 0..24; it does not automatically allocate a voice.
/// Observe the guest call after validation and ABI argument staging.
pub fn key_on(
    prepared: &mut Prepared,
    note: Note,
    observer: &mut impl FnMut(&Execution) -> Result<()>,
) -> Result<Call> {
    if note.voice >= 24 {
        return Err("PSX effects: explicit voice must be 0..23".into());
    }
    if [note.key, note.fine, note.left, note.right]
        .into_iter()
        .any(|v| v > 127)
    {
        return Err("PSX effects: supported key, fine and channel volumes are 0..127".into());
    }
    let tone = prepared
        .bank
        .programs
        .iter()
        .find(|p| p.program == note.program)
        .and_then(|p| p.tones.get(usize::from(note.tone)))
        .ok_or_else(|| {
            format!(
                "PSX effects: program {} tone {} is not populated in this bank",
                note.program, note.tone
            )
        })?;
    if tone.sample_reference < 1 || tone.sample_reference > prepared.bank.declared_samples as i16 {
        return Err("PSX effects: noncanonical sample reference needs runtime context".into());
    }
    let e = &mut prepared.execution;
    let caller_stack = e.cpu.register(29);
    if caller_stack & 7 != 0 {
        return Err("PSX effects: unaligned guest argument stack".into());
    }
    // A host-directed call has no compiled caller to reserve the o32 outgoing
    // argument area. Preserve its stack register around our eight-argument call.
    let stack = caller_stack
        .checked_sub(32)
        .ok_or("PSX effects: guest argument stack underflow")?;
    ram_offset(stack, 32)?;
    for (index, value) in [note.key, note.fine, note.left, note.right]
        .into_iter()
        .enumerate()
    {
        e.bus
            .write(stack + 16 + index as u32 * 4, Width::Word, value.into())?;
    }
    e.cpu.set_register(29, stack);
    let call = e.call_observed(
        KEY_ON,
        [
            note.voice.into(),
            prepared.identity.game_bank_id.into(),
            note.program.into(),
            note.tone.into(),
        ],
        100_000,
        observer,
    );
    e.cpu.set_register(29, caller_stack);
    let call = call?;
    if call.result != u32::from(note.voice) {
        return Err(format!(
            "PSX effects: original key-on returned {:#x}, requested voice {}",
            call.result, note.voice
        )
        .into());
    }
    Ok(call)
}

/// Release an explicit voice through the original adjacent SDK wrapper.
/// Unlike the managed-voice all-off routine, this covers all 24 voice indices.
pub fn key_off(
    prepared: &mut Prepared,
    voice: u8,
    observer: &mut impl FnMut(&Execution) -> Result<()>,
) -> Result<Call> {
    if voice >= 24 {
        return Err("PSX effects: explicit voice must be 0..23".into());
    }
    let call =
        prepared
            .execution
            .call_observed(KEY_OFF, [voice.into(), 0, 0, 0], 100_000, observer)?;
    if call.result != 0 {
        return Err(format!("PSX effects: original key-off returned {:#x}", call.result).into());
    }
    Ok(call)
}
