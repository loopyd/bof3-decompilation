//! Bounded ROM execution with explicit RAM/register inputs; no HLE BIOS dispatch.
use bof3_audio::{
    digest::sha256_hex,
    machine::{
        adsr,
        bus::Ram,
        cpu::{Cpu, FaultKind},
        executable::RAM_BYTES,
        firmware::{Image, ROM_BYTES},
        interconnect::Interconnect,
        spu_clock, spu_reverb, spu_sample,
        spu_voice_ports::DisableModel,
    },
    Result,
};
use serde::{Deserialize, Serialize};
use std::{collections::VecDeque, io::Read, path::PathBuf};

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct Context {
    schema: String,
    provenance: String,
    #[serde(default)]
    reset_profile: Option<String>,
    #[serde(default)]
    spu: Option<SpuModels>,
    #[serde(default)]
    transfer_clock: Option<spu_clock::Model>,
    ram: Option<PathBuf>,
    entry_pc: u32,
    status: u32,
    registers: [u32; 32],
    stop_pc: Option<u32>,
    transition_limit: u64,
}

#[derive(Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
struct SpuModels {
    sample: spu_sample::Model,
    envelope: adsr::Model,
    disable: Option<DisableModel>,
    reverb: Option<spu_reverb::Model>,
}

#[derive(Serialize)]
#[serde(untagged)]
enum Recent {
    Instruction {
        pc: u32,
        instruction: u32,
        delay_slot: bool,
        branch_target: Option<u32>,
    },
    Exception {
        pc: u32,
        exception: &'static str,
        vector: u32,
    },
}

impl SpuModels {
    fn configure(&self, bus: &mut Interconnect) -> Result<()> {
        bus.configure_spu_voices(self.sample, self.envelope)?;
        if let Some(model) = self.disable {
            bus.configure_spu_disable(model)?;
        }
        if let Some(model) = self.reverb {
            bus.configure_spu_reverb(model)?;
        }
        Ok(())
    }
}
fn read_bounded(path: &std::path::Path, limit: usize) -> Result<Vec<u8>> {
    let mut bytes = Vec::new();
    std::fs::File::open(path)?
        .take(limit as u64 + 1)
        .read_to_end(&mut bytes)?;
    if bytes.len() > limit {
        return Err(format!("input exceeds {limit} bytes: {}", path.display()).into());
    }
    Ok(bytes)
}
fn main() -> Result<()> {
    let mut args = std::env::args_os().skip(1);
    let rom_path = PathBuf::from(args.next().ok_or("usage: bios_probe BIOS CONTEXT_JSON")?);
    let context_path = PathBuf::from(args.next().ok_or("missing context JSON")?);
    if args.next().is_some() {
        return Err("usage: bios_probe BIOS CONTEXT_JSON".into());
    }
    let context_bytes = read_bounded(&context_path, 65536)?;
    let context: Context = serde_json::from_slice(&context_bytes)?;
    if context.schema != "bof3.bios-probe-context/v1"
        || context.provenance.trim().is_empty()
        || context.registers[0] != 0
        || context.entry_pc & 3 != 0
        || context.stop_pc.is_some_and(|pc| pc & 3 != 0)
        || !(1..=100_000_000).contains(&context.transition_limit)
    {
        return Err("invalid probe context schema/provenance, zero register, PC alignment or transition limit (1..=100000000)".into());
    }
    let image = Image::from_bytes(read_bounded(&rom_path, ROM_BYTES)?)?;
    let rom_hash = image.sha256().to_owned();
    let ram = match &context.ram {
        Some(path) => Ram::from_bytes(read_bounded(
            &context_path
                .parent()
                .unwrap_or(std::path::Path::new("."))
                .join(path),
            RAM_BYTES,
        )?)?,
        None => Ram::default(),
    };
    let ram_hash = sha256_hex(ram.bytes());
    let mut bus = Interconnect::from_ram(ram);
    bus.attach_firmware(image)?;
    if let Some(models) = &context.spu {
        models.configure(&mut bus)?;
    }
    if context.transfer_clock.is_some() && context.reset_profile.as_deref() != Some("pcsx-redux") {
        return Err(
            "transfer-clock probe requires the explicit PCSX-Redux CPU reset profile".into(),
        );
    }
    let mut transfer_clock = context.transfer_clock.map(spu_clock::Clock::new);
    let mut cpu = match context.reset_profile.as_deref() {
        None => {
            let mut cpu = Cpu::new(context.entry_pc);
            cpu.cop0_mut().write(12, context.status)?;
            cpu
        }
        Some("pcsx-redux") => {
            if context.ram.is_some()
                || context.registers != [0; 32]
                || context.entry_pc != 0xbfc0_0000
                || context.status != 0x1090_0000
            {
                return Err(
                    "PCSX-Redux reset profile requires its exact zero RAM/register seed, PC and SR"
                        .into(),
                );
            }
            Cpu::pcsx_redux_reset()
        }
        Some(_) => return Err("unsupported reset profile".into()),
    };
    if cpu.cop0().status() != context.status {
        return Err("probe status contains unsupported/reserved bits".into());
    }
    for (r, v) in context.registers.into_iter().enumerate() {
        cpu.set_register(r, v);
    }
    let mut recent = VecDeque::with_capacity(17);
    let mut exceptions = 0;
    let mut transitions = 0;
    let mut outcome = "transition_limit".to_owned();
    loop {
        if Some(cpu.pc()) == context.stop_pc {
            outcome = "stop_pc_reached".into();
            break;
        }
        if transitions == context.transition_limit {
            break;
        }
        transitions += 1;
        match cpu.step(&mut bus) {
            Ok(step) => recent.push_back(Recent::Instruction {
                pc: step.pc,
                instruction: step.instruction,
                delay_slot: step.in_delay_slot,
                branch_target: step.branch_target,
            }),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                cpu.enter_exception(&fault)?;
                exceptions += 1;
                recent.push_back(Recent::Exception {
                    pc: fault.pc,
                    exception: "syscall",
                    vector: cpu.pc(),
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
        if let Some(clock) = &mut transfer_clock {
            // Pinned PCSX-Redux interpreter BIAS=2 base ticks per instruction.
            // Memory stalls, frame clocks and DMA arbitration remain absent.
            if let Err(error) = clock.advance(&mut bus, 2) {
                outcome = format!("SPU transfer clock: {error}");
                break;
            }
        }
    }
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema":"bof3.bios-probe/v1", "rom_sha256":rom_hash,
            "context_sha256":sha256_hex(&context_bytes), "provenance":context.provenance,
            "reset_profile":context.reset_profile,
            "spu_models":context.spu,
            "transfer_clock":transfer_clock,
            "spu_ram_sha256":sha256_hex(bus.spu_transfer().ram()),
            "spu_fifo_halfwords":bus.spu_transfer().fifo_halfwords(),
            "spu_transfer_address":bus.spu_transfer().current_address(),
            "timing_limits":"optional transfer clock uses two base ticks per CPU instruction/exception; no memory stalls, voice frames, timer/CD clocks or DMA arbitration",
            "initial_ram_sha256":ram_hash,"final_ram_sha256":sha256_hex(bus.ram().bytes()),
            "ram_input":context.ram,"initial_entry_pc":context.entry_pc,"initial_status":context.status,
            "initial_registers":context.registers,"transition_limit":context.transition_limit,"stop_pc":context.stop_pc,
            "outcome":outcome,"transitions":transitions,"instructions":cpu.instructions(),
            "syscall_exceptions":exceptions,"pc":cpu.pc(),"cop0":cpu.cop0(),
            "registers":(0..32).map(|r|cpu.register(r)).collect::<Vec<_>>(),
            "hi":cpu.hi(),"lo":cpu.lo(),"recent":recent,"post_status":bus.post_status(),
        "initial_state_limits":"HI/LO, Cause/EPC/BadVaddr/TAR zero; PRID=2; no pending load/branch; devices use model defaults; RAM input is not a complete save-state",
            "acceptance":"unverified BIOS revision; no HLE interception, clock scheduling, hardware reset or audio fidelity claim"
        }))?
    );
    Ok(())
}
