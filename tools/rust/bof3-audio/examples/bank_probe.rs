//! Host-fed archive buffers, original US SDK bank open/transfer/SEP routines.
//! This is a bounded runtime diagnostic, not yet a renderer or disc scheduler.
use bof3_audio::{
    bank::Bank,
    catalog::loader,
    digest::sha256_hex,
    machine::adsr,
    machine::bus::{Bus, Ram, Width},
    machine::cpu::Cpu,
    machine::event_calls::Service,
    machine::exception_calls::{Dispatch, Environment, MissingHook},
    machine::executable::{ram_offset, Executable},
    machine::execution::{Call, Execution},
    machine::interconnect::Interconnect,
    machine::kernel::Kernel,
    machine::output_clock,
    machine::profile::Profile,
    machine::spu_clock,
    machine::spu_reverb,
    machine::spu_sample,
    machine::spu_voice_ports::DisableModel,
    sequence::SequenceSet,
    Result,
};
use emi_ex_v2::image::ArchiveImage;

fn prepare_kernel(
    executable: &Executable,
    image: &[u8],
    metadata: &str,
) -> Result<(Execution, Call)> {
    let mut symbols = std::collections::BTreeMap::new();
    for line in metadata.lines().filter(|line| !line.trim().is_empty()) {
        let fields: Vec<_> = line.split_whitespace().collect();
        if fields.len() != 3 {
            return Err("kernel symbols require defined nm address/type/name rows".into());
        }
        if fields[2].starts_with("audio_") {
            let address = u32::from_str_radix(fields[0], 16)?;
            if symbols.insert(fields[2], address).is_some() {
                return Err(format!("duplicate kernel symbol {}", fields[2]).into());
            }
        }
    }
    let symbol = |name: &str| -> Result<u32> {
        symbols
            .get(name)
            .copied()
            .ok_or_else(|| format!("missing kernel symbol {name}").into())
    };
    let base = symbol("audio_kernel_image")?;
    let end = symbol("audio_kernel_image_end")?;
    let bss = symbol("audio_kernel_bss")?;
    let bss_end = symbol("audio_kernel_bss_end")?;
    let init_stack = symbol("audio_initialization_stack")?;
    let init_top = symbol("audio_initialization_stack_end")?;
    let exception_stack = symbol("audio_exception_stack")?;
    let exception_top = symbol("audio_exception_stack_end")?;
    let image_bytes = u32::try_from(image.len())?;
    if image.is_empty()
        || base < 0x8000_0128
        || base & 3 != 0
        || end.checked_sub(base) != Some(image_bytes)
        || !(end <= bss && bss < init_stack && init_stack < init_top)
        || init_top != exception_stack
        || !(exception_stack < exception_top && exception_top == bss_end)
        || init_top & 7 != 0
        || exception_top & 7 != 0
        || bss_end > 0x8001_0000
    {
        return Err("kernel image/allocation metadata is inconsistent or overlaps staging".into());
    }
    ram_offset(base, (bss_end - base) as usize)?;
    let exe_start = ram_offset(executable.header().load_address, executable.text().len())?;
    let exe_end = exe_start + executable.text().len();
    if ram_offset(base, 0)? < exe_end && exe_start < ram_offset(bss_end, 0)? {
        return Err("kernel allocation overlaps the original executable".into());
    }
    for name in [
        "audio_kernel_initialize",
        "audio_cd_remove",
        "audio_event_deliver",
        "audio_event_loop",
        "audio_event_mode",
        "audio_event_callback",
        "audio_event_return",
        "audio_exception_vector",
        "audio_exception_priority",
        "audio_exception_node",
        "audio_exception_verifier",
        "audio_exception_handler",
        "audio_exception_hook",
        "audio_exception_return",
        "audio_exception_return_rfe",
        "audio_syscall_handler",
    ] {
        let address = symbol(name)?;
        if address & 3 != 0 || !(base..end).contains(&address) {
            return Err(format!("kernel code symbol {name} is outside its image").into());
        }
    }
    for (name, bytes) in [
        ("audio_counter_flags", 16),
        ("audio_exception_hook_pointer", 4),
        ("audio_exception_stack_pointer", 4),
    ] {
        let address = symbol(name)?;
        if address & 3 != 0 || address < base || address.checked_add(bytes).is_none_or(|a| a > end)
        {
            return Err(format!("kernel data symbol {name} is outside its image").into());
        }
    }
    let mut bus = Interconnect::from_ram(Ram::from_executable(executable));
    for (offset, byte) in image.iter().enumerate() {
        bus.write(base + offset as u32, Width::Byte, u32::from(*byte))?;
    }
    bus.write(0x1f80_1014, Width::Word, 0x2009_31e1)?;
    let mut kernel = Kernel::default();
    kernel.configure_counter_flags(symbol("audio_counter_flags")?)?;
    kernel.configure_cd_removal(symbol("audio_cd_remove")?)?;
    kernel.configure_event_delivery(Service {
        entry: symbol("audio_event_deliver")?,
        scan: symbol("audio_event_loop")?,
        mode: symbol("audio_event_mode")?,
        callback: symbol("audio_event_callback")?,
        return_jump: symbol("audio_event_return")?,
    })?;
    kernel.configure_exception_dispatch(Dispatch {
        entry: symbol("audio_exception_vector")?,
        priority: symbol("audio_exception_priority")?,
        node: symbol("audio_exception_node")?,
        verifier: symbol("audio_exception_verifier")?,
        handler: symbol("audio_exception_handler")?,
        exit: symbol("audio_exception_hook")?,
        stack_pointer: symbol("audio_exception_stack_pointer")?,
    })?;
    kernel.configure_syscall_verifier(symbol("audio_syscall_handler")?)?;
    kernel.configure_exception_hook(
        symbol("audio_exception_hook")?,
        symbol("audio_exception_hook_pointer")?,
    )?;
    kernel.configure_exception_return(
        symbol("audio_exception_return")?,
        symbol("audio_exception_return_rfe")?,
    )?;
    let mut cpu = Cpu::new(symbol("audio_kernel_initialize")?);
    cpu.set_register(29, init_top);
    let mut execution = Execution::with_kernel(
        cpu,
        bus,
        spu_clock::Model::EmulatorReference,
        kernel,
        Environment {
            stack_top: exception_top,
            global_pointer: 0,
            missing_hook: MissingHook::ReturnFromException,
        },
    )?;
    let initialized = execution.call(symbol("audio_kernel_initialize")?, [0; 4], 10_000)?;
    if initialized.result != 0 {
        return Err("kernel initialization returned an unexpected result".into());
    }
    // The owned machine explicitly disables automatic PAD acknowledgement.
    execution.cpu.set_register(9, 0x5b);
    execution.call(0xb0, [0; 4], 1)?;
    execution
        .cpu
        .set_register(28, executable.header().global_pointer);
    let stack = executable
        .stack_pointer()
        .ok_or("original executable has no foreground stack")?;
    let stack_start = ram_offset(
        stack.checked_sub(16).ok_or("foreground stack underflows")?,
        16,
    )?;
    if stack_start < ram_offset(bss_end, 0)? && ram_offset(base, 0)? < stack_start + 16 {
        return Err("foreground stack overlaps the owned kernel".into());
    }
    execution.cpu.set_register(29, stack);
    execution.cpu.cop0_mut().write(12, 0x401)?;
    Ok((execution, initialized))
}

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
    if args.len() != 4 && args.len() != 6 && args.len() != 7 {
        return Err(
            "usage: bank_probe KERNEL_BIN KERNEL_SYM US_EXE BGM_EMI [SEQUENCE TICKS [WAV]]".into(),
        );
    }
    let playback = if args.len() >= 6 {
        let sequence: u32 = args[4].to_str().ok_or("invalid sequence")?.parse()?;
        let ticks: u32 = args[5].to_str().ok_or("invalid ticks")?.parse()?;
        if sequence >= 4 || !(1..=10_000).contains(&ticks) {
            return Err("sequence must be 0..3 and ticks 1..10000".into());
        }
        Some((sequence, ticks))
    } else {
        None
    };
    let executable = Executable::from_bytes(std::fs::read(&args[2])?)?;
    let profile = Profile::identify(&executable)?;
    let archive = ArchiveImage::from_bytes(std::fs::read(&args[3])?)?;
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
    let kernel_image = std::fs::read(&args[0])?;
    let kernel_symbols = std::fs::read_to_string(&args[1])?;
    let (mut execution, initialized) = prepare_kernel(&executable, &kernel_image, &kernel_symbols)?;
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
            if args.len() == 7 {
                execution.enable_output(output_clock::Model::PcsxReduxNtsc)?;
            }
            for tick in 0..ticks {
                let mut instructions = 0;
                let mut interrupts = 0;
                if args.len() == 7 {
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
                    // through owned services and the SDK callback before returning.
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
        if args.len() == 7 {
            stage = "wav_publish";
            publish_wave(std::path::Path::new(&args[6]), pcm.clone())?;
        }
        stage = "complete";
        Ok(())
    })();
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.bank-probe/v2","kernel_sha256":sha256_hex(&kernel_image),
            "kernel_symbols_sha256":sha256_hex(kernel_symbols.as_bytes()),"kernel_initialization":initialized,
            "archive_sha256":sha256_hex(archive.bytes()),
            "slot":slot,"vh_entry":vh,"vb_entry":vb,"sep_entry":sep,
            "samples":bank.declared_samples,"body_bytes":body.len(),"transfer_bytes":transfer_bytes,"stage":stage,
            "outcome":result.err().map(|e|e.to_string()).unwrap_or_else(||"complete".into()),
        "calls":calls,"pc":execution.cpu.pc(),"registers":(0..32).map(|r|execution.cpu.register(r)).collect::<Vec<_>>(),
        "sequence_ticks":sequence_ticks,
        "output_clock":execution.output(),"pcm_frames":pcm.len()/2,"pcm_peak":pcm.iter().map(|x|x.unsigned_abs()).max(),
            "transfer_clock":execution.clock,"spu_ram_sha256":sha256_hex(execution.bus.spu_transfer().ram()),
            "acceptance":"host supplies archive buffers, layout 0, explicit device inputs and a linked owned kernel; original game/SDK routines remain required; base clocks and optional output/VBlank clock; no firmware boot, CD streaming or independent PCM acceptance"
        }))?
    );
    Ok(())
}
