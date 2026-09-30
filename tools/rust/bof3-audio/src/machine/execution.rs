//! Bounded guest calls with BIOS exceptions, IRQ delivery and explicit base
//! instruction-clock service. Output/VBlank clocks are opt-in; CPU stalls and
//! complete GPU/CD timing remain unmodeled.
use super::{
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
        }
    }

    pub fn step(&mut self) -> Result<()> {
        self.step_observed(&mut |_| Ok(()))
    }

    /// Observe the instruction selected after IRQ admission, before execution.
    /// The observer cannot mutate guest state. Its failure aborts this run.
    pub fn step_observed(&mut self, observer: &mut impl FnMut(&Self) -> Result<()>) -> Result<()> {
        if self.cpu.take_interrupt(self.bus.interrupts().pending()) {
            self.interrupts += 1;
        }
        observer(self)?;
        match self.cpu.step(&mut self.bus) {
            Ok(_) => (),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                self.cpu.enter_exception(&fault)?;
                self.syscalls += 1;
            }
            Err(fault) => return Err(fault.into()),
        }
        // PCSX-Redux interpreter base BIAS, not a hardware instruction cost.
        if let Some(output) = &mut self.output {
            output.advance(&mut self.bus, &mut self.clock, 2)?;
        } else {
            self.clock.advance(&mut self.bus, 2)?;
            self.bus.advance_cpu(2)?;
        }
        Ok(())
    }

    /// Activate an explicit output/scanline phase at this guest boundary.
    /// Existing SPU arithmetic models must already be configured on the bus.
    pub fn enable_output(&mut self, model: output_clock::Model) -> Result<()> {
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
