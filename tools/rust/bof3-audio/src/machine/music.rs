//! Owned single-bank SEP loading and resolved playback bindings.
//! Host staging replaces disc transport; runtime identities remain separate.
use super::{
    bank::{self, Stage},
    bus::{Bus, Width},
    driver::{self, Audio},
    execution::{Call, Execution},
};
use crate::{sequence::SequenceSet, Result};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct Identity {
    pub archive_sha256: String,
    pub header_entry: usize,
    pub body_entry: usize,
    pub sequence_entry: usize,
    pub header_bank_id: u32,
    pub game_bank_id: u16,
    pub sequence_index: usize,
    pub encoded_sequence_id: u16,
    pub layout: u8,
    pub requested_layout: Option<u8>,
    pub slot: crate::catalog::model::Slot,
    pub body_bytes: usize,
    pub dma_bytes: usize,
    pub next_slot_boundary: u32,
}

pub struct Prepared<P = driver::Evidence> {
    pub execution: Execution,
    pub startup: P,
    pub(crate) audio: Audio,
    pub identity: Identity,
    pub calls: Vec<Call>,
    pub handle: u32,
    pub record: u32,
}

pub fn prepare(
    image: &driver::Image,
    archive: &ArchiveImage,
    requested_layout: Option<u8>,
    sequence: usize,
) -> Result<Prepared> {
    prepare_observed(image, archive, requested_layout, sequence, &mut |_, _| {
        Ok(())
    })
}

/// Observe owned preparation instructions. Archive staging remains host work;
/// the observer cannot mutate guest state.
pub fn prepare_observed(
    image: &driver::Image,
    archive: &ArchiveImage,
    requested_layout: Option<u8>,
    sequence: usize,
    observer: &mut impl FnMut(Stage, &Execution) -> Result<()>,
) -> Result<Prepared> {
    prepare_with(
        archive,
        requested_layout,
        sequence,
        observer,
        |options, observer| bank::prepare(image, archive, options, observer),
    )
}

pub(crate) fn prepare_with<P, O: FnMut(Stage, &Execution) -> Result<()>>(
    archive: &ArchiveImage,
    requested_layout: Option<u8>,
    sequence: usize,
    observer: &mut O,
    prepare: impl FnOnce(&bank::Options, &mut O) -> Result<bank::Prepared<P>>,
) -> Result<Prepared<P>> {
    let unique = |kinds: &[u16]| -> Result<usize> {
        let entries: Vec<_> = archive
            .entries()
            .iter()
            .enumerate()
            .filter(|(_, e)| kinds.contains(&e.file_type))
            .map(|(i, _)| i)
            .collect();
        if entries.len() != 1 {
            return Err(format!("PSX music requires one entry of types {kinds:?}; found entries {entries:?}; multi-bank selection is not implemented").into());
        }
        Ok(entries[0])
    };
    let (vh, vb, sep) = (unique(&[6])?, unique(&[7])?, unique(&[9, 10])?);
    let sep_bytes = archive.entry(sep)?;
    let sequences = SequenceSet::parse(sep_bytes)?;
    let selected = sequences
        .sequences
        .get(sequence)
        .ok_or_else(|| format!("PSX music: SEP entry {sep} has no sequence index {sequence}"))?;
    if sequence >= 4 || sequences.sequences.len() > 4 {
        return Err("PSX music: original US layout permits four sequences per handle".into());
    }
    let loaded = prepare(
        &bank::Options {
            header_entry: vh,
            body_entry: vb,
            sequence_entry: Some(sep),
            layout: requested_layout,
        },
        observer,
    )?;
    let audio = loaded.audio;
    let bank = loaded.identity;
    let slot = bank.slot.clone();
    let id = u32::from(bank.game_bank_id);
    let mut execution = loaded.execution;
    let mut calls = loaded.calls;
    let identity = Identity {
        archive_sha256: bank.archive_sha256,
        header_entry: vh,
        body_entry: vb,
        sequence_entry: sep,
        header_bank_id: bank.header_bank_id,
        game_bank_id: bank.game_bank_id,
        sequence_index: sequence,
        encoded_sequence_id: selected.sequence_id,
        layout: bank.layout,
        requested_layout,
        slot: bank.slot,
        body_bytes: bank.body_bytes,
        dma_bytes: bank.dma_bytes,
        next_slot_boundary: bank.next_slot_boundary,
    };
    let mut call = |execution: &mut Execution, stage, entry, arguments, limit| {
        execution.call_observed(entry, arguments, limit, &mut |e| observer(stage, e))
    };
    let opened = call(
        &mut execution,
        Stage::Sequence,
        audio.sequence_open,
        [
            slot.sequence_address,
            id,
            sequences.sequences.len() as u32,
            0,
        ],
        100_000,
    )?;
    let handle = opened.result;
    if handle >= 2 {
        return Err(format!("PSX music: invalid SEP handle {handle:#x}").into());
    }
    calls.push(opened);
    // Use the game wrapper for callback registration and initial sound routing.
    calls.push(call(
        &mut execution,
        Stage::Callbacks,
        audio.callbacks,
        [0; 4],
        100_000,
    )?);
    let base = execution
        .bus
        .read(audio.sequence_handles + handle * 4, Width::Word)?;
    Ok(Prepared {
        execution,
        startup: loaded.startup,
        audio,
        identity,
        calls,
        handle,
        record: base + sequence as u32 * 0xac,
    })
}
