//! Bounded guest calls with BIOS exceptions, IRQ delivery and explicit base
//! instruction-clock service. Original-ROM references preserve interpreter costs
//! and NTSC VBlank phase. Complete GPU/CD and reference root-counter timing remain
//! unmodeled; PCM output uses a separate explicit clock.
use super::{
    boot,
    cpu::{Cpu, FaultKind},
    executable::ram_offset,
    interconnect::Interconnect,
    output_clock, spu_clock,
};
use crate::Result;
use serde::Serialize;

const RETURN: u32 = 0x8000_1000;

#[derive(Debug, Serialize)]
pub struct Call {
    pub entry: u32,
    pub arguments: [u32; 4],
    pub instructions: u64,
    pub syscalls: u64,
    pub interrupts: u64,
    pub result: u32,
}

pub struct Execution {
    pub cpu: Cpu,
    pub bus: Interconnect,
    pub clock: spu_clock::Clock,
    syscalls: u64,
    interrupts: u64,
    output: Option<output_clock::Clock>,
    reference: Option<boot::timing::Clock>,
}

impl Execution {
    pub fn new(cpu: Cpu, bus: Interconnect, model: spu_clock::Model) -> Self {
        Self {
            cpu,
            bus,
            clock: spu_clock::Clock::new(model),
            syscalls: 0,
            interrupts: 0,
            output: None,
            reference: None,
        }
    }

    /// Preserve the original BIOS interpreter's absolute NTSC phase for
    /// development reference calls. This does not enable the PCM output clock.
    pub fn from_boot(machine: boot::Machine, model: spu_clock::Model) -> Self {
        let mut execution = Self::new(machine.cpu, machine.bus, model);
        execution.reference = Some(machine.timing);
        execution
    }

    pub fn reference_cycles(&self) -> Option<u64> {
        self.reference.as_ref().map(boot::timing::Clock::cycles)
    }

    /// Configure CPU-cycle ENVX observations before guest SPU initialization.
    /// Settings come from the retained native capture; no PCM clock is enabled.
    pub fn configure_reference_spu(
        &mut self,
        settings: super::spu_readback::Settings,
    ) -> Result<()> {
        if self.reference.is_none() {
            return Err("reference SPU observations require original-BIOS timing".into());
        }
        self.bus.configure_spu_readback(settings)
    }

    pub fn syscalls(&self) -> u64 {
        self.syscalls
    }

    pub fn step(&mut self) -> Result<()> {
        self.step_observed(&mut |_| Ok(()))
    }

    /// Observe the instruction selected after IRQ admission, before execution.
    /// The observer cannot mutate guest state. Its failure aborts this run.
    pub fn step_observed(&mut self, observer: &mut impl FnMut(&Self) -> Result<()>) -> Result<()> {
        let interrupt = match &self.reference {
            Some(clock) => clock.admit_interrupt(&mut self.cpu, &self.bus),
            None => self.cpu.take_interrupt(self.bus.interrupts().pending()),
        };
        if interrupt {
            self.interrupts += 1;
        }
        observer(self)?;
        let before = self.reference_cycles();
        let step = match &mut self.reference {
            Some(clock) => clock.step(&mut self.cpu, &mut self.bus),
            None => self.cpu.step(&mut self.bus),
        };
        match step {
            Ok(_) => (),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                self.cpu.enter_exception(&fault)?;
                self.syscalls += 1;
            }
            Err(fault) => return Err(fault.into()),
        }
        let ticks = before.map_or(2, |before| {
            (self.reference_cycles().unwrap() - before) as u32
        });
        // Raw-state execution retains its explicit base BIAS schedule.
        if let Some(output) = &mut self.output {
            output.advance(&mut self.bus, &mut self.clock, ticks)?;
        } else {
            self.clock.advance(&mut self.bus, ticks)?;
            if self.reference.is_none() {
                self.bus.advance_cpu(ticks)?;
            }
        }
        Ok(())
    }

    /// Activate an explicit output/scanline phase at this guest boundary.
    /// Existing SPU arithmetic models must already be configured on the bus.
    pub fn enable_output(&mut self, model: output_clock::Model) -> Result<()> {
        if self.reference.is_some() {
            return Err(
                "original-BIOS reference timing does not implement PCM output scheduling".into(),
            );
        }
        if self.output.is_some() {
            return Err("output clock already enabled".into());
        }
        self.bus.set_vblank(false);
        self.output = Some(output_clock::Clock::new(model));
        Ok(())
    }
    pub fn output(&self) -> Option<&output_clock::Clock> {
        self.output.as_ref()
    }
    pub fn take_audio_frames(&mut self) -> Result<Vec<[i16; 2]>> {
        Ok(self
            .output
            .as_mut()
            .ok_or("output clock is not enabled")?
            .take_frames())
    }

    /// Host-directed routine entry using current CPU/kernel/device state.
    /// RETURN is a stop address only; no instruction or trampoline is written.
    /// Errors may leave guest execution advanced; callers must not retry calls.
    pub fn call(&mut self, entry: u32, arguments: [u32; 4], limit: u64) -> Result<Call> {
        self.call_observed(entry, arguments, limit, &mut |_| Ok(()))
    }

    pub fn call_observed(
        &mut self,
        entry: u32,
        arguments: [u32; 4],
        limit: u64,
        observer: &mut impl FnMut(&Self) -> Result<()>,
    ) -> Result<Call> {
        ram_offset(entry, 4)?;
        if entry & 3 != 0 || entry == RETURN || !(1..=10_000_000).contains(&limit) {
            return Err("guest call requires aligned RAM entry and limit 1..=10000000".into());
        }
        let (instructions, syscalls, interrupts) =
            (self.cpu.instructions(), self.syscalls, self.interrupts);
        self.cpu.resume_at(entry);
        for (i, value) in arguments.into_iter().enumerate() {
            self.cpu.set_register(4 + i, value);
        }
        self.cpu.set_register(31, RETURN);
        for _ in 0..limit {
            if self.cpu.pc() == RETURN {
                break;
            }
            self.step_observed(observer)?;
        }
        if self.cpu.pc() != RETURN {
            return Err(format!(
                "guest call {entry:#010x} instruction safety limit at {:#010x}",
                self.cpu.pc()
            )
            .into());
        }
        Ok(Call {
            entry,
            arguments,
            instructions: self.cpu.instructions() - instructions,
            syscalls: self.syscalls - syscalls,
            interrupts: self.interrupts - interrupts,
            result: self.cpu.register(2),
        })
    }
}
