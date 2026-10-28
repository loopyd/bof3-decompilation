//! DeliverEvent continuations for host scans and bound guest services.
use super::{
    bus::Bus,
    cpu::Cpu,
    events::{Delivery, Table},
    executable::ram_offset,
};
use crate::Result;

// Reserved HLE return trap, not original BIOS code or an emulated TLB vector.
pub(crate) const CALLBACK_RETURN: u32 = 0xbfc0_0100;
pub(crate) const CALLBACK_ENTER: u32 = 0xbfc0_0108;
const SAVED: [usize; 9] = [16, 17, 18, 19, 20, 21, 22, 23, 30];
const MAX_DEPTH: usize = 64;

#[derive(Clone, Debug)]
struct Frame {
    delivery: Source,
    return_pc: u32,
    stack: u32,
    saved: [u32; 9],
    callback_stack: Option<u32>,
    active: bool,
}

#[derive(Clone, Debug)]
enum Source {
    Host(Delivery),
    Guest,
    Callback,
}

#[derive(Clone, Debug, Default)]
pub(crate) struct Calls {
    frames: Vec<Frame>,
}

impl Calls {
    pub fn depth(&self) -> usize {
        self.frames
            .iter()
            .filter(|frame| !matches!(frame.delivery, Source::Callback))
            .count()
    }

    pub fn start(
        &mut self,
        cpu: &mut Cpu,
        bus: &mut impl Bus,
        guest_entry: Option<u32>,
    ) -> Result<Option<u32>> {
        if self.depth() >= MAX_DEPTH {
            return Err("BIOS DeliverEvent: callback nesting limit (64) exceeded".into());
        }
        let table = Table::read(bus)?;
        let delivery = match guest_entry {
            Some(_) => Source::Guest,
            None => Source::Host(table.delivery(cpu.register(4), cpu.register(5))),
        };
        self.frames.push(Frame {
            delivery,
            return_pc: cpu.register(31),
            stack: cpu.register(29),
            saved: SAVED.map(|register| cpu.register(register)),
            callback_stack: None,
            active: false,
        });
        if let Some(entry) = guest_entry {
            enter_callback(cpu, self.frames.last_mut().unwrap(), entry, true).map(Some)
        } else {
            self.advance(cpu, bus)
        }
    }

    /// Guard one callback selected by the guest scanner. Its O32 argument home
    /// area is already part of the guest frame. This is not a nested delivery.
    pub fn start_callback(&mut self, cpu: &mut Cpu) -> Result<u32> {
        if !self
            .frames
            .last()
            .is_some_and(|frame| frame.active && matches!(frame.delivery, Source::Guest))
        {
            return Err("guest event callback requires an active guest delivery".into());
        }
        self.frames.push(Frame {
            delivery: Source::Callback,
            return_pc: cpu.register(31),
            stack: cpu.register(29),
            saved: SAVED.map(|register| cpu.register(register)),
            callback_stack: None,
            active: false,
        });
        let handler = cpu.register(25);
        enter_callback(cpu, self.frames.last_mut().unwrap(), handler, false)
    }

    pub fn resume(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        let frame = self
            .frames
            .last_mut()
            .ok_or("BIOS callback return without pending delivery")?;
        if !frame.active
            || Some(cpu.register(29)) != frame.callback_stack
            || SAVED.map(|register| cpu.register(register)) != frame.saved
        {
            return Err(
                "BIOS event callback violated the stack/callee-saved register contract".into(),
            );
        }
        frame.active = false;
        if !matches!(frame.delivery, Source::Host(_)) {
            self.finish(cpu);
            return Ok(None);
        }
        self.advance(cpu, bus)
    }

    fn advance(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        let frame = self
            .frames
            .last_mut()
            .ok_or("missing BIOS event continuation")?;
        let Source::Host(delivery) = &mut frame.delivery else {
            return Err("guest event service has not returned".into());
        };
        if let Some(handler) = delivery.next_callback(bus)? {
            enter_callback(cpu, frame, handler, true).map(Some)
        } else {
            self.finish(cpu);
            Ok(None)
        }
    }

    fn finish(&mut self, cpu: &mut Cpu) {
        let frame = self.frames.pop().unwrap();
        cpu.set_register(29, frame.stack);
        cpu.set_register(31, frame.return_pc);
        cpu.resume_at(frame.return_pc);
    }
}

fn enter_callback(
    cpu: &mut Cpu,
    frame: &mut Frame,
    handler: u32,
    reserve_home: bool,
) -> Result<u32> {
    if handler & 3 != 0 {
        return Err(format!("unaligned BIOS event callback 0x{handler:08X}").into());
    }
    ram_offset(handler, 4)?;
    let stack = frame
        .stack
        .checked_sub(if reserve_home { 16 } else { 0 })
        .filter(|_| frame.stack & 7 == 0)
        .ok_or("BIOS event callback requires an aligned guest stack with argument space")?;
    ram_offset(stack, 16)?;
    // Each delivery owns an O32 argument home area. The bound guest routine
    // allocates its own frame and executes all callbacks in the CPU loop.
    frame.callback_stack = Some(stack);
    frame.active = true;
    cpu.set_register(29, stack);
    cpu.set_register(31, CALLBACK_RETURN);
    cpu.resume_at(handler);
    Ok(handler)
}
