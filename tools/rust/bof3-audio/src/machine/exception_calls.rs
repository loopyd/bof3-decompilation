//! Bounded BIOS priority-chain continuations; handlers execute as guest code.
use super::{
    bus::{Bus, Width},
    cpu::Cpu,
    exception_chains::Table,
    executable::ram_offset,
};
use crate::Result;
use std::collections::BTreeSet;

pub(crate) const HANDLER_RETURN: u32 = 0xbfc0_0104;
pub(crate) const EXCEPTION_ENTER: u32 = 0xbfc0_010c;
pub(crate) const NODE_ENTER: u32 = 0xbfc0_0110;
pub(crate) const HANDLER_ENTER: u32 = 0xbfc0_0114;
pub(crate) const EXCEPTION_EXIT: u32 = 0xbfc0_0118;
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

    pub fn enter_guest(&mut self, cpu: &mut Cpu) -> Result<u32> {
        if !self.guest || !matches!(self.cursor, Cursor::Capture) {
            return Err("owned exception entry requires an unfinished guest capture".into());
        }
        let destination = cpu.register(31);
        validate_handler(destination)?;
        self.cursor = Cursor::Guest;
        cpu.set_register(4, self.table.base());
        cpu.set_register(28, self.global_pointer);
        cpu.set_register(29, self.stack);
        cpu.set_register(30, self.stack + 16);
        cpu.resume_at(destination);
        Ok(destination)
    }

    pub fn guard_guest_node(&mut self, cpu: &mut Cpu) -> Result<u32> {
        self.require_guest_scan()?;
        let priority = cpu.register(24);
        if priority >= 4 {
            return Err("owned exception priority outside 0..4".into());
        }
        let destination = cpu.register(31);
        validate_handler(destination)?;
        if priority != self.priority {
            if priority != self.priority.wrapping_add(1) {
                return Err("owned exception priorities must advance in order".into());
            }
            self.visited.clear();
            self.priority = priority;
        }
        let node = cpu.register(25);
        if node != 0 {
            self.visit(node)?;
        }
        cpu.resume_at(destination);
        Ok(destination)
    }

    pub fn enter_guest_handler(&mut self, cpu: &mut Cpu) -> Result<u32> {
        self.require_guest_scan()?;
        if cpu.register(29) != self.stack {
            return Err("owned exception scanner changed its handler stack".into());
        }
        let destination = cpu.register(31);
        validate_handler(destination)?;
        let handler = cpu.register(25);
        validate_handler(handler)?;
        self.return_pc = Some(destination);
        self.invoke(cpu, handler)
    }

    pub fn exit_guest(&mut self) -> Result<()> {
        self.require_guest_scan()?;
        if self.priority != 3 {
            return Err("owned exception exit requires all four priorities".into());
        }
        self.cursor = Cursor::Exit;
        Ok(())
    }

    fn require_guest_scan(&self) -> Result<()> {
        if !self.guest_running() || self.return_pc.is_some() {
            return Err("owned exception guard requires an active guest scan".into());
        }
        Ok(())
    }

    pub fn resume(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        let saved = self
            .saved
            .ok_or("BIOS exception dispatch: return trap without active handler")?;
        if cpu.register(29) != self.stack || SAVED.map(|r| cpu.register(r)) != saved {
            return Err("BIOS exception handler violated stack/callee-saved contract".into());
        }
        self.saved = None;
        if self.guest {
            let destination = self
                .return_pc
                .take()
                .ok_or("owned exception handler has no guest continuation")?;
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
