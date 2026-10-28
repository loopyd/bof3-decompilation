//! Bounded BIOS priority-chain continuations; handlers execute as guest code.
use super::{
    bus::{Bus, Width},
    cpu::{Cpu, Step},
    exception_chains::Table,
    executable::ram_offset,
};
use crate::Result;
use std::collections::BTreeSet;

pub(crate) const HANDLER_RETURN: u32 = 0xbfc0_0104;
pub(crate) const THREAD_CONTEXT: u32 = 0xbfc0_011c;
const SAVED: [usize; 10] = [16, 17, 18, 19, 20, 21, 22, 23, 28, 30];
const MAX_NODES: usize = 4096;

#[derive(Clone, Copy, Debug)]
pub enum MissingHook {
    Reject,
    /// Explicit functional default; does not manufacture a BIOS jump buffer.
    ReturnFromException,
}
#[derive(Clone, Copy, Debug)]
pub struct Environment {
    /// Caller-owned RAM stack, with a separate 16-byte O32 argument home area.
    pub stack_top: u32,
    pub global_pointer: u32,
    pub missing_hook: MissingHook,
}

/// Linked instruction boundaries and initialized state for the owned scanner.
/// The two JALR return boundaries are the call addresses plus eight bytes.
#[derive(Clone, Copy, Debug)]
pub struct Dispatch {
    pub entry: u32,
    pub priority: u32,
    pub node: u32,
    pub verifier: u32,
    pub handler: u32,
    pub exit: u32,
    pub stack_pointer: u32,
}
impl Dispatch {
    pub(crate) fn addresses(self) -> Result<[u32; 9]> {
        Ok([
            self.entry,
            self.priority,
            self.node,
            self.verifier,
            self.verifier
                .checked_add(8)
                .ok_or("owned verifier return overflow")?,
            self.handler,
            self.handler
                .checked_add(8)
                .ok_or("owned handler return overflow")?,
            self.exit,
            self.stack_pointer,
        ])
    }
}

pub(crate) enum GuestBoundary {
    Continue,
    Handler(u32),
    Exit,
}

#[derive(Clone, Copy, Debug)]
enum Cursor {
    Capture,
    Guest,
    Priority,
    Node(u32),
    Verifier { node: u32, second: u32 },
    Handler { node: u32 },
    Exit,
}
#[derive(Clone, Debug)]
pub(crate) struct Calls {
    table: Table,
    priority: u32,
    cursor: Cursor,
    visited: BTreeSet<usize>,
    stack: u32,
    saved: Option<[u32; 10]>,
    guest: bool,
    return_pc: Option<u32>,
    handler_entry: Option<u32>,
    call_instruction: Option<u32>,
    global_pointer: u32,
    pub missing_hook: MissingHook,
}
impl Calls {
    pub fn prepare(bus: &mut impl Bus, environment: Environment) -> Result<Self> {
        let stack = environment
            .stack_top
            .checked_sub(16)
            .filter(|_| environment.stack_top & 7 == 0)
            .ok_or("BIOS exception dispatch: aligned RAM handler stack required")?;
        ram_offset(stack, 16)?;
        Ok(Self {
            table: Table::read(bus)?,
            priority: 0,
            cursor: Cursor::Priority,
            visited: BTreeSet::new(),
            stack,
            saved: None,
            guest: false,
            return_pc: None,
            handler_entry: None,
            call_instruction: None,
            global_pointer: environment.global_pointer,
            missing_hook: environment.missing_hook,
        })
    }

    pub fn bind_guest(&mut self) {
        self.guest = true;
        self.cursor = Cursor::Capture;
        self.priority = u32::MAX;
    }

    pub fn guest_running(&self) -> bool {
        self.guest && matches!(self.cursor, Cursor::Guest) && self.saved.is_none()
    }

    pub fn validate_guest_stack(&self, bus: &mut impl Bus, binding: Dispatch) -> Result<()> {
        if bus.read(binding.stack_pointer, Width::Word)? != self.stack {
            return Err(
                "owned exception stack cell disagrees with the explicit environment".into(),
            );
        }
        Ok(())
    }

    pub fn prepare_guest_step(
        &mut self,
        cpu: &Cpu,
        binding: Dispatch,
        delivery_active: bool,
    ) -> Result<GuestBoundary> {
        if !self.guest {
            return Ok(GuestBoundary::Continue);
        }
        let pc = cpu.pc();
        let boundary = binding.addresses()?.into_iter().take(8).any(|at| at == pc);
        if self.saved.is_some() {
            if let Some(expected) = self.call_instruction {
                if pc != expected || delivery_active {
                    return Err("owned exception call must retire its JALR and delay slot".into());
                }
                return Ok(GuestBoundary::Continue);
            }
            if self.return_pc == Some(pc) {
                if self.handler_entry.is_some() {
                    return Err("owned exception return preceded handler entry".into());
                }
                if delivery_active {
                    return Err(
                        "owned exception handler return would abandon event/wait continuation"
                            .into(),
                    );
                }
                self.validate_handler_return(cpu)?;
                self.saved = None;
                self.return_pc = None;
                self.handler_entry = None;
                return Ok(GuestBoundary::Continue);
            }
            if boundary {
                return Err("owned exception boundary would abandon an active handler".into());
            }
            if self.handler_entry == Some(pc) {
                if cpu.register(31) != self.return_pc.unwrap() {
                    return Err("owned exception JALR did not establish its return address".into());
                }
                self.handler_entry = None;
                return Ok(GuestBoundary::Handler(pc));
            }
            return Ok(GuestBoundary::Continue);
        }
        if boundary && delivery_active {
            return Err("owned exception boundary would abandon an active delivery".into());
        }
        if pc == binding.priority {
            if matches!(self.cursor, Cursor::Capture) {
                if cpu.register(19) != self.table.base()
                    || cpu.register(20) != self.table.base().wrapping_add(32)
                    || cpu.register(28) != self.global_pointer
                    || cpu.register(29) != self.stack
                    || cpu.register(30) != self.stack
                {
                    return Err(
                        "owned exception capture disagrees with its table/stack/GP environment"
                            .into(),
                    );
                }
                self.cursor = Cursor::Guest;
            }
            self.require_guest_scan()?;
            let next = self.priority.wrapping_add(1);
            if next >= 4 || cpu.register(19) != self.table.base().wrapping_add(next * 8) {
                return Err("owned exception priorities must advance in order".into());
            }
            self.priority = next;
            self.visited.clear();
        } else if pc == binding.node {
            self.require_guest_scan()?;
            self.visit(cpu.register(22))?;
        } else if pc == binding.verifier || pc == binding.handler {
            self.require_guest_scan()?;
            if cpu.register(29) != self.stack {
                return Err("owned exception scanner changed its handler stack".into());
            }
            let handler = cpu.register(if pc == binding.verifier { 17 } else { 16 });
            validate_handler(handler)?;
            let target = ram_offset(handler, 4)?;
            if (ram_offset(binding.entry, 4)?..=ram_offset(binding.exit, 4)?).contains(&target) {
                return Err("owned exception handler targets reserved scanner code".into());
            }
            self.call_instruction = Some(pc);
            self.return_pc = Some(pc + 8);
            self.handler_entry = Some(handler);
            self.saved = Some(SAVED.map(|r| cpu.register(r)));
        } else if pc == binding.exit {
            self.require_guest_scan()?;
            if self.priority != 3 {
                return Err("owned exception exit requires all four priorities".into());
            }
            self.cursor = Cursor::Exit;
            return Ok(GuestBoundary::Exit);
        }
        Ok(GuestBoundary::Continue)
    }

    pub fn complete_guest_step(&mut self, cpu: &Cpu, step: &Step, binding: Dispatch) -> Result<()> {
        let Some(expected) = self.call_instruction else {
            return Ok(());
        };
        let return_pc = self
            .return_pc
            .ok_or("owned exception call lost its return address")?;
        let target = self
            .handler_entry
            .ok_or("owned exception call lost its target")?;
        if step.pc != expected {
            return Err("owned exception call retired an unexpected instruction".into());
        }
        if expected == return_pc - 8 {
            let instruction = if expected == binding.verifier {
                0x0220_f809
            } else {
                0x0200_f809
            };
            if step.in_delay_slot
                || step.instruction != instruction
                || step.branch_target != Some(target)
                || cpu.pc() != expected + 4
                || cpu.register(31) != return_pc
            {
                return Err("owned exception call requires its bound JALR instruction".into());
            }
            self.call_instruction = Some(expected + 4);
        } else {
            if !step.in_delay_slot
                || step.instruction != 0
                || cpu.pc() != target
                || cpu.register(31) != return_pc
            {
                return Err("owned exception call requires its completed NOP delay slot".into());
            }
            self.call_instruction = None;
        }
        Ok(())
    }

    pub fn guard_thread_context(&self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<u32> {
        if self.saved.is_none() {
            return Err("owned thread guard requires an active exception handler".into());
        }
        let destination = cpu.register(31);
        validate_handler(destination)?;
        let context = super::thread_context::Context::current(bus)?.address();
        if !matches!(bus.read(context + 0x98, Width::Word)? & 0x3c, 0 | 0x20) {
            return Err(
                "owned syscall handler: other exception failure policy is unsupported".into(),
            );
        }
        cpu.set_register(2, context);
        cpu.resume_at(destination);
        Ok(destination)
    }

    fn require_guest_scan(&self) -> Result<()> {
        if !self.guest_running() || self.return_pc.is_some() {
            return Err("owned exception guard requires an active guest scan".into());
        }
        Ok(())
    }

    pub fn resume(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        self.saved
            .ok_or("BIOS exception dispatch: return trap without active handler")?;
        self.validate_handler_return(cpu)?;
        self.saved = None;
        if self.guest {
            let destination = self
                .return_pc
                .take()
                .ok_or("owned exception handler has no guest continuation")?;
            self.handler_entry = None;
            cpu.set_register(31, destination);
            cpu.resume_at(destination);
            return Ok(Some(destination));
        }
        match self.cursor {
            Cursor::Verifier { node, second } => {
                if cpu.register(2) != 0 {
                    cpu.set_register(4, cpu.register(2));
                    if second != 0 {
                        self.cursor = Cursor::Handler { node };
                        return self.invoke(cpu, second).map(Some);
                    }
                }
                self.cursor = Cursor::Node(bus.read(node, Width::Word)?);
            }
            Cursor::Handler { node } => self.cursor = Cursor::Node(bus.read(node, Width::Word)?),
            _ => return Err("BIOS exception dispatch: inconsistent return continuation".into()),
        }
        self.advance(cpu, bus)
    }

    pub fn advance(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        loop {
            match self.cursor {
                Cursor::Priority => {
                    if self.priority == 4 {
                        self.cursor = Cursor::Exit;
                        return Ok(None);
                    }
                    self.visited.clear();
                    self.cursor = Cursor::Node(self.table.head(bus, self.priority)?);
                }
                Cursor::Node(0) => {
                    self.priority += 1;
                    self.cursor = Cursor::Priority;
                }
                Cursor::Node(node) => {
                    self.visit(node)?;
                    // Both pointers are captured before the verifier runs. The
                    // next link is read only after the guest handlers complete.
                    let first = bus.read(node + 8, Width::Word)?;
                    let second = bus.read(node + 4, Width::Word)?;
                    if first == 0 {
                        self.cursor = Cursor::Node(bus.read(node, Width::Word)?);
                        continue;
                    }
                    self.cursor = Cursor::Verifier { node, second };
                    return self.invoke(cpu, first).map(Some);
                }
                Cursor::Exit => {
                    return Err("BIOS exception dispatch: exit hook cannot reenter chain".into())
                }
                _ => return Err("BIOS exception dispatch: handler has not returned".into()),
            }
        }
    }

    fn visit(&mut self, node: u32) -> Result<()> {
        let physical = self.table.node(node)?;
        if self.visited.len() == MAX_NODES {
            return Err("BIOS exception dispatch: 4096-node priority safety limit".into());
        }
        if self
            .visited
            .range(physical.saturating_sub(15)..physical + 16)
            .next()
            .is_some()
        {
            return Err("BIOS exception dispatch: cycle or overlapping visited nodes".into());
        }
        self.visited.insert(physical);
        Ok(())
    }

    fn validate_handler_return(&self, cpu: &Cpu) -> Result<()> {
        if cpu.register(29) != self.stack || self.saved != Some(SAVED.map(|r| cpu.register(r))) {
            return Err("BIOS exception handler violated stack/callee-saved contract".into());
        }
        Ok(())
    }

    fn invoke(&mut self, cpu: &mut Cpu, handler: u32) -> Result<u32> {
        validate_handler(handler)?;
        cpu.set_register(29, self.stack);
        cpu.set_register(31, HANDLER_RETURN);
        self.saved = Some(SAVED.map(|r| cpu.register(r)));
        cpu.resume_at(handler);
        Ok(handler)
    }
}
pub(crate) fn validate_handler(handler: u32) -> Result<()> {
    if handler == 0 || handler & 3 != 0 {
        return Err("BIOS exception dispatch: null/unaligned guest handler".into());
    }
    ram_offset(handler, 4)?;
    Ok(())
}
