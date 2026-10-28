//! Narrow BIOS service model used by the original audio bootstrap. This executes
//! kernel service semantics, never substitutes game or libsnd functions.
//! https://psx-spx.consoledev.net/kernelbios/

use super::{
    bus::{Bus, Width},
    cpu::{Cpu, Fault, FaultKind},
    event_calls::{Calls as EventCalls, CALLBACK_RETURN},
    events::{Table as Events, BUSY},
    exception_calls::{Calls as ExceptionCalls, Environment, MissingHook, HANDLER_RETURN},
    exception_chains::Table as ExceptionChains,
    executable::ram_offset,
};
use crate::Result;

const SAVED: [usize; 12] = [31, 29, 30, 16, 17, 18, 19, 20, 21, 22, 23, 28];

#[derive(Clone, Debug)]
pub struct Kernel {
    entry_hook: Option<u32>,
    pad_auto_ack: u32,
    counter_auto_ack: [u32; 4],
    counter_handlers: Option<[[u32; 2]; 4]>,
    generic_handler: Option<(u32, u32)>,
    syscall_handler: Option<u32>,
    cd_removal: Option<u32>,
    exception_delivery: Option<ExceptionDelivery>,
    waiting_event: Option<PendingWait>,
    event_calls: EventCalls,
    exception_calls: Option<ExceptionCalls>,
}

#[derive(Clone, Debug)]
struct PendingWait {
    handle: u32,
    address: u32,
    return_pc: u32,
    callback_depth: usize,
}

impl Default for Kernel {
    fn default() -> Self {
        Self {
            entry_hook: None,
            pad_auto_ack: 1,
            counter_auto_ack: [1; 4],
            counter_handlers: None,
            generic_handler: None,
            syscall_handler: None,
            cd_removal: None,
            exception_delivery: None,
            waiting_event: None,
            event_calls: EventCalls::default(),
            exception_calls: None,
        }
    }
}

#[derive(Debug)]
pub struct Call {
    pub vector: u32,
    pub service: u32,
    pub name: &'static str,
    /// A blocked WaitEvent retains the CPU at its BIOS entry for the scheduler.
    pub waiting: bool,
    /// The CPU now points at this guest handler; the BIOS call is not finished.
    pub callback: Option<u32>,
}

#[derive(Clone, Copy, Debug)]
enum ExceptionDelivery {
    Counter,
    Generic { next: usize },
    Syscall,
}

impl Kernel {
    /// Bind caller-owned service identifiers in guest ExCB nodes. Each pair is
    /// verifier/acknowledger for timer 0, timer 1, timer 2, then VBlank.
    /// These identifiers select owned semantics, not instructions to fetch.
    /// Startup owns their allocation and chain placement; no BIOS address is
    /// recognized automatically. Ordinary unbound targets still execute code.
    pub fn configure_counter_handlers(&mut self, handlers: [[u32; 2]; 4]) -> Result<()> {
        self.require_plain_context()?;
        if self.counter_handlers.is_some() {
            return Err("owned counter handlers already configured".into());
        }
        let mut offsets = Vec::new();
        for address in handlers.into_iter().flatten() {
            let offset = self.validate_owned_handler(address)?;
            if offsets.contains(&offset) {
                return Err("owned counter handlers require distinct RAM identifiers".into());
            }
            offsets.push(offset);
        }
        self.counter_handlers = Some(handlers);
        Ok(())
    }

    /// Bind an owned generic IRQ verifier and eleven live RAM acknowledgement
    /// flags, indexed by IRQ bit. Startup owns their allocation and ExCB placement.
    pub fn configure_generic_handler(&mut self, handler: u32, flags: u32) -> Result<()> {
        self.require_plain_context()?;
        if self.generic_handler.is_some() {
            return Err("owned generic IRQ handler already configured".into());
        }
        self.validate_owned_handler(handler)?;
        if flags & 3 != 0 {
            return Err("owned generic IRQ flags require aligned RAM".into());
        }
        ram_offset(flags, 11 * 4)?;
        self.generic_handler = Some((handler, flags));
        Ok(())
    }

    /// Bind the priority-zero IRQ/syscall handler to a caller-owned RAM identifier.
    /// Other exception causes retain an explicit unsupported failure policy.
    pub fn configure_syscall_handler(&mut self, handler: u32) -> Result<()> {
        self.require_plain_context()?;
        if self.syscall_handler.is_some() {
            return Err("owned syscall handler already configured".into());
        }
        self.validate_owned_handler(handler)?;
        self.syscall_handler = Some(handler);
        Ok(())
    }

    /// Bind authored guest A0/72 code. Startup loads its instructions and
    /// initializes its mutable handles and node pointers; no BIOS state is used.
    /// The guest routine performs its syscall through normal exception delivery.
    pub fn configure_cd_removal(&mut self, entry: u32) -> Result<()> {
        self.require_plain_context()?;
        if self.cd_removal.is_some() {
            return Err("owned CD removal entry already configured".into());
        }
        self.validate_owned_handler(entry)?;
        self.cd_removal = Some(entry);
        Ok(())
    }

    fn validate_owned_handler(&self, handler: u32) -> Result<usize> {
        super::exception_calls::validate_handler(handler)?;
        if Self::is_call(handler) {
            return Err("owned handler conflicts with a kernel service vector".into());
        }
        let offset = ram_offset(handler, 4)?;
        let occupied = self.counter_handlers.is_some_and(|handlers| {
            handlers
                .into_iter()
                .flatten()
                .any(|address| ram_offset(address, 4).ok() == Some(offset))
        }) || self
            .generic_handler
            .is_some_and(|(address, _)| ram_offset(address, 4).ok() == Some(offset))
            || self
                .syscall_handler
                .is_some_and(|address| ram_offset(address, 4).ok() == Some(offset))
            || self
                .cd_removal
                .is_some_and(|address| ram_offset(address, 4).ok() == Some(offset));
        if occupied {
            return Err("owned exception handlers require distinct RAM identifiers".into());
        }
        Ok(offset)
    }

    fn syscall_progress(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<Call>> {
        let context = super::thread_context::Context::current(bus)?.address();
        match bus.read(context + 0x98, Width::Word)? & 0x3c {
            0 => {
                cpu.set_register(2, 0);
                return Ok(None);
            }
            0x20 => {}
            _ => {
                return Err(
                    "owned syscall handler: other exception failure policy is unsupported".into(),
                )
            }
        }
        let pc = bus.read(context + 0x88, Width::Word)?;
        let service = bus.read(context + 0x18, Width::Word)?;
        bus.write(context + 0x88, Width::Word, pc.wrapping_add(4))?;
        match service {
            0 => {}
            1 => {
                let status = bus.read(context + 0x94, Width::Word)?;
                bus.write(
                    context + 0x10,
                    Width::Word,
                    u32::from(status & 0x404 == 0x404),
                )?;
                let status = bus.read(context + 0x94, Width::Word)?;
                bus.write(context + 0x94, Width::Word, status & !0x404)?;
            }
            2 => {
                let status = bus.read(context + 0x94, Width::Word)?;
                bus.write(context + 0x94, Width::Word, status | 0x404)?;
            }
            3 => {
                let target = bus.read(context + 0x1c, Width::Word)?;
                let pcb = bus.read(0x108, Width::Word)?;
                bus.write(pcb, Width::Word, target)?;
                bus.write(context + 0x10, Width::Word, 1)?;
            }
            _ => {
                cpu.set_register(4, 0xf000_0010);
                cpu.set_register(5, 0x4000);
                self.exception_delivery = Some(ExceptionDelivery::Syscall);
                if let Some(callback) = self.event_calls.start(cpu, bus)? {
                    return Ok(Some(Call {
                        vector: 0x80,
                        service: 6,
                        name: "SyscallExceptionEvent",
                        waiting: false,
                        callback: Some(callback),
                    }));
                }
            }
        }
        self.return_exception(cpu, bus)?;
        Ok(Some(Call {
            vector: 0x80,
            service: 6,
            name: "SyscallExceptionReturn",
            waiting: false,
            callback: None,
        }))
    }

    fn generic_progress(
        &mut self,
        cpu: &mut Cpu,
        bus: &mut impl Bus,
        next: usize,
    ) -> Result<Option<u32>> {
        const SOURCES: [(u32, u32); 11] = [
            (2, 3),
            (9, 9),
            (1, 2),
            (10, 10),
            (8, 11),
            (0, 1),
            (4, 5),
            (5, 6),
            (6, 6),
            (7, 8),
            (3, 4),
        ];
        for (index, &(bit, class)) in SOURCES.iter().enumerate().skip(next) {
            let mask = bus.read(0x1f80_1074, Width::Word)?;
            let status = bus.read(0x1f80_1070, Width::Word)?;
            if mask & status & (1 << bit) == 0 {
                continue;
            }
            cpu.set_register(4, 0xf000_0000 | class);
            cpu.set_register(5, 0x1000);
            self.exception_delivery = Some(ExceptionDelivery::Generic { next: index + 1 });
            let callback = self.event_calls.start(cpu, bus)?;
            if callback.is_some() {
                return Ok(callback);
            }
        }
        let flags = self.generic_handler.unwrap().1;
        let mut clear = 0;
        for bit in 0..11 {
            if bus.read(flags + bit * 4, Width::Word)? != 0 {
                clear |= 1 << bit;
            }
        }
        bus.write(0x1f80_1070, Width::Word, !clear)?;
        self.exception_delivery = None;
        cpu.set_register(2, 0);
        Ok(None)
    }

    fn counter_action(&self, target: u32) -> Option<(usize, bool)> {
        self.counter_handlers?
            .iter()
            .enumerate()
            .find_map(|(index, pair)| {
                pair.iter()
                    .position(|&address| address == target)
                    .map(|part| (index, part == 1))
            })
    }

    /// Functional ABI bridge after CPU exception entry. Stack/GP and the missing
    /// hook policy are explicit inputs, not claimed original BIOS boot contents.
    pub fn begin_exception(
        &mut self,
        cpu: &mut Cpu,
        bus: &mut impl Bus,
        environment: Environment,
    ) -> Result<Call> {
        self.require_plain_context()?;
        let calls = ExceptionCalls::prepare(bus, environment)?;
        let context = super::thread_context::Context::current(bus)?;
        context.save_exception(cpu, bus)?;
        cpu.set_register(28, environment.global_pointer);
        cpu.set_register(30, environment.stack_top);
        self.exception_calls = Some(calls);
        let callback = self.exception_calls.as_mut().unwrap().advance(cpu, bus)?;
        self.exception_progress(cpu, bus, callback)
    }

    pub fn pending_exception(&self) -> bool {
        self.exception_calls.is_some()
    }

    fn exception_progress(
        &mut self,
        cpu: &mut Cpu,
        bus: &mut impl Bus,
        mut callback: Option<u32>,
    ) -> Result<Call> {
        while let Some(target) = callback {
            if self.syscall_handler == Some(target) {
                if let Some(call) = self.syscall_progress(cpu, bus)? {
                    return Ok(call);
                }
                callback = self.exception_calls.as_mut().unwrap().resume(cpu, bus)?;
                continue;
            }
            if self
                .generic_handler
                .is_some_and(|(handler, _)| handler == target)
            {
                if let Some(event) = self.generic_progress(cpu, bus, 0)? {
                    return Ok(Call {
                        vector: 0x80,
                        service: 6,
                        name: "GenericInterruptEvent",
                        waiting: false,
                        callback: Some(event),
                    });
                }
                callback = self.exception_calls.as_mut().unwrap().resume(cpu, bus)?;
                continue;
            }
            let Some((index, acknowledge)) = self.counter_action(target) else {
                break;
            };
            let bit = 1u32 << [4, 5, 6, 0][index];
            if acknowledge {
                if self.counter_auto_ack[index] != 0 {
                    bus.write(0x1f80_1070, Width::Word, !bit)?;
                    self.return_exception(cpu, bus)?;
                    return Ok(Call {
                        vector: 0x80,
                        service: 6,
                        name: "CounterInterruptReturn",
                        waiting: false,
                        callback: None,
                    });
                }
            } else if bus.read(0x1f80_1074, Width::Word)? & bit == 0
                || bus.read(0x1f80_1070, Width::Word)? & bit == 0
            {
                cpu.set_register(2, 0);
            } else {
                cpu.set_register(4, 0xf200_0000 | index as u32);
                cpu.set_register(5, 2);
                self.exception_delivery = Some(ExceptionDelivery::Counter);
                let event = self.event_calls.start(cpu, bus)?;
                if event.is_some() {
                    return Ok(Call {
                        vector: 0x80,
                        service: 6,
                        name: "CounterInterruptEvent",
                        waiting: false,
                        callback: event,
                    });
                }
                self.exception_delivery = None;
                cpu.set_register(2, 1);
            }
            callback = self.exception_calls.as_mut().unwrap().resume(cpu, bus)?;
        }
        if callback.is_none() {
            if let Some(address) = self.entry_hook {
                let saved = read_jump(bus, address)?;
                super::exception_calls::validate_handler(saved[0])?;
                if saved[1] & 3 != 0 {
                    return Err("BIOS exception hook: unaligned saved stack".into());
                }
                ram_offset(saved[1], 0)?;
                for (register, value) in SAVED.iter().zip(saved) {
                    cpu.set_register(*register, value);
                }
                cpu.set_register(2, 1);
                cpu.resume_at(saved[0]);
                callback = Some(saved[0]);
            } else {
                match self.exception_calls.as_ref().ok_or("BIOS exception exit without continuation")?.missing_hook {
                    MissingHook::Reject => return Err("BIOS exception dispatch: no registered exit hook; default BIOS state is unverified".into()),
                    MissingHook::ReturnFromException => self.return_exception(cpu, bus)?,
                }
            }
        }
        Ok(Call {
            vector: 0x80,
            service: 6,
            name: "ExceptionHandler",
            waiting: false,
            callback,
        })
    }

    fn return_exception(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<()> {
        if self.exception_calls.is_none() {
            self.require_plain_context()?;
        }
        super::thread_context::Context::current(bus)?.restore(cpu, bus)?;
        // Entry rejects preexisting HLE continuations. Therefore these all belong
        // to the exception being abandoned by this explicit nonlocal return.
        self.exception_calls = None;
        self.event_calls = EventCalls::default();
        self.waiting_event = None;
        self.exception_delivery = None;
        Ok(())
    }
    /// Save the active guest thread after CPU exception entry. Handler dispatch,
    /// BIOS initial state and nested HLE continuation unwinding remain separate.
    pub fn save_exception(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<u32> {
        self.require_plain_context()?;
        let context = super::thread_context::Context::current(bus)?;
        context.save_exception(cpu, bus)?;
        Ok(context.address())
    }

    fn require_plain_context(&self) -> Result<()> {
        if self.exception_calls.is_some() {
            return Err("BIOS nested exception dispatch is unsupported".into());
        }
        if self.event_calls.depth() != 0 || self.waiting_event.is_some() {
            return Err("BIOS exception context: active event/wait continuation cannot be unwound implicitly".into());
        }
        Ok(())
    }
    /// Bounded BIOS syscall semantics, separate from priority-chain dispatch.
    /// This handles only the two critical-section calls.
    pub fn dispatch_syscall(&mut self, cpu: &mut Cpu, fault: &Fault) -> Result<Call> {
        if self.exception_calls.is_some() {
            return Err("BIOS nested syscall during exception dispatch is unsupported".into());
        }
        let service = cpu.register(4);
        if fault.kind != FaultKind::Syscall || fault.in_delay_slot || !matches!(service, 1 | 2) {
            return Err("unsupported BIOS syscall or branch-delay syscall context".into());
        }
        cpu.enter_exception(fault)?;
        let status = cpu.cop0().status();
        let mask = (1 << 2) | (1 << 10);
        let name = if service == 1 {
            cpu.set_register(2, u32::from(status & mask == mask));
            cpu.cop0_mut().write(12, status & !mask)?;
            "EnterCriticalSection"
        } else {
            cpu.cop0_mut().write(12, status | mask)?;
            "ExitCriticalSection"
        };
        cpu.cop0_mut().return_from_exception();
        cpu.resume_at(fault.pc.wrapping_add(4));
        Ok(Call {
            vector: 0x80,
            service,
            name,
            waiting: false,
            callback: None,
        })
    }

    pub fn is_call(pc: u32) -> bool {
        if pc == CALLBACK_RETURN || pc == HANDLER_RETURN {
            return true;
        }
        matches!(
            pc,
            0xa0 | 0xb0
                | 0xc0
                | 0x8000_00a0
                | 0x8000_00b0
                | 0x8000_00c0
                | 0xa000_00a0
                | 0xa000_00b0
                | 0xa000_00c0
        )
    }
    pub fn entry_hook(&self) -> Option<u32> {
        self.entry_hook
    }
    pub fn pending_callbacks(&self) -> usize {
        self.event_calls.depth()
    }
    pub fn pad_auto_ack(&self) -> u32 {
        self.pad_auto_ack
    }
    pub fn counter_auto_ack(&self) -> &[u32; 4] {
        &self.counter_auto_ack
    }

    pub fn dispatch(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Call> {
        if !Self::is_call(cpu.pc()) {
            return Err("not a BIOS entry vector".into());
        }
        cpu.synchronize_load();
        if cpu.pc() == HANDLER_RETURN {
            if self.event_calls.depth() != 0 || self.waiting_event.is_some() {
                return Err(
                    "BIOS exception handler return would abandon event/wait continuation".into(),
                );
            }
            let callback = self
                .exception_calls
                .as_mut()
                .ok_or("BIOS exception return trap without dispatch")?
                .resume(cpu, bus)?;
            return self.exception_progress(cpu, bus, callback);
        }
        if cpu.pc() == CALLBACK_RETURN {
            let callback = self.event_calls.resume(cpu, bus)?;
            if self.event_calls.depth() == 0 {
                if let Some(delivery) = self.exception_delivery {
                    match delivery {
                        ExceptionDelivery::Counter => {
                            self.exception_delivery = None;
                            cpu.set_register(2, 1);
                        }
                        ExceptionDelivery::Syscall => {
                            self.return_exception(cpu, bus)?;
                            return Ok(Call {
                                vector: 0x80,
                                service: 6,
                                name: "SyscallExceptionReturn",
                                waiting: false,
                                callback: None,
                            });
                        }
                        ExceptionDelivery::Generic { next } => {
                            if let Some(event) = self.generic_progress(cpu, bus, next)? {
                                return Ok(Call {
                                    vector: 0x80,
                                    service: 6,
                                    name: "GenericInterruptEvent",
                                    waiting: false,
                                    callback: Some(event),
                                });
                            }
                        }
                    }
                    let callback = self.exception_calls.as_mut().unwrap().resume(cpu, bus)?;
                    return self.exception_progress(cpu, bus, callback);
                }
            }
            return Ok(Call {
                vector: 0xb0,
                service: 7,
                name: "DeliverEventContinuation",
                waiting: false,
                callback,
            });
        }
        let vector = cpu.pc() & 0x1fff_ffff;
        let service = cpu.register(9);
        let argument = cpu.register(4);
        let mut destination = cpu.register(31);
        let mut waiting = false;
        let name = match (vector, service) {
            (0xb0, 0x17) => {
                self.return_exception(cpu, bus)?;
                return Ok(Call {
                    vector,
                    service,
                    name: "ReturnFromException",
                    waiting: false,
                    callback: None,
                });
            }
            (0xa0, 0x72) => {
                let entry = self
                    .cd_removal
                    .ok_or("owned CD removal entry is not configured")?;
                cpu.resume_at(entry);
                return Ok(Call {
                    vector,
                    service,
                    name: "_96_remove",
                    waiting: false,
                    callback: None,
                });
            }
            (0xa0, 0x13) => {
                validate_buffer(argument)?;
                for (slot, register) in SAVED.iter().enumerate() {
                    bus.write(
                        argument + slot as u32 * 4,
                        Width::Word,
                        cpu.register(*register),
                    )?;
                }
                cpu.set_register(2, 0);
                "setjmp"
            }
            (0xa0, 0x14) => {
                let value = cpu.register(5);
                let saved = read_jump(bus, argument)?;
                for (register, value) in SAVED.iter().zip(saved) {
                    cpu.set_register(*register, value);
                }
                cpu.set_register(2, value); // PSX does not replace zero with one.
                destination = cpu.register(31);
                "longjmp"
            }
            (0xb0, 0x19) => {
                validate_buffer(argument)?;
                self.entry_hook = Some(argument);
                "HookEntryInt"
            }
            (0xb0, 0x07) => {
                let callback = self.event_calls.start(cpu, bus)?;
                return Ok(Call {
                    vector,
                    service,
                    name: "DeliverEvent",
                    waiting: false,
                    callback,
                });
            }
            (0xb0, 0x08) => {
                let handle = Events::read(bus)?.open(
                    bus,
                    argument,
                    cpu.register(5),
                    cpu.register(6),
                    cpu.register(7),
                )?;
                cpu.set_register(2, handle);
                "OpenEvent"
            }
            (0xb0, 0x09) => {
                Events::read(bus)?.close(bus, argument)?;
                cpu.set_register(2, 1);
                "CloseEvent"
            }
            (0xb0, 0x0a) => {
                if let Some(pending) = &self.waiting_event {
                    if pending.handle != argument
                        || pending.return_pc != destination
                        || pending.callback_depth != self.event_calls.depth()
                    {
                        return Err(
                            "BIOS WaitEvent: nested or changed wait context is unsupported".into(),
                        );
                    }
                    // The real busy loop retains its original record pointer,
                    // and disabling/closing the event does not release the wait.
                    if Events::consume_ready(bus, pending.address)? {
                        cpu.set_register(2, 1);
                        self.waiting_event = None;
                    } else {
                        waiting = true;
                    }
                } else {
                    let address = Events::read(bus)?.address(argument)?;
                    if Events::consume_ready(bus, address)? {
                        cpu.set_register(2, 1);
                    } else if bus.read(address + 4, Width::Word)? == BUSY {
                        self.waiting_event = Some(PendingWait {
                            handle: argument,
                            address,
                            return_pc: destination,
                            callback_depth: self.event_calls.depth(),
                        });
                        waiting = true;
                    } else {
                        cpu.set_register(2, 0);
                    }
                }
                "WaitEvent"
            }
            (0xb0, 0x0b) => {
                let ready = Events::read(bus)?.test(bus, argument)?;
                cpu.set_register(2, u32::from(ready));
                "TestEvent"
            }
            (0xb0, 0x0c | 0x0d) => {
                Events::read(bus)?.enable(bus, argument, service == 0x0c)?;
                cpu.set_register(2, 1);
                if service == 0x0c {
                    "EnableEvent"
                } else {
                    "DisableEvent"
                }
            }
            (0xb0, 0x20) => {
                Events::read(bus)?.undeliver(bus, argument, cpu.register(5))?;
                "UnDeliverEvent"
            }
            (0xb0, 0x5b) => {
                let old = self.pad_auto_ack;
                self.pad_auto_ack = argument;
                cpu.set_register(2, old);
                "ChangeClearPAD"
            }
            (0xc0, 0x0a) => {
                let flag = self
                    .counter_auto_ack
                    .get_mut(argument as usize)
                    .ok_or("invalid timer/vblank auto-ack index")?;
                let old = *flag;
                *flag = cpu.register(5);
                cpu.set_register(2, old);
                "ChangeClearRCnt"
            }
            (0xc0, 0x04) => {
                cpu.set_register(2, Events::read(bus)?.free_slot(bus)?);
                "get_free_EvCB_slot"
            }
            (0xc0, 0x02) => {
                ExceptionChains::read(bus)?.enqueue(bus, argument, cpu.register(5))?;
                cpu.set_register(2, 0);
                "SysEnqIntRP"
            }
            (0xc0, 0x03) => {
                let removed =
                    ExceptionChains::read(bus)?.dequeue(bus, argument, cpu.register(5))?;
                cpu.set_register(2, removed);
                "SysDeqIntRP"
            }
            _ => {
                return Err(format!(
                    "unsupported BIOS vector 0x{vector:02X}, service 0x{service:02X}"
                )
                .into())
            }
        };
        if !waiting {
            cpu.resume_at(destination);
        }
        Ok(Call {
            vector,
            service,
            name,
            waiting,
            callback: None,
        })
    }
}

fn validate_buffer(address: u32) -> Result<()> {
    if address & 3 != 0 {
        return Err("unaligned kernel context buffer".into());
    }
    ram_offset(address, 48)?;
    Ok(())
}

fn read_jump(bus: &mut impl Bus, address: u32) -> Result<[u32; 12]> {
    validate_buffer(address)?;
    let mut saved = [0; 12];
    for (slot, register) in saved.iter_mut().enumerate() {
        *register = bus.read(address + slot as u32 * 4, Width::Word)?;
    }
    Ok(saved)
}
