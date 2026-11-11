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
    pub start: u32,
    pub enter: u32,
    pub returned: u32,
    pub end: u32,
    pub arguments: u32,
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
        let cold = execution.cpu.pc() == guest.start;
        if cold {
            if entry != guest.start || arguments != [0; 4] || stack_arguments.is_some() {
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
        for (i, value) in std::iter::once(entry)
            .chain(arguments)
            .chain(stack_arguments.unwrap_or([0; 4]))
            .enumerate()
        {
            execution
                .bus
                .write(guest.arguments + i as u32 * 4, Width::Word, value)?;
        }
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
