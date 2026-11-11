use super::{Execution, RETURN};
use crate::{
    machine::{
        bus::{Bus, Width},
        image::executable::ram_offset,
    },
    Result,
};

#[derive(Clone, Copy)]
pub(crate) struct Guest {
    pub reset: u32,
    pub start: u32,
    pub enter: u32,
    pub returned: u32,
    pub end: u32,
    pub arguments: u32,
}

impl Execution {
    pub(crate) fn copy_input(
        &mut self,
        entry: u32,
        address: u32,
        bytes: &[u8],
        observer: &mut impl FnMut(&Self) -> Result<()>,
    ) -> Result<Option<super::Call>> {
        ram_offset(address, bytes.len())?;
        if bytes.is_empty() {
            return Ok(None);
        }
        if bytes.len() > crate::machine::inputs::MEDIA_BYTES {
            return Err("owned guest copy exceeds media input capacity".into());
        }
        self.stage_media_input(bytes)?;
        let copied = self.call_observed(
            entry,
            [
                address,
                crate::machine::inputs::MEDIA,
                bytes.len() as u32,
                0,
            ],
            1_000_000,
            observer,
        )?;
        if copied.result != address {
            return Err("owned guest copy returned a different destination".into());
        }
        Ok(Some(copied))
    }
}

pub(super) struct Boundary {
    pub returned: u32,
    pub resuming: bool,
    guest_stack: Option<u32>,
    host_stack: Option<u32>,
}

impl Boundary {
    pub fn require_stack(&self, execution: &Execution) -> Result<()> {
        if self
            .guest_stack
            .is_some_and(|sp| execution.cpu.register(29) != sp)
        {
            return Err("guest call did not restore its outgoing argument frame".into());
        }
        Ok(())
    }

    pub fn restore_host_stack(&self, execution: &mut Execution) {
        if let Some(sp) = self.host_stack {
            execution.cpu.set_register(29, sp);
        }
    }
}

fn frame(execution: &Execution) -> Result<u32> {
    let sp = execution.cpu.register(29);
    if sp & 7 != 0 {
        return Err("guest call requires an aligned o32 argument stack".into());
    }
    let at = sp.checked_sub(32).ok_or("guest argument stack underflow")?;
    ram_offset(at, 32)?;
    Ok(at)
}

pub(super) fn prepare(
    execution: &mut Execution,
    entry: u32,
    arguments: [u32; 4],
    stack_arguments: Option<[u32; 4]>,
) -> Result<Boundary> {
    if let Some(guest) = execution.calls {
        let cold = execution.cpu.pc() == guest.reset && execution.cpu.instructions() == 0;
        if cold {
            if entry != guest.reset || arguments != [0; 4] || stack_arguments.is_some() {
                return Err("owned cold startup requires its entry and no arguments".into());
            }
            return Ok(Boundary {
                returned: guest.returned,
                resuming: false,
                guest_stack: None,
                host_stack: None,
            });
        }
        if execution.cpu.pc() != guest.returned {
            return Err("finish the owned dispatcher before admitting another call".into());
        }
        if ((guest.start & 0x1fff_ffff)..(guest.end & 0x1fff_ffff)).contains(&(entry & 0x1fff_ffff))
        {
            return Err("owned call cannot target its dispatcher".into());
        }
        frame(execution)?;
        let mut words = [0; 9];
        words[0] = entry;
        words[1..5].copy_from_slice(&arguments);
        words[5..].copy_from_slice(&stack_arguments.unwrap_or([0; 4]));
        execution
            .bus
            .inputs
            .as_mut()
            .ok_or("owned input ABI is not configured")?
            .command(words)?;
        return Ok(Boundary {
            returned: guest.returned,
            resuming: true,
            guest_stack: Some(execution.cpu.register(29)),
            host_stack: None,
        });
    }

    // Caller-loaded development execution retains explicit host entry. Its
    // eight-argument bridge preserves the existing stack restoration on errors.
    let mut host_stack = None;
    if let Some(values) = stack_arguments {
        let at = frame(execution)?;
        for (i, value) in values.into_iter().enumerate() {
            execution
                .bus
                .write(at + 16 + i as u32 * 4, Width::Word, value)?;
        }
        host_stack = Some(execution.cpu.register(29));
        execution.cpu.set_register(29, at);
    }
    execution.cpu.resume_at(entry);
    for (i, value) in arguments.into_iter().enumerate() {
        execution.cpu.set_register(4 + i, value);
    }
    execution.cpu.set_register(31, RETURN);
    Ok(Boundary {
        returned: RETURN,
        resuming: false,
        guest_stack: None,
        host_stack,
    })
}
