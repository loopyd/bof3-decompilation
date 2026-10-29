//! Validated owned guest image and cold service startup.
use super::{
    adsr,
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    event_calls::Service,
    exception_calls::{Dispatch, Environment, MissingHook},
    executable::ram_offset,
    execution::{Call, Execution},
    interconnect::Interconnect,
    kernel::Kernel,
    spu_clock, spu_reverb, spu_sample,
    spu_voice_ports::DisableModel,
};
use crate::{digest::sha256_hex, Result};
use serde::{Deserialize, Serialize};
use std::{collections::BTreeMap, path::Path};

const FOREGROUND_STACK: u32 = 0x801f_ff00;
const FOREGROUND_STACK_START: u32 = 0x801f_0000;

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct Manifest {
    schema: String,
    abi: String,
    image_sha256: String,
    symbols_sha256: String,
}

#[derive(Clone, Debug, Serialize)]
pub struct Evidence {
    pub schema: &'static str,
    pub image_sha256: String,
    pub symbols_sha256: String,
    pub image_bytes: usize,
    pub foreground_stack: u32,
    pub initial_status: u32,
}

pub struct Image {
    bytes: Vec<u8>,
    symbols: BTreeMap<String, (u32, char)>,
    evidence: Evidence,
}

pub struct Started {
    pub execution: Execution,
    pub initialization: Call,
    pub evidence: Evidence,
}

#[derive(Clone, Copy)]
pub(crate) struct Audio {
    pub initialize: u32,
    pub layout: u32,
    pub slot_table: u32,
    pub header_capacities: u32,
    pub sequence_capacities: u32,
    pub open_head: u32,
    pub transfer: u32,
    pub completed: u32,
    pub sequence_open: u32,
    pub callbacks: u32,
    pub sequence_handles: u32,
    pub play: u32,
    pub stop: u32,
    pub idle: u32,
    pub key_on: u32,
    pub key_off: u32,
    pub cue_dispatch: u32,
    pub cue_refresh: u32,
    pub auxiliary_end: u32,
    pub cue_gate: u32,
    pub cue_work: u32,
    pub voice_status: u32,
    pub cue_lookup: u32,
    pub tone_read: u32,
    pub loop_start: u32,
    pub infinite_loop: u32,
    pub sequence_end: u32,
}

impl Image {
    /// Read only the built owned artifact and its integrity metadata.
    pub fn read(directory: &Path) -> Result<Self> {
        Self::parse(
            std::fs::read(directory.join("kernel.bin"))?,
            &std::fs::read_to_string(directory.join("kernel.sym"))?,
            &std::fs::read_to_string(directory.join("kernel.json"))?,
        )
    }

    pub fn parse(bytes: Vec<u8>, metadata: &str, manifest: &str) -> Result<Self> {
        let manifest: Manifest = serde_json::from_str(manifest)?;
        let image_sha256 = sha256_hex(&bytes);
        let symbols_sha256 = sha256_hex(metadata.as_bytes());
        if manifest.schema != "bof3.audio-driver/v1"
            || manifest.abi != "mips1-le-o32"
            || manifest.image_sha256 != image_sha256
            || manifest.symbols_sha256 != symbols_sha256
        {
            return Err("owned driver: unsupported manifest or mismatched artifact hashes".into());
        }
        let mut symbols = BTreeMap::new();
        for line in metadata.lines().filter(|line| !line.trim().is_empty()) {
            let fields: Vec<_> = line.split_whitespace().collect();
            if fields.len() != 3 || fields[1].len() != 1 {
                return Err("owned driver: expected defined nm address/type/name rows".into());
            }
            let address = u32::from_str_radix(fields[0], 16)?;
            let kind = fields[1].chars().next().unwrap();
            if symbols
                .insert(fields[2].to_owned(), (address, kind))
                .is_some()
            {
                return Err(format!("owned driver: duplicate symbol {}", fields[2]).into());
            }
        }
        let image = Self {
            evidence: Evidence {
                schema: "bof3.audio-startup/v1",
                image_sha256,
                symbols_sha256,
                image_bytes: bytes.len(),
                foreground_stack: FOREGROUND_STACK,
                initial_status: 0x401,
            },
            bytes,
            symbols,
        };
        image.validate()?;
        Ok(image)
    }

    pub fn symbol(&self, name: &str) -> Result<u32> {
        self.symbols
            .get(name)
            .map(|&(address, _)| address)
            .ok_or_else(|| format!("owned driver: missing symbol {name}").into())
    }

    pub fn entry(&self, name: &str) -> Result<u32> {
        let address = self.symbol(name)?;
        let kind = self.symbols[name].1;
        let low =
            self.symbol("audio_kernel_image")?..self.symbol("audio_sequence_scheduler_load")?;
        let scheduler = self.symbol("audio_sequence_scheduler")?
            ..self.symbol("audio_sequence_scheduler_end")?;
        if !matches!(kind, 'T' | 't')
            || address & 3 != 0
            || !(low.contains(&address) || scheduler.contains(&address))
        {
            return Err(
                format!("owned driver: code symbol {name} is outside executable sections").into(),
            );
        }
        Ok(address)
    }

    fn validate(&self) -> Result<()> {
        let base = self.symbol("audio_kernel_image")?;
        let end = self.symbol("audio_kernel_image_end")?;
        let bss = self.symbol("audio_kernel_bss")?;
        let state_end = self.symbol("audio_kernel_state_end")?;
        let init_stack = self.symbol("audio_initialization_stack")?;
        let init_top = self.symbol("audio_initialization_stack_end")?;
        let exception_stack = self.symbol("audio_exception_stack")?;
        let exception_top = self.symbol("audio_exception_stack_end")?;
        let bss_end = self.symbol("audio_kernel_bss_end")?;
        if self.bytes.is_empty()
            || base != 0x8002_0000
            || end.checked_sub(base) != Some(u32::try_from(self.bytes.len())?)
            || !(end <= bss
                && bss < state_end
                && state_end.checked_add(7).map(|v| v & !7) == Some(init_stack))
            || init_top.checked_sub(init_stack) != Some(1024)
            || init_top != exception_stack
            || exception_top.checked_sub(exception_stack) != Some(4096)
            || exception_top != bss_end
            || bss_end > 0x8005_0000
            || state_end & 3 != 0
            || [bss, init_stack, init_top, exception_top]
                .iter()
                .any(|v| v & 7 != 0)
        {
            return Err("owned driver: image and allocation metadata is inconsistent".into());
        }
        ram_offset(base, (bss_end - base) as usize)?;
        let scheduler = self.symbol("audio_sequence_scheduler")?;
        let scheduler_end = self.symbol("audio_sequence_scheduler_end")?;
        let load = self.symbol("audio_sequence_scheduler_load")?;
        let load_end = self.symbol("audio_sequence_scheduler_load_end")?;
        if scheduler != 0x8016_c548
            || scheduler_end != 0x8016_c7bc
            || load & 3 != 0
            || load < base
            || load_end.checked_sub(load) != Some(628)
            || load_end != end
            || bss_end > scheduler
        {
            return Err("owned driver: scheduler metadata is inconsistent".into());
        }
        ram_offset(scheduler, 628)?;
        ram_offset(FOREGROUND_STACK_START, 64 * 1024)?;
        if bss_end > FOREGROUND_STACK_START
            || scheduler_end > FOREGROUND_STACK_START
            || self.symbol("audio_exception_global_pointer")? != 0
        {
            return Err(
                "owned driver: foreground stack or exception GP conflicts with startup policy"
                    .into(),
            );
        }
        for (name, bytes) in [
            ("audio_counter_flags", 16),
            ("audio_exception_hook_pointer", 4),
            ("audio_exception_stack_pointer", 4),
            ("audio_exception_default_hook", 48),
        ] {
            let address = self.symbol(name)?;
            if address & 3 != 0
                || address < base
                || address.checked_add(bytes).is_none_or(|a| a > end)
            {
                return Err(
                    format!("owned driver: data symbol {name} is outside its image").into(),
                );
            }
        }
        Ok(())
    }

    fn data(&self, name: &str, bytes: u32) -> Result<u32> {
        let address = self.symbol(name)?;
        let end = self.symbol("audio_kernel_bss_end")?;
        if address < self.symbol("audio_kernel_image")?
            || address.checked_add(bytes).is_none_or(|limit| limit > end)
        {
            return Err(format!("owned driver: audio data {name} exceeds its allocation").into());
        }
        Ok(address)
    }

    pub(crate) fn audio(&self) -> Result<Audio> {
        Ok(Audio {
            initialize: self.entry("audio_sound_initialize")?,
            layout: self.entry("audio_loader_initialize_layout")?,
            slot_table: self.data("audio_loader_slots", 140)?,
            header_capacities: self.data("audio_loader_header_capacities", 84)?,
            sequence_capacities: self.data("audio_loader_sequence_capacities", 84)?,
            open_head: self.entry("audio_bank_open_head_sticky")?,
            transfer: self.entry("audio_bank_transfer_partial")?,
            completed: self.entry("audio_bank_transfer_completed")?,
            sequence_open: self.entry("audio_sequence_open")?,
            callbacks: self.entry("audio_music_start_callbacks")?,
            sequence_handles: self.data("audio_sequence_handles", 8)?,
            play: self.entry("audio_sequence_play")?,
            stop: self.entry("audio_sequence_stop_record")?,
            idle: self.entry("audio_video_get_mode")?,
            key_on: self.entry("audio_effects_key_on")?,
            key_off: self.entry("audio_effects_key_off")?,
            cue_dispatch: self.entry("audio_cue_dispatch")?,
            cue_refresh: self.entry("audio_effects_refresh_status")?,
            auxiliary_end: self.data("audio_sound_gains", 12)?,
            cue_gate: self.data("audio_cue_dispatch_prefix", 32)? + 2,
            cue_work: self.data("audio_cue_work", 264)?,
            voice_status: self.data("audio_sequence_voice_globals", 1616)? + 0x5f0,
            cue_lookup: self.data("audio_cue_lookup", 256)?,
            tone_read: self.entry("audio_effects_tone_read")?,
            loop_start: self.entry("audio_sequence_loop_start_store")?,
            infinite_loop: self.entry("audio_sequence_infinite_loop_store")?,
            sequence_end: self.entry("audio_sequence_end_event")?,
        })
    }

    pub(crate) fn ram(&self) -> Result<Ram> {
        let mut ram = Ram::default();
        let base = self.symbol("audio_kernel_image")?;
        for (offset, &byte) in self.bytes.iter().enumerate() {
            ram.write(base + offset as u32, Width::Byte, u32::from(byte))?;
        }
        Ok(ram)
    }

    /// Initialize services in zeroed RAM. Audio initialization remains an
    /// explicit subsequent guest call, before layout selection and bank upload.
    pub fn start(&self, observer: &mut impl FnMut(&Execution) -> Result<()>) -> Result<Started> {
        let mut bus = Interconnect::from_ram(self.ram()?);
        let base = self.symbol("audio_kernel_image")?;
        bus.write(0x1f80_1014, Width::Word, 0x2009_31e1)?;
        let mut kernel = Kernel::default();
        kernel.configure_console(64 * 1024)?;
        kernel.configure_counter_flags(self.symbol("audio_counter_flags")?)?;
        kernel.configure_cd_removal(self.entry("audio_cd_remove")?)?;
        kernel.configure_event_delivery(Service {
            entry: self.entry("audio_event_deliver")?,
            scan: self.entry("audio_event_loop")?,
            mode: self.entry("audio_event_mode")?,
            callback: self.entry("audio_event_callback")?,
            return_jump: self.entry("audio_event_return")?,
        })?;
        kernel.configure_exception_dispatch(Dispatch {
            entry: self.entry("audio_exception_vector")?,
            priority: self.entry("audio_exception_priority")?,
            node: self.entry("audio_exception_node")?,
            verifier: self.entry("audio_exception_verifier")?,
            handler: self.entry("audio_exception_handler")?,
            exit: self.entry("audio_exception_hook")?,
            stack_pointer: self.symbol("audio_exception_stack_pointer")?,
        })?;
        kernel.configure_syscall_verifier(self.entry("audio_syscall_handler")?)?;
        kernel.configure_exception_hook(
            self.entry("audio_exception_hook")?,
            self.symbol("audio_exception_hook_pointer")?,
        )?;
        kernel.configure_exception_return(
            self.entry("audio_exception_return")?,
            self.entry("audio_exception_return_rfe")?,
        )?;
        kernel.configure_exception_reset(self.entry("audio_exception_reset")?)?;
        let initialize = self.entry("audio_kernel_initialize")?;
        let mut cpu = Cpu::new(initialize);
        cpu.set_register(29, self.symbol("audio_initialization_stack_end")?);
        let mut execution = Execution::with_kernel(
            cpu,
            bus,
            spu_clock::Model::EmulatorReference,
            kernel,
            Environment {
                stack_top: self.symbol("audio_exception_stack_end")?,
                global_pointer: 0,
                missing_hook: MissingHook::ReturnFromException,
            },
        )?;
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
        let initialization_limit =
            u64::from(self.symbol("audio_kernel_state_end")? - self.symbol("audio_kernel_bss")?)
                + 1_000
                + 157 * 5
                + 6;
        let initialization =
            execution.call_observed(initialize, [0; 4], initialization_limit, observer)?;
        if initialization.result != 0 {
            return Err(
                "owned driver: service initialization returned an unexpected result".into(),
            );
        }
        let scheduler = self.symbol("audio_sequence_scheduler")?;
        let load = self.symbol("audio_sequence_scheduler_load")?;
        let payload = &self.bytes[(load - base) as usize..];
        let at = ram_offset(scheduler, payload.len())?;
        if &execution.bus.ram().bytes()[at..at + payload.len()] != payload {
            return Err(
                "owned driver: scheduler installation differs from its source bytes".into(),
            );
        }
        // Disable automatic PAD acknowledgement before enabling interrupts.
        execution.cpu.set_register(9, 0x5b);
        execution.call(0xb0, [0; 4], 1)?;
        execution.cpu.set_register(28, 0);
        execution.cpu.set_register(29, FOREGROUND_STACK);
        execution
            .cpu
            .cop0_mut()
            .write(12, self.evidence.initial_status)?;
        Ok(Started {
            execution,
            initialization,
            evidence: self.evidence.clone(),
        })
    }
}
