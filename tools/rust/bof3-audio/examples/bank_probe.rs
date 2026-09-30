//! Host-fed archive buffers, original US SDK bank open/transfer/SEP routines.
//! This is a bounded runtime diagnostic, not yet a renderer or disc scheduler.
use bof3_audio::{
    bank::Bank, catalog::loader, digest::sha256_hex, machine::adsr, machine::boot,
    machine::bus::Bus, machine::bus::Width, machine::executable::Executable,
    machine::execution::Execution, machine::firmware::Image, machine::output_clock,
    machine::profile::Profile, machine::spu_clock, machine::spu_reverb, machine::spu_sample,
    machine::spu_voice_ports::DisableModel, sequence::SequenceSet, Result,
};
use emi_ex_v2::image::ArchiveImage;

fn copy(execution: &mut Execution, address: u32, data: &[u8]) -> Result<()> {
    bof3_audio::machine::executable::ram_offset(address, data.len())?;
    for (i, &value) in data.iter().enumerate() {
        execution
            .bus
            .write(address + i as u32, Width::Byte, u32::from(value))?;
    }
    Ok(())
}

fn publish_wave(path: &std::path::Path, pcm: Vec<i16>) -> Result<()> {
    use std::io::Write;
    let wave = bof3_audio::interchange::wave::Wave::new(2, 44100, pcm)?;
    let bytes = wave.to_bytes()?;
    let parent = path
        .parent()
        .filter(|p| !p.as_os_str().is_empty())
        .unwrap_or_else(|| std::path::Path::new("."));
    let temporary = parent.join(format!(".bof3-bank-probe-{}.wav.tmp", std::process::id()));
    let mut file = std::fs::OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(&temporary)?;
    let result = (|| -> Result<()> {
        file.write_all(&bytes)?;
        file.sync_all()?;
        let restored =
            bof3_audio::interchange::wave::Wave::from_bytes(&std::fs::read(&temporary)?)?;
        if restored != wave {
            return Err("diagnostic WAV did not verify".into());
        }
        std::fs::hard_link(&temporary, path)?;
        Ok(())
    })();
    drop(file);
    let cleanup = std::fs::remove_file(&temporary);
    result?;
    cleanup?;
    Ok(())
}

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 3 && args.len() != 5 && args.len() != 6 {
        return Err("usage: bank_probe BIOS US_EXE BGM_EMI [SEQUENCE TICKS [WAV]]".into());
    }
    let playback = if args.len() >= 5 {
        let sequence: u32 = args[3].to_str().ok_or("invalid sequence")?.parse()?;
        let ticks: u32 = args[4].to_str().ok_or("invalid ticks")?.parse()?;
        if sequence >= 4 || !(1..=10_000).contains(&ticks) {
            return Err("sequence must be 0..3 and ticks 1..10000".into());
        }
        Some((sequence, ticks))
    } else {
        None
    };
    let executable = Executable::from_bytes(std::fs::read(&args[1])?)?;
    let profile = Profile::identify(&executable)?;
    let archive = ArchiveImage::from_bytes(std::fs::read(&args[2])?)?;
    let unique = |kinds: &[u16]| -> Result<usize> {
        let entries: Vec<_> = archive
            .entries()
            .iter()
            .enumerate()
            .filter(|(_, e)| kinds.contains(&e.file_type))
            .map(|(i, _)| i)
            .collect();
        if entries.len() != 1 {
            return Err(format!("bank probe requires exactly one entry of types {kinds:?}").into());
        }
        Ok(entries[0])
    };
    let (vh, vb, sep) = (unique(&[6])?, unique(&[7])?, unique(&[9, 10])?);
    let layout = loader::initialize_layout(&executable, 0)?;
    let slot = layout
        .slots
        .get(archive.entries()[vh].ram_ptr as usize)
        .ok_or("invalid archive slot")?;
    let header = archive.entry(vh)?;
    let bank = Bank::parse(header)?;
    let body = archive.entry(vb)?;
    // SDK DMA rounds each final transfer up to a 64-byte block. Retain the
    // actual archive-sector bytes it reads rather than stale staging data.
    let transfer_bytes = body.len().div_ceil(64) * 64;
    let body_start = archive.entries()[vb].offset as usize;
    let transfer_source = archive
        .bytes()
        .get(body_start..body_start + transfer_bytes)
        .ok_or("archive lacks the final DMA block's preservation bytes")?;
    let sequence = archive.entry(sep)?;
    let sequences = SequenceSet::parse(sequence)?;
    if let Some((index, _)) = playback {
        if index as usize >= sequences.sequences.len() {
            return Err(format!("SEP has no sequence index {index}").into());
        }
    }
    if header.len() > slot.header_capacity as usize
        || sequence.len() > slot.sequence_capacity as usize
        || slot.spu_base as usize + transfer_bytes > 512 * 1024
    {
        return Err("probe archive exceeds original layout capacity".into());
    }
    let machine = boot::load_us(
        Image::from_bytes(std::fs::read(&args[0])?)?,
        &executable,
        3_000_000,
    )?;
    let evidence = machine.evidence;
    let mut execution = Execution::new(
        machine.cpu,
        machine.bus,
        spu_clock::Model::EmulatorReference,
    );
    execution.bus.configure_spu_voices(
        spu_sample::Model::EmulatorReference,
        adsr::Model::EmulatorReference,
    )?;
    execution
        .bus
        .configure_spu_disable(DisableModel::EmulatorReference)?;
    execution
        .bus
        .configure_spu_reverb(spu_reverb::Model::EmulatorReference)?;
    let mut calls = Vec::new();
    let mut sequence_ticks = Vec::new();
    let mut pcm = Vec::new();
    let mut stage = "sound_initialize";
    let result = (|| -> Result<()> {
        calls.push(execution.call(profile.sound_initialize, [0; 4], 1_000_000)?);
        stage = "layout_initialize";
        calls.push(execution.call(loader::LAYOUT_INITIALIZE, [0; 4], 1000)?);
        copy(&mut execution, slot.header_address, header)?;
        copy(&mut execution, slot.sequence_address, sequence)?;
        stage = "vab_open";
        let opened = execution.call(
            0x80173c50,
            [
                slot.header_address,
                u32::from(slot.vab_id),
                slot.spu_base,
                0,
            ],
            100_000,
        )?;
        let id = opened.result;
        calls.push(opened);
        if id != u32::from(slot.vab_id) {
            return Err(format!("VAB open returned {id:#x}").into());
        }
        stage = "vab_body";
        for (index, chunk) in body.chunks(2048).enumerate() {
            let start = index * 2048;
            copy(
                &mut execution,
                0x80010000,
                &transfer_source[start..start + chunk.len().div_ceil(64) * 64],
            )?;
            let call =
                execution.call(0x80174354, [0x80010000, chunk.len() as u32, id, 0], 100_000)?;
            let result = call.result;
            calls.push(call);
            if result == u32::MAX {
                return Err("VAB partial transfer rejected".into());
            }
            let wait = execution.call(0x80174598, [1, 0, 0, 0], 100_000)?;
            let completed = wait.result;
            calls.push(wait);
            if completed != 1 {
                return Err(format!("VAB transfer wait returned {completed:#x}").into());
            }
        }
        stage = "body_compare";
        let start = slot.spu_base as usize;
        if execution.bus.spu_transfer().ram()[start..start + transfer_bytes] != *transfer_source {
            return Err("SPU RAM differs from archive VB including final DMA padding".into());
        }
        stage = "sep_open";
        let opened = execution.call(0x8016b38c, [slot.sequence_address, id, 4, 0], 100_000)?;
        let handle = opened.result;
        calls.push(opened);
        if handle >= 2 {
            return Err(format!("SEP open returned invalid handle {handle:#x}").into());
        }
        if let Some((sequence, ticks)) = playback {
            stage = "sequence_play";
            calls.push(execution.call(0x8016b9cc, [handle, sequence, 1, 2], 100_000)?);
            stage = "tick_start";
            calls.push(execution.call(0x8015ce70, [0; 4], 100_000)?);
            let base = execution.bus.read(0x80190308 + handle * 4, Width::Word)?;
            let record = base + sequence * 0xac;
            stage = "sequence_ticks";
            if args.len() == 6 {
                execution.enable_output(output_clock::Model::PcsxReduxNtsc)?;
            }
            for tick in 0..ticks {
                let mut instructions = 0;
                let mut interrupts = 0;
                if args.len() == 6 {
                    while execution.output().unwrap().vblanks() <= u64::from(tick) {
                        let call = execution.call(0x801753dc, [0; 4], 100_000)?;
                        instructions += call.instructions;
                        interrupts += call.interrupts;
                        pcm.extend(execution.take_audio_frames()?.into_iter().flatten());
                    }
                    // A line edge can fall on the leaf's return instruction.
                    // Deliver that pending IRQ before recording this tick.
                    if execution.bus.interrupts().pending() {
                        let call = execution.call(0x801753dc, [0; 4], 100_000)?;
                        instructions += call.instructions;
                        interrupts += call.interrupts;
                        pcm.extend(execution.take_audio_frames()?.into_iter().flatten());
                    }
                } else {
                    execution.bus.set_vblank(true);
                    // GetVideoMode is a bounded guest leaf. Pending VBlank runs
                    // through the BIOS and SDK callback before that leaf returns.
                    let call = execution.call(0x801753dc, [0; 4], 100_000)?;
                    execution.bus.set_vblank(false);
                    instructions = call.instructions;
                    interrupts = call.interrupts;
                }
                if interrupts != 1 {
                    return Err(format!(
                        "tick {tick}: expected one VBlank interrupt, observed {}",
                        interrupts
                    )
                    .into());
                }
                sequence_ticks.push(serde_json::json!({
                    "tick":tick,"interrupts":interrupts,"instructions":instructions,
                    "cursor":execution.bus.read(record + 4, Width::Word)?,
                    "delay":execution.bus.read(record + 0x88, Width::Word)?,
                    "flags":execution.bus.read(record + 0x90, Width::Word)?,
                    "key_on_low":execution.bus.read(0x1f801d88, Width::Half)?,
                    "key_on_high":execution.bus.read(0x1f801d8a, Width::Half)?
                }));
            }
        }
        if args.len() == 6 {
            stage = "wav_publish";
            publish_wave(std::path::Path::new(&args[5]), pcm.clone())?;
        }
        stage = "complete";
        Ok(())
    })();
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.bank-probe/v1","boot":evidence,"archive_sha256":sha256_hex(archive.bytes()),
            "slot":slot,"vh_entry":vh,"vb_entry":vb,"sep_entry":sep,
            "samples":bank.declared_samples,"body_bytes":body.len(),"transfer_bytes":transfer_bytes,"stage":stage,
            "outcome":result.err().map(|e|e.to_string()).unwrap_or_else(||"complete".into()),
        "calls":calls,"pc":execution.cpu.pc(),"registers":(0..32).map(|r|execution.cpu.register(r)).collect::<Vec<_>>(),
        "sequence_ticks":sequence_ticks,
        "output_clock":execution.output(),"pcm_frames":pcm.len()/2,"pcm_peak":pcm.iter().map(|x|x.unsigned_abs()).max(),
            "transfer_clock":execution.clock,"spu_ram_sha256":sha256_hex(execution.bus.spu_transfer().ram()),
            "acceptance":"host supplies archive buffers and layout 0; original guest routines/BIOS and IRQ execution, reference base clocks; optional output/VBlank reference clock; no CD streaming or independent PCM acceptance"
        }))?
    );
    Ok(())
}
