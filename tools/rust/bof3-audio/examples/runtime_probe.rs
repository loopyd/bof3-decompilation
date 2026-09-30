//! Original US executable or explicitly selected sound-routine diagnostic after
//! the BIOS shell-load handoff. No HLE BIOS dispatch or invented kernel tables.
use bof3_audio::{
    digest::sha256_hex,
    machine::{
        adsr, boot,
        cpu::{Cpu, FaultKind},
        executable::Executable,
        firmware::Image,
        profile::Profile,
        spu_clock, spu_reverb, spu_sample,
        spu_voice_ports::DisableModel,
    },
    Result,
};
use serde::Serialize;
use std::{collections::VecDeque, io::Read, path::Path};

fn read(path: &Path, limit: usize) -> Result<Vec<u8>> {
    let mut bytes = Vec::new();
    std::fs::File::open(path)?
        .take(limit as u64 + 1)
        .read_to_end(&mut bytes)?;
    if bytes.len() > limit {
        return Err("runtime probe input exceeds size limit".into());
    }
    Ok(bytes)
}

#[derive(Serialize)]
struct Trace {
    pc: u32,
    instruction: Option<u32>,
    delay_slot: bool,
    exception: bool,
}

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 3 {
        return Err("usage: runtime_probe BIOS EXE entry|callback-init|sound-init".into());
    }
    let executable = Executable::from_bytes(read(Path::new(&args[1]), 4 * 1024 * 1024)?)?;
    let profile = Profile::identify(&executable)?;
    let mode = args[2].to_str().ok_or("non-UTF8 probe mode")?;
    let routine = match mode {
        "entry" => None,
        "callback-init" => Some(0x8017_48e4),
        "sound-init" => Some(profile.sound_initialize),
        _ => return Err("unknown runtime probe mode".into()),
    };
    let mut machine = boot::load_us(
        Image::from_bytes(read(Path::new(&args[0]), 512 * 1024)?)?,
        &executable,
        3_000_000,
    )?;
    if let Some(entry) = routine {
        let mut cpu = Cpu::new(entry);
        for register in 1..32 {
            cpu.set_register(register, machine.cpu.register(register));
        }
        *cpu.cop0_mut() = machine.cpu.cop0().clone();
        for register in 4..8 {
            cpu.set_register(register, 0);
        }
        cpu.set_register(31, 0x8001_0000);
        machine.cpu = cpu;
    }
    machine.bus.configure_spu_voices(
        spu_sample::Model::EmulatorReference,
        adsr::Model::EmulatorReference,
    )?;
    machine
        .bus
        .configure_spu_disable(DisableModel::EmulatorReference)?;
    machine
        .bus
        .configure_spu_reverb(spu_reverb::Model::EmulatorReference)?;
    let mut clock = spu_clock::Clock::new(spu_clock::Model::EmulatorReference);
    let initial_instructions = machine.cpu.instructions();
    let initial_registers: Vec<_> = (0..32).map(|r| machine.cpu.register(r)).collect();
    let mut recent = VecDeque::with_capacity(17);
    let mut exceptions = 0;
    let mut outcome = "transition_limit".to_owned();
    for _ in 0..1_000_000 {
        if routine.is_some() && machine.cpu.pc() == 0x8001_0000 {
            outcome = "routine_returned".into();
            break;
        }
        match machine.cpu.step(&mut machine.bus) {
            Ok(step) => recent.push_back(Trace {
                pc: step.pc,
                instruction: Some(step.instruction),
                delay_slot: step.in_delay_slot,
                exception: false,
            }),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                machine.cpu.enter_exception(&fault)?;
                exceptions += 1;
                recent.push_back(Trace {
                    pc: fault.pc,
                    instruction: fault.instruction,
                    delay_slot: fault.in_delay_slot,
                    exception: true,
                });
            }
            Err(fault) => {
                outcome = fault.to_string();
                break;
            }
        }
        if recent.len() > 16 {
            recent.pop_front();
        }
        if let Err(error) = clock.advance(&mut machine.bus, 2) {
            outcome = format!("SPU transfer clock: {error}");
            break;
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.runtime-probe/v1", "boot":machine.evidence,
            "mode":mode, "initial_registers":initial_registers,
            "instructions":machine.cpu.instructions()-initial_instructions,
            "syscall_exceptions":exceptions, "outcome":outcome,
            "pc":machine.cpu.pc(), "cop0":machine.cpu.cop0(),
            "registers":(0..32).map(|r|machine.cpu.register(r)).collect::<Vec<_>>(),
            "recent":recent,"transfer_clock":clock,
            "final_ram_sha256":sha256_hex(machine.bus.ram().bytes()),
            "spu_ram_sha256":sha256_hex(machine.bus.spu_transfer().ram()),
            "context_limits":"explicit emulator-reference SPU sample/envelope/disable/reverb models; no audio frames, CD/timer/GPU clocks or IRQ scheduler; callback/sound modes replace CPU entry with a diagnostic call, handoff GPR/COP0, zero HI/LO and arguments, sentinel RA",
            "acceptance":"original BIOS kernel execution; not full startup, clock timing or audio fidelity evidence"
        }))?
    );
    Ok(())
}
