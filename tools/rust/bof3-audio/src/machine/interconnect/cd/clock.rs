//! Native FIFO, command, read and DMA callbacks at CPU branch boundaries.
use super::super::Interconnect;
use crate::{
    machine::{
        bus::Bus,
        cd_drive::{Activity, Delivery, Drive},
        cd_media::Media,
        cd_queue::Admission,
    },
    Result,
};

// durationToCycles(797us), with unit schedule scale and integer truncation.
const FIFO_CYCLES: u64 = 797 * 33_868_800 / 1_000_000;

pub(in crate::machine::interconnect) struct Controller {
    drive: Drive,
    cycle: u64,
    deadlines: [Deadline; 4],
    media: Option<Media>,
    lowest_target: u64,
    seekp_command: SeekPCommand,
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum SeekPCommand {
    Other,
    Executing,
    Cleared,
}

const FIFO: usize = 0;
const COMMAND: usize = 1;
const READ: usize = 2;
const DMA: usize = 3;

#[derive(Clone, Copy, Default)]
struct Deadline {
    target: u64,
    pending: bool,
}

impl Deadline {
    fn pending_target(self) -> Option<u64> {
        self.pending.then_some(self.target)
    }
}

impl Controller {
    fn schedule(&mut self, slot: usize, cycle: u64, delay: u64) -> Result<()> {
        let target = cycle
            .checked_add(delay)
            .ok_or("CD controller clock overflow")?;
        self.deadlines[slot] = Deadline {
            target,
            pending: true,
        };
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
    /// continuations execute at branch boundaries. ReadN/S and Play require
    /// owned media; host-wait timing is not implemented.
    pub fn configure_cd_controller(&mut self, drive: Drive, cycle: u64) -> Result<()> {
        if self.cd_controller.is_some()
            || !self.cd_host()?.idle()
            || drive.activity() != Activity::Paused
        {
            return Err(
                "CD controller requires an idle host, paused drive and no prior controller".into(),
            );
        }
        self.dma.configure_reference()?;
        self.cd_command_muted = drive.muted();
        self.cd_controller = Some(Controller {
            drive,
            cycle,
            deadlines: [Deadline::default(); 4],
            media: None,
            lowest_target: cycle,
            seekp_command: SeekPCommand::Other,
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
            .deadlines[FIFO]
            .pending_target())
    }

    pub fn cd_command_deadline(&self) -> Result<Option<u64>> {
        Ok(self
            .cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?
            .deadlines[COMMAND]
            .pending_target())
    }

    /// Attach owned raw-sector backing before any scheduled work starts.
    pub fn configure_cd_media(&mut self, media: Media) -> Result<()> {
        let controller = self
            .cd_controller
            .as_mut()
            .ok_or("CD controller is not configured")?;
        if controller.media.is_some()
            || controller.drive.activity() != Activity::Paused
            || controller.deadlines.iter().any(|slot| slot.pending)
        {
            return Err(
                "CD media requires a paused controller without media or scheduled work".into(),
            );
        }
        controller.media = Some(media);
        Ok(())
    }

    pub fn cd_read_deadline(&self) -> Result<Option<u64>> {
        Ok(self
            .cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?
            .deadlines[READ]
            .pending_target())
    }

    pub fn cd_dma_deadline(&self) -> Result<Option<u64>> {
        Ok(self
            .cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?
            .deadlines[DMA]
            .pending_target())
    }

    pub(super) fn write_clocked_cd_dma(&mut self, offset: u32, value: u32) -> Result<()> {
        let mut channel = self.dma.cd.clone();
        channel.write_reference(offset, value)?;
        if offset != 8 || !channel.active() || self.dma.priority() & (1 << 15) == 0 {
            self.dma.cd = channel;
            return Ok(());
        }
        let (address, limit, divisor) = channel.reference_transfer();
        let count = self.cd_host()?.dma_data_length(limit)?;
        if address as usize + count > 0x0080_0000 {
            return Err("DMA3: clocked transfer crosses the RAM mirror region".into());
        }
        let bytes = self.cd_mut()?.dma_data(limit)?;
        for (at, byte) in bytes.iter().enumerate() {
            self.ram.write(
                (address + at as u32) & 0x001f_ffff,
                crate::machine::bus::Width::Byte,
                u32::from(*byte),
            )?;
        }
        channel.advance_reference_registers();
        self.dma.cd = channel;
        let controller = self
            .cd_controller
            .as_mut()
            .ok_or("CD controller is not configured")?;
        controller.schedule(
            DMA,
            self.reference_access.unwrap_or(controller.cycle),
            (bytes.len() / divisor) as u64,
        )
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
            let pending = controller.deadlines.map(|slot| slot.pending);
            if !branch_boundary
                || !pending.iter().any(|&value| value)
                || controller.lowest_target >= cycle
            {
                return Ok(());
            }
            for (slot, was_pending) in pending.into_iter().enumerate() {
                if !was_pending {
                    continue;
                }
                let target = controller.deadlines[slot].target;
                if target <= cycle {
                    controller.deadlines[slot].pending = false;
                    if slot == FIFO {
                        self.service_clocked_cd_fifo(&mut controller, cycle)?;
                    } else if slot == COMMAND {
                        self.service_clocked_cd_command(&mut controller, cycle)?;
                    } else if slot == READ {
                        self.service_clocked_cd_read(&mut controller, cycle)?;
                    } else if self.dma.cd.finish_reference() {
                        self.dma.complete(3)?;
                        self.interrupts
                            .set_line(crate::machine::interrupts::Source::Dma, self.dma.irq_line());
                    }
                }
            }
            // This native build initializes signed32 lowestDistance with
            // INT64_MAX, yielding -1. No positive distance replaces the pass's
            // current-cycle target, including after a callback schedules work.
            controller.lowest_target = cycle;
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
        if !matches!(command.opcode, 0x01..=0x03 | 0x06 | 0x09..=0x11 | 0x13..=0x16 | 0x1b) {
            return Err(format!(
                "CD controller: automatic command {:#04x} is not implemented",
                command.opcode
            )
            .into());
        }
        let read = matches!(command.opcode, 0x06 | 0x1b);
        if command.opcode == 3 && controller.media.is_none() {
            return Err("CD Play requires configured media".into());
        }
        if read && command.parameters.is_empty() && controller.media.is_none() {
            return Err("CD read requires configured media".into());
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
            if command.opcode == 3 {
                let delay = if matches!(controller.drive.activity(), Activity::PreparingPlay { .. })
                {
                    20_000 * 33_868_800 / 1_000_000
                } else {
                    controller.drive.read_period_cycles()
                };
                controller.schedule(READ, cycle, delay)?;
            }
            if read {
                if controller.seekp_command == SeekPCommand::Executing {
                    controller.deadlines[COMMAND].pending = false;
                    // Queue clear retains opcode22. A selected callback still
                    // invokes it, but a later Read does not cancel it again.
                    controller.seekp_command = SeekPCommand::Cleared;
                }
                controller.schedule(READ, cycle, 20_000 * 33_868_800 / 1_000_000)?;
            }
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
                controller.seekp_command = if command.opcode == 0x16 {
                    SeekPCommand::Executing
                } else {
                    SeekPCommand::Other
                };
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
        if controller.seekp_command != SeekPCommand::Other {
            let delay = if controller.drive.status() & 0xe0 != 0x40 {
                Some(controller.drive.seekp_delay_cycles()?)
            } else {
                None
            };
            let response = controller.drive.continue_seekp();
            if let Some(delay) = delay {
                controller.schedule(COMMAND, cycle, delay)?;
            }
            if let Some(response) = response {
                self.respond_clocked_cd(response.interrupt, &response.bytes)?;
                self.schedule_clocked_cd_fifo_if_space(controller, cycle)?;
            }
            return Ok(());
        }
        if !matches!(
            controller.drive.activity(),
            Activity::PreparingSeek { .. }
                | Activity::Seeking { .. }
                | Activity::Pausing
                | Activity::Resetting
        ) {
            return Err("CD controller: command continuation has no owning transition".into());
        }
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

    fn service_clocked_cd_read(&mut self, controller: &mut Controller, cycle: u64) -> Result<()> {
        match controller.drive.activity() {
            Activity::PreparingRead { .. } | Activity::PreparingPlay { .. } => {
                let delay = controller.drive.read_seek_delay_cycles()?;
                controller.drive.complete(cycle as u32)?;
                return controller.schedule(READ, cycle, delay);
            }
            Activity::SeekingRead { .. } | Activity::SeekingPlay { .. } => {
                if let Some(response) = controller.drive.complete(cycle as u32)? {
                    return self.respond_clocked_cd(response.interrupt, &response.bytes);
                }
            }
            _ if matches!(controller.drive.status() & 0xe0, 0x20 | 0x80) => {}
            _ => return Ok(()),
        }
        if controller.drive.status() & 0xe0 == 0x80 {
            return self.service_clocked_cd_play(controller, cycle);
        }
        let raw = if controller.drive.read_media_valid() {
            Some(
                controller
                    .media
                    .as_mut()
                    .ok_or("CD read requires configured media")?
                    .read_sector(controller.drive.next_lba())?,
            )
        } else {
            None
        };
        let delivery = controller
            .drive
            .sector(raw.as_ref().map(|bytes| &bytes[..]))?;
        match delivery {
            Delivery::Error(response) => {
                self.respond_clocked_cd(response.interrupt, &response.bytes)?;
                return Ok(());
            }
            Delivery::Data { bytes, response } => {
                self.cd_mut()?.replace_read_data(&bytes)?;
                self.respond_clocked_cd(response.interrupt, &response.bytes)?;
            }
            Delivery::Xa(bytes) => {
                if matches!(self.enqueue_cd_audio(&bytes)?, Admission::Pending { .. }) {
                    return Err(
                        "CD controller: pending audio host-wait timing is not implemented".into(),
                    );
                }
            }
            Delivery::Filtered => {}
        }
        controller.schedule(READ, cycle, controller.drive.read_period_cycles())
    }

    fn service_clocked_cd_play(&mut self, controller: &mut Controller, cycle: u64) -> Result<()> {
        let raw = if let Some(lba) = controller.drive.play_media_lba()? {
            Some(
                controller
                    .media
                    .as_mut()
                    .ok_or("CD Play requires configured media")?
                    .read_sector(lba)?,
            )
        } else {
            None
        };
        let delivery = controller.drive.play_sector(
            crate::machine::cd_position::CdPosition::from_lba(controller.drive.next_lba())?,
            raw.as_ref().map(|bytes| &bytes[..]),
            cycle as u32,
        )?;
        if let Some(bytes) = delivery.audio {
            if matches!(self.enqueue_cd_cdda(&bytes)?, Admission::Pending { .. }) {
                return Err(
                    "CD controller: pending audio host-wait timing is not implemented".into(),
                );
            }
        }
        if let Some(response) = delivery.response {
            self.respond_clocked_cd(response.interrupt, &response.bytes)?;
        }
        if controller.drive.status() & 0xe0 == 0x80 {
            controller.schedule(READ, cycle, controller.drive.read_period_cycles())?;
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
        let mut value = self.cd_mut()?.read(offset)?;
        if offset == 0
            && self
                .cd_controller
                .as_ref()
                .is_some_and(|controller| controller.drive.adpcm_busy())
        {
            value |= 4;
        }
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
