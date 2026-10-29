//! Owned VAB planning, initialization and upload for music and effects.
use super::{
    bus::{Bus, Width},
    cpu::Cpu,
    driver::{self, Audio},
    execution::{Call, Execution},
};
use crate::{bank::Bank, catalog::loader, digest::sha256_hex, Result};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct Identity {
    pub archive_sha256: String,
    pub header_entry: usize,
    pub header_bytes: usize,
    pub body_entry: usize,
    pub sequence_entry: Option<usize>,
    pub header_bank_id: u32,
    pub game_bank_id: u16,
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
    pub bank: Bank,
}
fn copy(execution: &mut Execution, address: u32, bytes: &[u8]) -> Result<()> {
    super::executable::ram_offset(address, bytes.len())?;
    for (offset, &byte) in bytes.iter().enumerate() {
        execution
            .bus
            .write(address + offset as u32, Width::Byte, u32::from(byte))?;
    }
    Ok(())
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Stage {
    Initialization,
    Layout,
    Header,
    Samples,
    Sequence,
    Callbacks,
}

pub struct Options {
    pub header_entry: usize,
    pub body_entry: usize,
    pub sequence_entry: Option<usize>,
    pub layout: Option<u8>,
}

pub fn prepare(
    image: &driver::Image,
    archive: &ArchiveImage,
    options: &Options,
    observer: &mut impl FnMut(Stage, &Execution) -> Result<()>,
) -> Result<Prepared> {
    let plan = plan(image, archive, options)?;
    let audio = image.audio()?;
    let started = image.start(&mut |e| observer(Stage::Initialization, e))?;
    load(
        started.execution,
        started.evidence,
        vec![started.initialization],
        audio,
        plan,
        observer,
    )
}

pub(crate) fn load<P>(
    mut execution: Execution,
    startup: P,
    mut calls: Vec<Call>,
    audio: Audio,
    plan: Plan<'_>,
    observer: &mut impl FnMut(Stage, &Execution) -> Result<()>,
) -> Result<Prepared<P>> {
    let Plan {
        identity,
        bank,
        header,
        sequence: sep_bytes,
        transfer: source,
    } = plan;
    let slot = &identity.slot;
    let layout = identity.layout;
    let dma_bytes = identity.dma_bytes;
    let body = &source[..identity.body_bytes];
    let mut call = |execution: &mut Execution, stage: Stage, entry, arguments, limit| {
        execution.call_observed(entry, arguments, limit, &mut |e| observer(stage, e))
    };
    calls.extend([
        call(
            &mut execution,
            Stage::Initialization,
            audio.initialize,
            [0; 4],
            1_000_000,
        )?,
        call(
            &mut execution,
            Stage::Layout,
            audio.layout,
            [u32::from(layout), 0, 0, 0],
            1000,
        )?,
    ]);
    let reverb_base = execution.bus.read(0x1f801da2, Width::Half)? * 8;
    if slot.spu_base as usize + dma_bytes > reverb_base as usize {
        return Err(format!(
            "PSX bank: VB transfer would overlap guest reverb work area at {reverb_base:#x}"
        )
        .into());
    }
    copy(&mut execution, slot.header_address, header)?;
    copy(&mut execution, slot.sequence_address, sep_bytes)?;
    let id = u32::from(slot.vab_id);
    let opened = call(
        &mut execution,
        Stage::Header,
        audio.open_head,
        [slot.header_address, id, slot.spu_base, 0],
        100_000,
    )?;
    if opened.result != id {
        return Err(format!("PSX bank: VAB open returned {:#x}", opened.result).into());
    }
    calls.push(opened);
    for (index, chunk) in body.chunks(2048).enumerate() {
        let start = index * 2048;
        copy(
            &mut execution,
            0x80010000,
            &source[start..start + chunk.len().div_ceil(64) * 64],
        )?;
        let transfer = call(
            &mut execution,
            Stage::Samples,
            audio.transfer,
            [0x80010000, chunk.len() as u32, id, 0],
            100_000,
        )?;
        let expected = if start + chunk.len() == body.len() {
            id
        } else {
            0xffff_fffe
        };
        if transfer.result != expected {
            return Err(format!(
                "PSX bank: VB chunk {index} returned {:#x}, expected {expected:#x}",
                transfer.result
            )
            .into());
        }
        calls.push(transfer);
        let wait = call(
            &mut execution,
            Stage::Samples,
            audio.completed,
            [1, 0, 0, 0],
            100_000,
        )?;
        if wait.result != 1 {
            return Err(format!("PSX bank: VB wait returned {:#x}", wait.result).into());
        }
        calls.push(wait);
    }
    let start = slot.spu_base as usize;
    if execution.bus.spu_transfer().ram()[start..start + dma_bytes] != *source {
        return Err("PSX bank: SPU RAM differs from VB including DMA padding".into());
    }
    Ok(Prepared {
        execution,
        startup,
        audio,
        identity,
        calls,
        bank,
    })
}

/// Validated single-bank staging shared by guest rendering and PS-X compilation.
pub struct Plan<'a> {
    pub identity: Identity,
    pub bank: Bank,
    pub header: &'a [u8],
    pub sequence: &'a [u8],
    pub transfer: &'a [u8],
}

pub fn plan<'a>(
    image: &driver::Image,
    archive: &'a ArchiveImage,
    options: &Options,
) -> Result<Plan<'a>> {
    plan_with_layout(archive, options, |selector| layout(image, selector))
}

pub(crate) fn plan_with_layout<'a>(
    archive: &'a ArchiveImage,
    options: &Options,
    mut provider: impl FnMut(u8) -> Result<crate::catalog::model::Layout>,
) -> Result<Plan<'a>> {
    if options.layout.is_some_and(|v| v > 2) {
        return Err("US audio layout selector must be 0, 1, or 2".into());
    }
    let vh = options.header_entry;
    let vb = options.body_entry;
    for (index, kind) in [(vh, 6), (vb, 7)] {
        if archive
            .entries()
            .get(index)
            .is_none_or(|entry| entry.file_type != kind)
        {
            return Err(format!("PSX bank: entry {index} must have type {kind}").into());
        }
    }
    if vb <= vh
        || archive.entries()[vh + 1..vb]
            .iter()
            .any(|entry| entry.file_type == 6)
    {
        return Err("PSX bank: body is not owned by the selected header".into());
    }
    let header = archive.entry(vh)?;
    let body = archive.entry(vb)?;
    let sep_bytes = if let Some(index) = options.sequence_entry {
        if archive
            .entries()
            .get(index)
            .is_none_or(|entry| !matches!(entry.file_type, 9 | 10))
        {
            return Err("PSX bank: sequence staging requires a sequence entry".into());
        }
        if index <= vh
            || archive.entries()[vh + 1..index]
                .iter()
                .any(|entry| entry.file_type == 6)
        {
            return Err("PSX bank: sequence is not owned by the selected header".into());
        }
        archive.entry(index)?
    } else {
        &[]
    };
    let bank = Bank::parse(header)?;
    let requested_layout = options.layout;
    let expected_body = bank
        .samples
        .last()
        .map_or(bank.body_prefix_bytes, |s| s.body_offset + s.encoded_bytes);
    if expected_body != body.len() {
        return Err(format!(
            "PSX bank: VH describes {expected_body} VB bytes, entry {vb} has {}",
            body.len()
        )
        .into());
    }
    let dma_bytes = body.len().div_ceil(64) * 64;
    let mut failures = Vec::new();
    let candidates: Vec<u8> = requested_layout.map_or_else(|| vec![0, 1, 2], |v| vec![v]);
    let mut selection = None;
    for layout in candidates {
        let slots = provider(layout)?;
        let slot = slots
            .slots
            .get(archive.entries()[vh].ram_ptr as usize)
            .ok_or("PSX bank: invalid game bank slot")?;
        let next_slot_boundary = slots
            .slots
            .iter()
            .map(|s| s.spu_base)
            .filter(|&base| base > slot.spu_base)
            .min()
            .unwrap_or(512 * 1024);
        if header.len() <= slot.header_capacity as usize
            && sep_bytes.len() <= slot.sequence_capacity as usize
            && slot.spu_base as usize + dma_bytes <= next_slot_boundary as usize
        {
            selection = Some((layout, slot.clone(), next_slot_boundary));
            break;
        }
        failures.push(format!("layout {layout} slot {} capacity: VH {}/{}, SEP {}/{}, SPU DMA end {:#x}, next slot boundary {next_slot_boundary:#x}", slot.index, header.len(), slot.header_capacity, sep_bytes.len(), slot.sequence_capacity, slot.spu_base as usize + dma_bytes));
    }
    let (layout, slot, next_slot_boundary) = selection.ok_or_else(|| {
        format!(
            "PSX bank: archive exceeds available layout capacity: {}",
            failures.join("; ")
        )
    })?;
    let offset = archive.entries()[vb].offset as usize;
    let source = archive
        .bytes()
        .get(offset..offset + dma_bytes)
        .ok_or("PSX bank: missing preserved final DMA padding")?;
    let identity = Identity {
        archive_sha256: sha256_hex(archive.bytes()),
        header_entry: vh,
        header_bytes: header.len(),
        body_entry: vb,
        sequence_entry: options.sequence_entry,
        header_bank_id: bank.header_id,
        game_bank_id: slot.vab_id,
        layout,
        requested_layout,
        slot: slot.clone(),
        body_bytes: body.len(),
        dma_bytes,
        next_slot_boundary,
    };
    Ok(Plan {
        identity,
        bank,
        header,
        sequence: sep_bytes,
        transfer: source,
    })
}

/// Execute a candidate layout in isolated scratch RAM, without changing playback.
pub fn layout(image: &driver::Image, selector: u8) -> Result<crate::catalog::model::Layout> {
    use crate::catalog::model::{Layout, Slot};
    if selector > 2 {
        return Err("US audio layout selector must be 0, 1, or 2".into());
    }
    let audio = image.audio()?;
    let mut ram = image.ram()?;
    let mut cpu = Cpu::new(audio.layout);
    cpu.set_register(4, u32::from(selector));
    cpu.set_register(31, 0x8000_1000);
    let instructions = cpu.run_until(&mut ram, 0x8000_1000, 1000)?;
    let mut slots = Vec::new();
    for index in 0..loader::LOADER_SLOTS {
        let address = audio.slot_table + index as u32 * 20;
        let table_index = u32::from(selector) * 28 + index as u32 * 4;
        slots.push(Slot {
            index,
            spu_base: ram.read(address, Width::Word)?,
            header_address: ram.read(address + 4, Width::Word)?,
            auxiliary_address: ram.read(address + 8, Width::Word)?,
            sequence_address: ram.read(address + 12, Width::Word)?,
            vab_id: ram.read(address + 16, Width::Half)? as u16,
            flags: ram.read(address + 18, Width::Half)? as u16,
            header_capacity: ram.read(audio.header_capacities + table_index, Width::Word)?,
            sequence_capacity: ram.read(audio.sequence_capacities + table_index, Width::Word)?,
        });
    }
    Ok(Layout {
        selector,
        instructions,
        slots,
    })
}
