//! DeliverEvent ownership for functional scans and ordinary guest calls.
use super::{
    bus::Bus,
    cpu::{Cpu, Step},
    events::{Delivery, Table, CALLBACK, POLLING, RECORD_BYTES},
    exception_calls::HANDLER_RETURN,
    executable::ram_offset,
};
use crate::Result;

// The functional development model retains its explicit HLE return trap.
pub(crate) const CALLBACK_RETURN: u32 = 0xbfc0_0100;
const SAVED: [usize; 9] = [16, 17, 18, 19, 20, 21, 22, 23, 30];
const MAX_DEPTH: usize = 64;

/// Linked boundaries of the guest DeliverEvent routine. The callback return is
/// callback + 8; the final return delay slot is return_jump + 4.
#[derive(Clone, Copy, Debug)]
pub struct Service {
    pub entry: u32,
    pub scan: u32,
    pub mode: u32,
    pub callback: u32,
    pub return_jump: u32,
}

impl Service {
    pub(crate) fn addresses(self) -> Result<[u32; 7]> {
        Ok([
            self.entry,
            self.scan,
            self.mode,
            self.callback,
            self.callback
                .checked_add(8)
                .ok_or("event callback return overflow")?,
            self.return_jump,
            self.return_jump
                .checked_add(4)
                .ok_or("event return delay overflow")?,
        ])
    }

    fn contains(self, address: u32) -> bool {
        let first = self.entry & 0x1fff_ffff;
        let last = self.return_jump.saturating_add(4) & 0x1fff_ffff;
        (first..=last).contains(&(address & 0x1fff_ffff))
    }
}

#[derive(Clone, Debug)]
struct Host {
    delivery: Delivery,
    return_pc: u32,
    stack: u32,
    saved: [u32; 9],
    callback_stack: Option<u32>,
}

#[derive(Clone, Copy, Debug)]
enum CallbackPhase {
    Call,
    Delay,
    Entry,
    Active,
}

#[derive(Clone, Debug)]
struct Callback {
    target: u32,
    saved: [u32; 9],
    phase: CallbackPhase,
}

#[derive(Clone, Debug)]
struct Guest {
    binding: Service,
    return_pc: u32,
    stack: u32,
    saved: [u32; 9],
    class: u32,
    spec: u32,
    next: u32,
    end: u32,
    validated: bool,
    entered: bool,
    in_record: bool,
    callback: Option<Callback>,
    return_instruction: Option<u32>,
}

#[derive(Clone, Debug)]
enum Frame {
    Host(Host),
    Guest(Guest),
}

#[derive(Clone, Debug, Default)]
pub(crate) struct Calls {
    frames: Vec<Frame>,
}

impl Calls {
    pub fn depth(&self) -> usize {
        self.frames.len()
    }

    pub fn start(
        &mut self,
        cpu: &mut Cpu,
        bus: &mut impl Bus,
        binding: Option<Service>,
        exception_active: bool,
    ) -> Result<Option<u32>> {
        if let Some(binding) = binding {
            self.start_guest(cpu, bus, binding, exception_active)?;
            cpu.resume_at(binding.entry);
            return Ok(Some(binding.entry));
        }
        self.check_depth()?;
        let table = Table::read(bus)?;
        self.frames.push(Frame::Host(Host {
            delivery: table.delivery(cpu.register(4), cpu.register(5)),
            return_pc: cpu.register(31),
            stack: cpu.register(29),
            saved: saved(cpu),
            callback_stack: None,
        }));
        self.advance(cpu, bus)
    }

    fn check_depth(&self) -> Result<()> {
        if self.depth() >= MAX_DEPTH {
            return Err("BIOS DeliverEvent: callback nesting limit (64) exceeded".into());
        }
        Ok(())
    }

    fn start_guest(
        &mut self,
        cpu: &Cpu,
        bus: &mut impl Bus,
        binding: Service,
        exception_active: bool,
    ) -> Result<()> {
        self.check_depth()?;
        let mut incoming = cpu.clone();
        incoming.synchronize_load();
        let cpu = &incoming;
        let (base, end) = Table::read(bus)?.range();
        let return_pc = cpu.register(31);
        let owned_return = (return_pc == HANDLER_RETURN && exception_active)
            || (return_pc == CALLBACK_RETURN
                && matches!(self.frames.last(), Some(Frame::Host(host)) if host.callback_stack.is_some()));
        let stack = cpu.register(29);
        self.frames.push(Frame::Guest(Guest {
            binding,
            return_pc,
            stack,
            saved: saved(cpu),
            class: cpu.register(4),
            spec: cpu.register(5),
            next: base,
            end,
            validated: false,
            entered: false,
            in_record: false,
            callback: None,
            return_instruction: None,
        }));
        let frame = stack
            .checked_sub(64)
            .filter(|_| stack & 7 == 0)
            .ok_or("BIOS event callback requires an aligned guest stack with argument space")?;
        ram_offset(frame, 64)?;
        if return_pc & 3 != 0 || binding.contains(return_pc) {
            return Err("guest event service requires an external aligned return address".into());
        }
        if !owned_return {
            ram_offset(return_pc, 4)?;
        }
        let Some(Frame::Guest(guest)) = self.frames.last_mut() else {
            unreachable!()
        };
        guest.validated = true;
        Ok(())
    }

    /// Validate direct entries and service boundaries without changing the CPU.
    pub fn prepare_guest_step(
        &mut self,
        cpu: &Cpu,
        bus: &mut impl Bus,
        binding: Service,
        exception_active: bool,
        waiting_depth: Option<usize>,
    ) -> Result<()> {
        let depth = self.depth();
        let nested_entry = cpu.pc() == binding.entry;
        if let Some(Frame::Guest(guest)) = self.frames.last_mut() {
            if !guest.validated {
                return Err("owned event entry retains a failed preflight".into());
            }
            if !guest.entered {
                if cpu.pc() != binding.entry {
                    return Err("owned event service has not entered its bound routine".into());
                }
                guest.entered = true;
                return Ok(());
            }
            guest.prepare(cpu, waiting_depth.is_some_and(|wait| wait >= depth))?;
            if nested_entry
                && !matches!(
                    guest.callback.as_ref().map(|c| c.phase),
                    Some(CallbackPhase::Active)
                )
            {
                return Err("owned event entry would abandon its active service".into());
            }
        } else if binding.addresses()?.contains(&cpu.pc()) && !nested_entry {
            return Err("guest event boundary without an owned delivery".into());
        }
        if nested_entry {
            self.start_guest(cpu, bus, binding, exception_active)?;
            let Some(Frame::Guest(guest)) = self.frames.last_mut() else {
                unreachable!()
            };
            guest.entered = true;
        }
        Ok(())
    }

    /// Require the actual callback JALR and final JR, including their delay slots.
    pub fn complete_guest_step(&mut self, cpu: &Cpu, step: &Step) -> Result<bool> {
        let Some(Frame::Guest(guest)) = self.frames.last_mut() else {
            return Ok(false);
        };
        if guest.complete(cpu, step)? {
            self.frames.pop();
            return Ok(true);
        }
        Ok(false)
    }

    pub fn resume(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        let Some(Frame::Host(host)) = self.frames.last_mut() else {
            return Err("functional callback return requires a functional delivery".into());
        };
        if Some(cpu.register(29)) != host.callback_stack || saved(cpu) != host.saved {
            return Err(
                "BIOS event callback violated the stack/callee-saved register contract".into(),
            );
        }
        host.callback_stack = None;
        self.advance(cpu, bus)
    }

    fn advance(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        let Some(Frame::Host(host)) = self.frames.last_mut() else {
            return Err("missing functional event continuation".into());
        };
        if let Some(handler) = host.delivery.next_callback(bus)? {
            if handler & 3 != 0 {
                return Err(format!("unaligned BIOS event callback 0x{handler:08X}").into());
            }
            ram_offset(handler, 4)?;
            let stack = host
                .stack
                .checked_sub(16)
                .filter(|_| host.stack & 7 == 0)
                .ok_or("BIOS event callback requires an aligned guest stack with argument space")?;
            ram_offset(stack, 16)?;
            host.callback_stack = Some(stack);
            cpu.set_register(29, stack);
            cpu.set_register(31, CALLBACK_RETURN);
            cpu.resume_at(handler);
            Ok(Some(handler))
        } else {
            let Frame::Host(host) = self.frames.pop().unwrap() else {
                unreachable!()
            };
            cpu.set_register(29, host.stack);
            cpu.set_register(31, host.return_pc);
            cpu.resume_at(host.return_pc);
            Ok(None)
        }
    }
}

impl Guest {
    fn validate_scan(&self, cpu: &Cpu) -> Result<()> {
        if cpu.register(29) != self.stack - 64
            || cpu.register(16) != self.next
            || self.next >= self.end
            || cpu.register(20) != self.end
            || cpu.register(21) != self.class
            || cpu.register(22) != self.spec
            || cpu.register(17) != 0x2000
            || cpu.register(18) != 0x4000
            || cpu.register(19) != 0x1000
        {
            return Err("owned event scanner lost its table, stack or saved cursor".into());
        }
        Ok(())
    }

    fn prepare(&mut self, cpu: &Cpu, waiting: bool) -> Result<()> {
        let pc = cpu.pc();
        let binding = self.binding;
        if let Some(expected) = self.return_instruction {
            if pc != expected {
                return Err("owned event return must retire its JR and delay slot".into());
            }
            return Ok(());
        }
        if let Some(callback) = &mut self.callback {
            let expected = match callback.phase {
                CallbackPhase::Call => Some(binding.callback),
                CallbackPhase::Delay => Some(binding.callback + 4),
                CallbackPhase::Entry => Some(callback.target),
                CallbackPhase::Active => None,
            };
            if let Some(expected) = expected {
                if pc != expected {
                    return Err(
                        "owned event callback must retire its JALR and enter its target".into(),
                    );
                }
                if matches!(callback.phase, CallbackPhase::Entry) {
                    if cpu.register(31) != binding.callback + 8 {
                        return Err("owned event callback lost its guest return address".into());
                    }
                    callback.phase = CallbackPhase::Active;
                }
                return Ok(());
            }
            if pc == binding.callback + 8 {
                if waiting || cpu.register(29) != self.stack - 64 || saved(cpu) != callback.saved {
                    return Err("BIOS event callback violated the stack/callee-saved register contract or abandoned WaitEvent".into());
                }
                self.callback = None;
            } else {
                if binding.addresses()?.contains(&pc) && pc != binding.entry {
                    return Err("owned event boundary would abandon an active callback".into());
                }
                return Ok(());
            }
        }
        if pc == binding.scan {
            if self.in_record {
                return Err("owned event scan revisited a record before advancing".into());
            }
            self.validate_scan(cpu)?;
            self.in_record = true;
        } else if pc == binding.mode || pc == binding.callback || pc == binding.callback + 8 {
            if !self.in_record {
                return Err("owned event boundary without an active record".into());
            }
            self.validate_scan(cpu)?;
            if pc == binding.mode {
                if !matches!(cpu.register(2), CALLBACK | POLLING) {
                    return Err(format!(
                        "BIOS DeliverEvent: unsupported mode 0x{:08X}",
                        cpu.register(2)
                    )
                    .into());
                }
            } else if pc == binding.callback {
                let target = cpu.register(2);
                if target & 3 != 0 {
                    return Err(format!("unaligned BIOS event callback 0x{target:08X}").into());
                }
                if target == 0 || binding.contains(target) {
                    return Err("invalid or overlapping guest event callback".into());
                }
                ram_offset(target, 4)?;
                self.callback = Some(Callback {
                    target,
                    saved: saved(cpu),
                    phase: CallbackPhase::Call,
                });
            } else {
                self.next += RECORD_BYTES;
                self.in_record = false;
            }
        } else if pc == binding.return_jump {
            if waiting
                || self.in_record
                || self.next != self.end
                || cpu.register(29) != self.stack - 64
                || cpu.register(31) != self.return_pc
            {
                return Err("owned event service returned before completing its delivery".into());
            }
            self.return_instruction = Some(pc);
        } else if pc == binding.return_jump + 4 {
            return Err("owned event return delay slot without its JR".into());
        }
        Ok(())
    }

    fn complete(&mut self, cpu: &Cpu, step: &Step) -> Result<bool> {
        if let Some(expected) = self.return_instruction {
            if step.pc != expected {
                return Err("owned event return retired an unexpected instruction".into());
            }
            if expected == self.binding.return_jump {
                if step.in_delay_slot
                    || step.instruction != 0x03e0_0008
                    || step.branch_target != Some(self.return_pc)
                    || cpu.pc() != expected + 4
                    || saved(cpu) != self.saved
                {
                    return Err("owned event return requires its bound JR instruction".into());
                }
                self.return_instruction = Some(expected + 4);
            } else {
                if !step.in_delay_slot
                    || step.instruction != 0x27bd_0040
                    || cpu.pc() != self.return_pc
                    || cpu.register(29) != self.stack
                    || saved(cpu) != self.saved
                {
                    return Err(
                        "owned event return requires its completed stack-restoring delay slot"
                            .into(),
                    );
                }
                return Ok(true);
            }
        }
        if let Some(callback) = &mut self.callback {
            match callback.phase {
                CallbackPhase::Call => {
                    if step.pc != self.binding.callback
                        || step.in_delay_slot
                        || step.instruction != 0x0040_f809
                        || step.branch_target != Some(callback.target)
                        || cpu.pc() != self.binding.callback + 4
                        || cpu.register(31) != self.binding.callback + 8
                    {
                        return Err(
                            "owned event callback requires its bound JALR instruction".into()
                        );
                    }
                    callback.phase = CallbackPhase::Delay;
                }
                CallbackPhase::Delay => {
                    if step.pc != self.binding.callback + 4
                        || !step.in_delay_slot
                        || step.instruction != 0
                        || cpu.pc() != callback.target
                        || cpu.register(31) != self.binding.callback + 8
                    {
                        return Err(
                            "owned event callback requires its completed NOP delay slot".into()
                        );
                    }
                    callback.phase = CallbackPhase::Entry;
                }
                CallbackPhase::Entry => {
                    return Err("owned event callback entry was not validated".into())
                }
                CallbackPhase::Active => {}
            }
        }
        Ok(false)
    }
}

fn saved(cpu: &Cpu) -> [u32; 9] {
    SAVED.map(|register| cpu.register(register))
}
