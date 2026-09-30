//! Original BIOS execution to PCSX-Redux's PS-X EXE shell-load handoff.
//! This prepares kernel RAM, not the game's sound driver or device clocks.
use super::{
    bus::Ram,
    cpu::{Cpu, FaultKind},
    executable::Executable,
    firmware::{Image, US_SHA256},
    interconnect::Interconnect,
    profile::Profile,
};
use crate::{digest::sha256_hex, Result};
use serde::Serialize;

pub const SHELL_ENTRY: u32 = 0x8003_0000;

#[derive(Debug, Serialize)]
pub struct Evidence {
    pub bios_sha256: String,
    pub executable_sha256: String,
    pub bios_instructions: u64,
    pub syscall_exceptions: u64,
    pub shell_ram_sha256: String,
    pub loaded_ram_sha256: String,
    pub shell_return: u32,
    pub shell_stack: u32,
    pub entry: u32,
    pub stack: u32,
    pub global_pointer: u32,
}

pub struct Machine {
    pub cpu: Cpu,
    pub bus: Interconnect,
    pub evidence: Evidence,
}

/// Only the exact US BIOS/executable pair is supported. Execute ROM code until
/// its call into the shell, then load the EXE payload and set PC/SP like Redux's
/// UI::shellReached + BinaryLoader::loadPSEXE. That path does not set GP, clear
/// BSS, reset devices, or execute the shell. FastBoot's separate display-enable
/// write is not part of this EXE-load path. No BIOS calls are HLE-intercepted.
pub fn load_us(firmware: Image, executable: &Executable, transition_limit: u64) -> Result<Machine> {
    if firmware.sha256() != US_SHA256 {
        return Err("unsupported BIOS for shell handoff; US SCPH-5501 SHA-256 required".into());
    }
    let profile = Profile::identify(executable)?;
    if !(1..=10_000_000).contains(&transition_limit) {
        return Err("BIOS shell handoff requires transition limit 1..=10000000".into());
    }
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.attach_firmware(firmware)?;
    let mut cpu = Cpu::pcsx_redux_reset();
    let mut exceptions = 0;
    for _ in 0..transition_limit {
        if cpu.pc() == SHELL_ENTRY {
            break;
        }
        match cpu.step(&mut bus) {
            Ok(_) => (),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                cpu.enter_exception(&fault)?;
                exceptions += 1;
            }
            Err(fault) => return Err(fault.into()),
        }
    }
    if cpu.pc() != SHELL_ENTRY {
        return Err(format!("BIOS shell handoff safety limit at PC {:#010x}", cpu.pc()).into());
    }
    let shell_ram_sha256 = sha256_hex(bus.ram().bytes());
    let shell_return = cpu.register(31);
    let shell_stack = cpu.register(29);
    bus.load_executable(executable)?;
    // The supported US header has a zero stack offset; Redux uses stack_base.
    if executable.header().stack_base != 0 {
        cpu.set_register(29, executable.header().stack_base);
    }
    cpu.resume_at(executable.header().entry_pc);
    let evidence = Evidence {
        bios_sha256: US_SHA256.into(),
        executable_sha256: profile.exe_sha256,
        bios_instructions: cpu.instructions(),
        syscall_exceptions: exceptions,
        shell_ram_sha256,
        loaded_ram_sha256: sha256_hex(bus.ram().bytes()),
        shell_return,
        shell_stack,
        entry: cpu.pc(),
        stack: cpu.register(29),
        global_pointer: cpu.register(28),
    };
    Ok(Machine { cpu, bus, evidence })
}
