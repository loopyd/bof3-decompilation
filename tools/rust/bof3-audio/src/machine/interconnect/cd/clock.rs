//! Native FIFO deadlines and immediate commands at CPU branch boundaries.
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
    fifo: Option<u64>,
    lowest_target: u64,
}

impl Controller {
    fn schedule_fifo(&mut self, cycle: u64) -> Result<()> {
        let target = cycle
            .checked_add(FIFO_CYCLES)
            .ok_or("CD FIFO clock overflow")?;
        self.fifo = Some(target);
        self.lowest_target = self.lowest_target.min(target);
        Ok(())
    }

    fn dispatch_fifo(&mut self, cycle: u64, branch_boundary: bool) -> bool {
        if !branch_boundary || self.fifo.is_none() || self.lowest_target >= cycle {
            return false;
        }
        // Native branchTest snapshots the pending mask, tests the current target
        // and recomputes its minimum from future targets in that snapshot.
        let target = self.fifo.unwrap();
        if target > cycle {
            self.lowest_target = target;
            return false;
        }
        self.fifo = None;
        self.lowest_target = cycle;
        true
    }
}

impl Interconnect {
    /// Attach an explicit paused drive and unit-scale native FIFO clock. The
    /// selected cycle is the caller's CPU phase. Immediate commands execute at
    /// branch boundaries; mechanical and media commands fail explicitly.
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
            fifo: None,
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
            .fifo)
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
            if !controller.dispatch_fifo(cycle, branch_boundary) {
                return Ok(());
            }
            let command = self.cd_mut()?.service_fifo();
            self.update_cd_irq();
            let Some(command) = command else {
                return Ok(());
            };
            if !matches!(command.opcode, 0x01 | 0x02 | 0x0b..=0x11 | 0x13 | 0x14) {
                return Err(format!(
                    "CD controller: automatic command {:#04x} is not implemented",
                    command.opcode
                )
                .into());
            }
            let applied =
                self.apply_cd_drive_command(&mut controller.drive, &command, cycle as u32)?;
            self.cd_mut()?
                .respond(applied.response.interrupt, &applied.response.bytes)?;
            self.update_cd_irq();
            // Only completed command handlers request the next FIFO callback.
            // Read/data response insertion alone must not install a deadline.
            if !self.cd_host()?.response_full() {
                controller.schedule_fifo(cycle)?;
            }
            Ok(())
        })();
        self.cd_controller = Some(controller);
        result
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
        if index == 0 && offset == 1 && !pending {
            self.schedule_cd_fifo()?;
        } else if index == 1 && offset == 3 && matches!(value, 7 | 0x1f) {
            self.schedule_cd_fifo_if_space()?;
        }
        Ok(())
    }
}
