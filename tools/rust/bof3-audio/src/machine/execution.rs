//! Bounded guest calls with owned services or caller-loaded BIOS code and an
//! explicit instruction clock. Original-ROM references preserve interpreter costs
//! and NTSC VBlank phase. Attached CD FIFO callbacks run after delay slots;
//! mechanical/media timing and graphics processing remain unmodeled.
//! PCM output uses a separate explicit clock.
mod calls;
mod cooperative;
mod native;
use super::{
    boot,
    cpu::clock,
    cpu::{Cpu, FaultKind, Step},
    exception_calls::Environment,
    image::executable::ram_offset,
    interconnect::{DeviceProgress, Interconnect},
    kernel::Kernel,
    output_clock,
};
use crate::Result;
pub(crate) use calls::Guest as GuestCalls;
pub use cooperative::Clock as CooperativeClock;
pub use native::{ConsumerClock, NativeProducerProgress};
use serde::Serialize;

const RETURN: u32 = 0x8000_1000;

#[derive(Debug, Serialize)]
pub struct Call {
    pub entry: u32,
    pub arguments: [u32; 4],
    #[serde(skip_serializing_if = "Option::is_none")]
    pub stack_arguments: Option<[u32; 4]>,
    pub instructions: u64,
    pub syscalls: u64,
    pub interrupts: u64,
    pub result: u32,
}

pub struct Execution {
    pub cpu: Cpu,
    pub bus: Interconnect,
    pub clock: clock::Clock,
    syscalls: u64,
    interrupts: u64,
    output: Option<output_clock::Clock>,
    reference: Option<boot::timing::Clock>,
    instruction: Option<boot::timing::Clock>,
    kernel: Option<Services>,
    native: Option<native::Audio>,
    cooperative: Option<cooperative::Schedule>,
    calls: Option<GuestCalls>,
}

#[derive(Serialize)]
#[serde(untagged)]
pub enum AudioClock<'a> {
    Functional(&'a output_clock::Clock),
    Cooperative(&'a CooperativeClock),
}

struct Services {
    kernel: Kernel,
    environment: Environment,
    failed: bool,
}

enum Transition {
    Stopped,
    Advanced(StepOutcome),
}

/// A retired instruction is reported once, including when its device pass waits.
#[must_use]
pub struct StepOutcome {
    pub retired: Option<Step>,
    pub devices: DeviceProgress,
}

impl Execution {
    /// Execute caller-loaded code without host kernel services.
    pub fn new(cpu: Cpu, bus: Interconnect) -> Self {
        Self {
            cpu,
            bus,
            clock: clock::Clock::default(),
            syscalls: 0,
            interrupts: 0,
            output: None,
            reference: None,
            instruction: None,
            kernel: None,
            native: None,
            cooperative: None,
            calls: None,
        }
    }

    /// Execute caller-loaded owned services. Startup owns their code, state,
    /// stacks and device inputs; this does not load an image or boot firmware.
    /// Instructions use interpreter costs; host services retain two functional
    /// ticks per dispatch, without a native latency claim.
    /// Failures retain state and prevent subsequent steps or host calls.
    pub fn with_kernel(
        cpu: Cpu,
        bus: Interconnect,
        kernel: Kernel,
        environment: Environment,
    ) -> Result<Self> {
        kernel.require_plain_context()?;
        let mut execution = Self::new(cpu, bus);
        execution.instruction = Some(boot::timing::Clock::default());
        execution.kernel = Some(Services {
            kernel,
            environment,
            failed: false,
        });
        Ok(execution)
    }

    pub fn kernel(&self) -> Option<&Kernel> {
        self.kernel.as_ref().map(|services| &services.kernel)
    }

    pub(crate) fn configure_guest_calls(&mut self, calls: GuestCalls) -> Result<()> {
        if self.kernel.is_none()
            || self.calls.is_some()
            || self.cpu.instructions() != 0
            || self.cpu.pc() != calls.reset
        {
            return Err("owned call dispatcher requires untouched cold service execution".into());
        }
        if calls.arguments != super::inputs::COMMANDS || self.bus.inputs.is_some() {
            return Err("owned call dispatcher requires its read-only input ABI".into());
        }
        self.bus.inputs = Some(super::inputs::Inputs::default());
        self.calls = Some(calls);
        Ok(())
    }

    fn stage_media_input(&mut self, bytes: &[u8]) -> Result<()> {
        if self
            .calls
            .is_none_or(|calls| self.cpu.pc() != calls.returned)
        {
            return Err("owned media admission requires the dispatcher return boundary".into());
        }
        self.bus
            .inputs
            .as_mut()
            .ok_or("owned input ABI is not configured")?
            .media(bytes)
    }

    /// Preserve the original BIOS interpreter's absolute NTSC phase for
    /// development reference calls. This does not enable the PCM output clock.
    pub fn from_boot(machine: boot::Machine) -> Self {
        let mut execution = Self::new(machine.cpu, machine.bus);
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
        settings: crate::machine::spu::readback::Settings,
    ) -> Result<()> {
        if self.reference.is_none() {
            return Err("reference SPU observations require original-BIOS timing".into());
        }
        self.bus.configure_spu_readback(settings)
    }

    pub fn syscalls(&self) -> u64 {
        self.syscalls
    }

    pub fn step(&mut self) -> Result<StepOutcome> {
        self.step_retired(&mut |_| Ok(()))
    }

    /// Return the actual fetched instruction after CPU and scheduled device
    /// service complete. The observer runs at the selected pre-execution
    /// boundary, so it can retain address registers for retirement evidence.
    /// Syscalls and host service transitions have no retired instruction.
    /// Audio waits report no further instruction until released. Resume a CD
    /// wait with resume_cd_audio. A native CPU goal requires consumer advancement.
    /// Errors retain advanced state; owned execution cannot resume.
    pub fn step_retired(
        &mut self,
        observer: &mut impl FnMut(&Self) -> Result<()>,
    ) -> Result<StepOutcome> {
        match self.transition(observer, None)? {
            Transition::Stopped => unreachable!("a step has no stop address"),
            Transition::Advanced(step) => Ok(step),
        }
    }

    /// Finish a suspended device pass without CPU, timer or output-clock work.
    pub fn resume_cd_audio(
        &mut self,
        outcome: crate::machine::cd::queue::WaitOutcome,
    ) -> Result<DeviceProgress> {
        self.require_execution()?;
        let result = self.bus.resume_cd_audio(outcome);
        self.retain_failure(&result);
        result
    }

    fn require_execution(&self) -> Result<()> {
        if self.kernel.as_ref().is_some_and(|services| services.failed) {
            return Err("owned execution is unfinished after a previous failure".into());
        }
        Ok(())
    }

    fn retain_failure<T>(&mut self, result: &Result<T>) {
        if result.is_err() {
            if let Some(services) = &mut self.kernel {
                services.failed = true;
            }
        }
    }

    fn transition(
        &mut self,
        observer: &mut impl FnMut(&Self) -> Result<()>,
        stop: Option<u32>,
    ) -> Result<Transition> {
        self.require_execution()?;
        let result = self.advance(observer, stop).and_then(|mut transition| {
            if let Transition::Advanced(step) = &mut transition {
                if let Some(mut schedule) = self.cooperative.take() {
                    let result = schedule.advance(self, step.devices);
                    self.cooperative = Some(schedule);
                    result?;
                    step.devices = DeviceProgress::Ready;
                }
            }
            Ok(transition)
        });
        self.retain_failure(&result);
        result
    }

    fn advance(
        &mut self,
        observer: &mut impl FnMut(&Self) -> Result<()>,
        stop: Option<u32>,
    ) -> Result<Transition> {
        if self
            .native
            .as_ref()
            .is_some_and(native::Audio::has_continuation)
        {
            return self.resume_native();
        }
        if self.bus.device_progress() == DeviceProgress::WaitingForCdAudio {
            return Ok(Transition::Advanced(StepOutcome {
                retired: None,
                devices: DeviceProgress::WaitingForCdAudio,
            }));
        }
        if stop == Some(self.cpu.pc())
            && self
                .kernel
                .as_ref()
                .is_none_or(|services| services.kernel.require_plain_context().is_ok())
        {
            return Ok(Transition::Stopped);
        }
        let interrupt = match self
            .reference
            .as_ref()
            .or(self.instruction.as_ref().filter(|_| self.native.is_some()))
        {
            Some(clock) => clock.admit_interrupt(&mut self.cpu, &self.bus),
            None => self.cpu.take_interrupt(self.bus.interrupts().pending()),
        };
        if interrupt {
            self.interrupts += 1;
            if let Some(services) = &mut self.kernel {
                services.kernel.begin_exception(
                    &mut self.cpu,
                    &mut self.bus,
                    services.environment,
                )?;
            }
        }
        if let Some(services) = &mut self.kernel {
            services
                .kernel
                .prepare_exception_step(&mut self.cpu, &mut self.bus)?;
        }
        if stop == Some(self.cpu.pc()) {
            if let Some(services) = &self.kernel {
                services.kernel.require_plain_context()?;
            }
            return Ok(Transition::Stopped);
        }
        observer(self)?;
        let before = self
            .reference
            .as_ref()
            .or(self.instruction.as_ref())
            .map(boot::timing::Clock::cycles);
        let mut waiting = false;
        let host_call = Kernel::is_call(self.cpu.pc())
            && !self
                .kernel
                .as_ref()
                .is_some_and(|services| services.kernel.executes_guest(&self.cpu));
        let retired = if let Some(services) = self.kernel.as_mut().filter(|_| host_call) {
            if self.cooperative.is_some() {
                return Err(format!(
                    "cooperative audio does not admit functional host service {:#04x}/{:#04x}",
                    self.cpu.pc() & 0x1fff_ffff,
                    self.cpu.register(9),
                )
                .into());
            }
            waiting = services
                .kernel
                .dispatch(&mut self.cpu, &mut self.bus)?
                .waiting;
            if let Some(clock) = &mut self.instruction {
                clock.functional_service();
            }
            None
        } else {
            let step = if self.native.is_some() {
                let Some(step) = self.prepare_native_instruction(before.unwrap())? else {
                    return Ok(Transition::Advanced(StepOutcome {
                        retired: None,
                        devices: DeviceProgress::WaitingForSpu,
                    }));
                };
                step
            } else {
                match &mut self.reference {
                    Some(clock) => clock.step(&mut self.cpu, &mut self.bus),
                    None => match &mut self.instruction {
                        Some(clock) => clock.step_instruction(&mut self.cpu, &mut self.bus),
                        None => self.cpu.step(&mut self.bus),
                    },
                }
            };
            self.accept_instruction(step)?
        };
        self.finish_transition(before, retired, waiting)
    }

    fn accept_instruction(
        &mut self,
        step: std::result::Result<Step, super::cpu::Fault>,
    ) -> Result<Option<Step>> {
        Ok(match step {
            Ok(step) => {
                if let Some(services) = &mut self.kernel {
                    services.kernel.complete_exception_step(&self.cpu, &step)?;
                }
                Some(step)
            }
            Err(fault) if fault.kind == FaultKind::Syscall => {
                self.cpu.enter_exception(&fault)?;
                self.syscalls += 1;
                if let Some(services) = &mut self.kernel {
                    services.kernel.begin_exception(
                        &mut self.cpu,
                        &mut self.bus,
                        services.environment,
                    )?;
                }
                None
            }
            Err(fault) => return Err(fault.into()),
        })
    }

    fn finish_transition(
        &mut self,
        before: Option<u64>,
        retired: Option<Step>,
        waiting: bool,
    ) -> Result<Transition> {
        let branch_boundary = retired.as_ref().is_some_and(|step| step.in_delay_slot);
        let ticks = before.map_or(2, |before| {
            (self
                .reference
                .as_ref()
                .or(self.instruction.as_ref())
                .unwrap()
                .cycles()
                - before) as u32
        });
        // Raw-state execution retains its explicit base BIAS schedule.
        if let Some(output) = &mut self.output {
            output.advance(&mut self.bus, &mut self.clock, ticks)?;
        } else {
            self.clock.advance(ticks)?;
            if self.reference.is_none() && self.native.is_none() {
                self.bus.advance_cpu(ticks)?;
            }
        }
        if self.native.is_some() && self.native_branch_wait(branch_boundary)? {
            return Ok(Transition::Advanced(StepOutcome {
                retired,
                devices: DeviceProgress::WaitingForSpu,
            }));
        }
        if self.native.is_some() {
            self.bus
                .advance_native_time(self.instruction.as_ref().unwrap().cycles(), branch_boundary);
        }
        let devices = self.bus.advance_devices(
            self.reference_cycles().unwrap_or(self.clock.ticks()),
            branch_boundary,
        )?;
        if waiting && (self.reference.is_some() || self.native.is_some()) {
            return Err("native counter timing does not implement owned WaitEvent polling".into());
        }
        Ok(Transition::Advanced(StepOutcome { retired, devices }))
    }

    /// Activate an explicit output/scanline phase at this guest boundary.
    /// Existing SPU arithmetic models must already be configured on the bus.
    pub fn enable_output(
        &mut self,
        model: output_clock::Model,
        collection: output_clock::Collection,
    ) -> Result<()> {
        if self.reference.is_some() {
            return Err("original-BIOS output requires an explicit reference sample phase".into());
        }
        if self.native.is_some() {
            return Err("native audio already owns output timing".into());
        }
        if self.output.is_some() {
            return Err("output clock already enabled".into());
        }
        self.bus.set_vblank(false);
        self.output = Some(output_clock::Clock::new(model, collection));
        Ok(())
    }
    /// Activate functional PCM alongside original-BIOS timing. The caller must
    /// supply 1..=768 ticks until the first sample; no native phase is inferred.
    /// Instruction accesses precede elapsed sample work, then branch-dispatched
    /// CD service runs. A feed admitted there reaches subsequent sample work.
    /// Original counters retain their phase and own all timers and blanking.
    pub fn enable_reference_output(&mut self, first_sample_ticks: u32) -> Result<()> {
        let cycle = self
            .reference_cycles()
            .ok_or("reference output requires original-BIOS timing")?;
        if self.output.is_some() {
            return Err("output clock already enabled".into());
        }
        self.output = Some(output_clock::Clock::for_reference(
            cycle,
            first_sample_ticks,
        )?);
        Ok(())
    }
    pub fn output(&self) -> Option<&output_clock::Clock> {
        self.output.as_ref()
    }

    pub fn audio_frames(&self) -> Result<u64> {
        if self.native.is_some() {
            return self.native_delivered_frames();
        }
        Ok(self
            .output
            .as_ref()
            .ok_or("output clock is not enabled")?
            .frames())
    }

    pub fn audio_clock(&self) -> Result<AudioClock<'_>> {
        if let Some(schedule) = &self.cooperative {
            return Ok(AudioClock::Cooperative(&schedule.clock));
        }
        Ok(AudioClock::Functional(
            self.output.as_ref().ok_or("output clock is not enabled")?,
        ))
    }
    pub fn begin_audio_capture(&mut self) -> Result<u64> {
        if self.native.is_some() {
            return self.begin_native_capture();
        }
        self.output
            .as_mut()
            .ok_or("output clock is not enabled")?
            .begin_capture()
    }
    pub fn take_audio_frames(&mut self) -> Result<Vec<[f32; 2]>> {
        if self.native.is_some() {
            return self.take_native_frames();
        }
        Ok(self
            .output
            .as_mut()
            .ok_or("output clock is not enabled")?
            .take_frames())
    }

    /// Owned images continue through their guest dispatcher. Caller-loaded
    /// development contexts use explicit host entry with RETURN as a stop only.
    /// The limit counts transitions, including BIOS calls and blocked wait polls.
    /// Errors may leave guest execution advanced; callers must not retry calls.
    /// The cooperative context resolves its audio waits. Manual native contexts
    /// require call_scheduled or stepped execution to resolve audio waits.
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
        self.call_scheduled(entry, arguments, limit, observer, &mut |_, progress| {
            progress.require_ready()
        })
    }

    /// Call an eight-word o32 entry. Owned dispatchers reserve, populate and
    /// restore the outgoing frame through guest instructions. Development host
    /// entry preserves its stack bridge, including restoration after errors.
    pub fn call_o32_observed(
        &mut self,
        entry: u32,
        arguments: [u32; 8],
        limit: u64,
        observer: &mut impl FnMut(&Self) -> Result<()>,
    ) -> Result<Call> {
        self.call_arguments_scheduled(
            entry,
            arguments[..4].try_into().unwrap(),
            Some(arguments[4..].try_into().unwrap()),
            limit,
            observer,
            &mut |_, progress| progress.require_ready(),
        )
    }

    /// Execute a bounded call with an explicit host-audio scheduler. It runs
    /// after every advanced transition, including Ready and the return to the
    /// stop address. It may advance producer/consumer work and release waits;
    /// it must preserve the retained CPU, clock and cache configuration.
    /// Each transition, including a wait poll, counts toward the supplied limit.
    /// Scheduler errors retain advanced state and terminate the call.
    pub fn call_scheduled(
        &mut self,
        entry: u32,
        arguments: [u32; 4],
        limit: u64,
        observer: &mut impl FnMut(&Self) -> Result<()>,
        schedule: &mut impl FnMut(&mut Self, DeviceProgress) -> Result<()>,
    ) -> Result<Call> {
        self.call_arguments_scheduled(entry, arguments, None, limit, observer, schedule)
    }

    fn call_arguments_scheduled(
        &mut self,
        entry: u32,
        arguments: [u32; 4],
        stack_arguments: Option<[u32; 4]>,
        limit: u64,
        observer: &mut impl FnMut(&Self) -> Result<()>,
        schedule: &mut impl FnMut(&mut Self, DeviceProgress) -> Result<()>,
    ) -> Result<Call> {
        if !self.calls.is_some_and(|calls| {
            entry == calls.reset && self.cpu.pc() == entry && self.cpu.instructions() == 0
        }) {
            ram_offset(entry, 4)?;
        }
        if entry & 3 != 0 || entry == RETURN || !(1..=10_000_000).contains(&limit) {
            return Err("guest call requires aligned RAM entry and limit 1..=10000000".into());
        }
        if let Some(services) = &self.kernel {
            if services.failed {
                return Err("owned execution is unfinished after a previous failure".into());
            }
            services.kernel.require_plain_context()?;
        }
        self.bus.device_progress().require_ready()?;
        if self
            .native
            .as_ref()
            .is_some_and(native::Audio::has_continuation)
        {
            return Err("finish the native CPU audio wait before entering a guest call".into());
        }
        let result = self.run_call(entry, arguments, stack_arguments, limit, observer, schedule);
        if result.is_err() {
            if let Some(services) = &mut self.kernel {
                services.failed = true;
            }
        }
        result
    }

    fn run_call(
        &mut self,
        entry: u32,
        arguments: [u32; 4],
        stack_arguments: Option<[u32; 4]>,
        limit: u64,
        observer: &mut impl FnMut(&Self) -> Result<()>,
        schedule: &mut impl FnMut(&mut Self, DeviceProgress) -> Result<()>,
    ) -> Result<Call> {
        let (instructions, syscalls, interrupts) =
            (self.cpu.instructions(), self.syscalls, self.interrupts);
        let mut boundary = calls::prepare(self, entry, arguments, stack_arguments)?;
        let result = (|| {
            for _ in 0..limit {
                match self
                    .transition(observer, (!boundary.resuming).then_some(boundary.returned))?
                {
                    Transition::Stopped => break,
                    Transition::Advanced(step) => {
                        if step
                            .retired
                            .as_ref()
                            .is_some_and(|step| step.pc == boundary.returned)
                        {
                            boundary.resuming = false;
                        }
                        schedule(self, step.devices)?;
                    }
                }
            }
            if boundary.resuming || self.cpu.pc() != boundary.returned {
                return Err(format!(
                    "guest call {entry:#010x} transition safety limit at {:#010x}",
                    self.cpu.pc()
                )
                .into());
            }
            self.bus.device_progress().require_ready()?;
            if self
                .native
                .as_ref()
                .is_some_and(native::Audio::has_continuation)
            {
                return Err(
                    "guest call transition safety limit during native CPU audio wait".into(),
                );
            }
            if let Some(services) = &self.kernel {
                services.kernel.require_plain_context()?;
            }
            boundary.require_stack(self)?;
            Ok(Call {
                entry,
                arguments,
                stack_arguments,
                instructions: self.cpu.instructions() - instructions,
                syscalls: self.syscalls - syscalls,
                interrupts: self.interrupts - interrupts,
                result: self.cpu.register(2),
            })
        })();
        boundary.restore_host_stack(self);
        result
    }
}
