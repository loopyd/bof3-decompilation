//! Native FIFO and command continuations at CPU branch boundaries.
use super::super::Interconnect;
use crate::{
    machine::cd_drive::{Activity, Drive},
    Result,
};

// durationToCycles(797us), with unit schedule scale and integer truncation.
const FIFO_CYCLES: u64 = 797 * 33_868_800 / 1_000_000;

pub(in crate::machine::interconnect) struct Controller {
    drive: Drive,
    cycle: u64,
    deadlines: [Option<u64>; 2],
    lowest_target: u64,
}

const FIFO: usize = 0;
const COMMAND: usize = 1;

impl Controller {
    fn schedule(&mut self, slot: usize, cycle: u64, delay: u64) -> Result<()> {
        let target = cycle
            .checked_add(delay)
            .ok_or("CD controller clock overflow")?;
        self.deadlines[slot] = Some(target);
        self.lowest_target = self.lowest_target.min(target);
        Ok(())
    }

    fn schedule_fifo(&mut self, cycle: u64) -> Result<()> {
        self.schedule(FIFO, cycle, FIFO_CYCLES)
    }
}

impl Interconnect {
    /// Attach an explicit paused drive and unit-scale native controller clock.
    /// The selected cycle is the caller's CPU phase. FIFO and Pause/Init/Seek
    /// continuations execute at branch boundaries; media commands fail explicitly.
    pub fn configure_cd_controller(&mut self, drive: Drive, cycle: u64) -> Result<()> {
        if self.cd_controller.is_some()
            || !self.cd_host()?.idle()
            || drive.activity() != Activity::Paused
        {
            return Err(
                "CD controller requires an idle host, paused drive and no prior controller".into(),
            );
        }
        self.cd_command_muted = drive.muted();
        self.cd_controller = Some(Controller {
            drive,
            cycle,
            deadlines: [None; 2],
            lowest_target: cycle,
        });
        Ok(())
    }

    pub fn cd_controller_drive(&self) -> Result<&Drive> {
        Ok(&self
            .cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?
            .drive)
    }

    pub fn cd_fifo_deadline(&self) -> Result<Option<u64>> {
        Ok(self
            .cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?
            .deadlines[FIFO])
    }

    pub fn cd_command_deadline(&self) -> Result<Option<u64>> {
        Ok(self
            .cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?
            .deadlines[COMMAND])
    }

    /// Advance the attached controller's absolute cycle. Dispatch only after a
    /// retired branch delay slot. A failed callback is terminal; accepted host
    /// work is not rolled back. An unattached controller has no scheduled work.
    pub fn advance_cd_controller(&mut self, cycle: u64, branch_boundary: bool) -> Result<()> {
        let Some(mut controller) = self.cd_controller.take() else {
            return Ok(());
        };
        let result = (|| {
            if cycle < controller.cycle {
                return Err("CD controller cycle moved backwards".into());
            }
            controller.cycle = cycle;
            // Snapshot only pending slots. FIFO can replace a continuation's
            // current target before that slot is visited in the same pass.
            let pending = controller.deadlines.map(|target| target.is_some());
            if !branch_boundary
                || !pending.iter().any(|&value| value)
                || controller.lowest_target >= cycle
            {
                return Ok(());
            }
            let mut lowest_target = cycle;
            let mut future_target: Option<u64> = None;
            for (slot, was_pending) in pending.into_iter().enumerate() {
                if !was_pending {
                    continue;
                }
                let target = controller.deadlines[slot].unwrap();
                if target > cycle {
                    future_target =
                        Some(future_target.map_or(target, |previous| previous.min(target)));
                } else {
                    controller.deadlines[slot] = None;
                    if slot == FIFO {
                        self.service_clocked_cd_fifo(&mut controller, cycle)?;
                    } else {
                        self.service_clocked_cd_command(&mut controller, cycle)?;
                    }
                }
            }
            if let Some(target) = future_target {
                lowest_target = target;
            }
            // Native branchTest overwrites schedule()'s minimum with the future
            // targets encountered in its original pending-slot snapshot.
            controller.lowest_target = lowest_target;
            Ok(())
        })();
        self.cd_controller = Some(controller);
        result
    }

    fn assert_clocked_cd_irq(&mut self) {
        // Native setIRQ is an assertion event, including repeated mask writes.
        // Other devices and the manual CD path retain their line semantics.
        if self
            .cd_host
            .as_ref()
            .is_some_and(crate::machine::cd_host::Host::irq_line)
        {
            self.interrupts
                .set_line(crate::machine::interrupts::Source::Cdrom, false);
            self.interrupts
                .set_line(crate::machine::interrupts::Source::Cdrom, true);
        }
    }

    fn respond_clocked_cd(&mut self, interrupt: u8, bytes: &[u8]) -> Result<()> {
        if self.cd_mut()?.respond(interrupt, bytes)? {
            self.assert_clocked_cd_irq();
        }
        Ok(())
    }

    fn service_clocked_cd_fifo(&mut self, controller: &mut Controller, cycle: u64) -> Result<()> {
        let was_asserted = self.cd_host()?.irq_line();
        let command = self.cd_mut()?.service_fifo();
        if !was_asserted {
            self.assert_clocked_cd_irq();
        }
        let Some(command) = command else {
            return Ok(());
        };
        if !matches!(command.opcode, 0x01 | 0x02 | 0x09..=0x11 | 0x13..=0x16) {
            return Err(format!(
                "CD controller: automatic command {:#04x} is not implemented",
                command.opcode
            )
            .into());
        }
        let pause_delay = if controller.drive.status() & 0xe0 == 0 {
            200
        } else if controller.drive.double_speed() {
            35_000
        } else {
            70_000
        };
        let applied = self.apply_cd_drive_command(&mut controller.drive, &command, cycle as u32)?;
        self.respond_clocked_cd(applied.response.interrupt, &applied.response.bytes)?;
        if applied.response.interrupt == 3 {
            let delay = match command.opcode {
                0x09 => Some(pause_delay),
                0x0a => {
                    // The acknowledgement uses the old mask. Resetting the mask
                    // afterwards does not itself assert the interrupt.
                    self.cd_mut()?.init_interrupt_mask();
                    Some(120_000)
                }
                0x15 | 0x16 => Some(15_000),
                _ => None,
            };
            if let Some(micros) = delay {
                return controller.schedule(COMMAND, cycle, micros * 33_868_800 / 1_000_000);
            }
        }
        self.schedule_clocked_cd_fifo_if_space(controller, cycle)
    }

    fn service_clocked_cd_command(
        &mut self,
        controller: &mut Controller,
        cycle: u64,
    ) -> Result<()> {
        let seek_delay = if matches!(controller.drive.activity(), Activity::PreparingSeek { .. }) {
            Some(controller.drive.seek_delay_cycles()?)
        } else {
            None
        };
        let response = controller.drive.complete(cycle as u32)?;
        if let Some(delay) = seek_delay {
            controller.schedule(COMMAND, cycle, delay)?;
        }
        if let Some(response) = response {
            self.respond_clocked_cd(response.interrupt, &response.bytes)?;
            self.schedule_clocked_cd_fifo_if_space(controller, cycle)?;
        }
        Ok(())
    }

    fn schedule_clocked_cd_fifo_if_space(
        &self,
        controller: &mut Controller,
        cycle: u64,
    ) -> Result<()> {
        if !self.cd_host()?.response_full() {
            controller.schedule_fifo(cycle)?;
        }
        Ok(())
    }

    pub(super) fn require_manual_cd(&self) -> Result<()> {
        if self.cd_controller.is_some() {
            return Err("CD controller owns command acceptance and responses".into());
        }
        Ok(())
    }

    fn schedule_cd_fifo(&mut self) -> Result<()> {
        if let Some(controller) = &mut self.cd_controller {
            controller.schedule_fifo(self.reference_access.unwrap_or(controller.cycle))?;
        }
        Ok(())
    }

    fn schedule_cd_fifo_if_space(&mut self) -> Result<()> {
        if !self.cd_host()?.response_full() {
            self.schedule_cd_fifo()?;
        }
        Ok(())
    }

    pub(super) fn read_cd_register(&mut self, offset: u8) -> Result<u8> {
        let value = self.cd_mut()?.read(offset)?;
        if offset == 1 {
            self.schedule_cd_fifo_if_space()?;
        }
        Ok(value)
    }

    pub(super) fn write_cd_register(&mut self, offset: u8, value: u8) -> Result<()> {
        let index = self.cd_host()?.index();
        let pending = self.cd_host()?.busy();
        if self.cd_controller.is_some()
            && index == 1
            && offset == 3
            && value & 0x1f != 0
            && !matches!(value, 7 | 0x1f)
        {
            return Err("clocked CD requires a full IRQ acknowledgement".into());
        }
        self.cd_mut()?.write(offset, value)?;
        if self.cd_controller.is_some() && index == 1 && offset == 2 {
            self.assert_clocked_cd_irq();
        }
        if index == 0 && offset == 1 && !pending {
            self.schedule_cd_fifo()?;
        } else if index == 1 && offset == 3 && matches!(value, 7 | 0x1f) {
            self.schedule_cd_fifo_if_space()?;
        }
        Ok(())
    }
}
