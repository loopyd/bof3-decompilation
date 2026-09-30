//! HLE DeliverEvent continuations. Guest handlers execute in the normal CPU loop.
use super::{
    bus::Bus,
    cpu::Cpu,
    events::{Delivery, Table},
    executable::ram_offset,
};
use crate::Result;

// Reserved HLE return trap, not original BIOS code or an emulated TLB vector.
pub(crate) const CALLBACK_RETURN: u32 = 0xbfc0_0100;
const SAVED: [usize; 9] = [16, 17, 18, 19, 20, 21, 22, 23, 30];
const MAX_DEPTH: usize = 64;

#[derive(Clone, Debug)]
struct Frame {
    delivery: Delivery,
    return_pc: u32,
    stack: u32,
    saved: [u32; 9],
    callback_stack: Option<u32>,
    active: bool,
}

#[derive(Clone, Debug, Default)]
pub(crate) struct Calls {
    frames: Vec<Frame>,
}

impl Calls {
    pub fn depth(&self) -> usize {
        self.frames.len()
    }

    pub fn start(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        if self.frames.len() >= MAX_DEPTH {
            return Err("BIOS DeliverEvent: callback nesting limit (64) exceeded".into());
        }
        let delivery = Table::read(bus)?.delivery(cpu.register(4), cpu.register(5));
        self.frames.push(Frame {
            delivery,
            return_pc: cpu.register(31),
            stack: cpu.register(29),
            saved: SAVED.map(|register| cpu.register(register)),
            callback_stack: None,
            active: false,
        });
        self.advance(cpu, bus)
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
        self.advance(cpu, bus)
    }

    fn advance(&mut self, cpu: &mut Cpu, bus: &mut impl Bus) -> Result<Option<u32>> {
        let frame = self
            .frames
            .last_mut()
            .ok_or("missing BIOS event continuation")?;
        if let Some(handler) = frame.delivery.next_callback(bus)? {
            if handler & 3 != 0 {
                return Err(format!("unaligned BIOS event callback 0x{handler:08X}").into());
            }
            ram_offset(handler, 4)?;
            let stack = frame
                .stack
                .checked_sub(16)
                .filter(|_| frame.stack & 7 == 0)
                .ok_or("BIOS event callback requires an aligned guest stack with argument space")?;
            ram_offset(stack, 16)?;
            // Each HLE delivery owns a separate O32 argument home area. Exact
            // original BIOS frame layout and volatile argument values are unverified.
            frame.callback_stack = Some(stack);
            frame.active = true;
            cpu.set_register(29, stack);
            cpu.set_register(31, CALLBACK_RETURN);
            cpu.resume_at(handler);
            Ok(Some(handler))
        } else {
            let frame = self.frames.pop().unwrap();
            cpu.set_register(29, frame.stack);
            cpu.set_register(31, frame.return_pc);
            cpu.resume_at(frame.return_pc);
            Ok(None)
        }
    }
}
