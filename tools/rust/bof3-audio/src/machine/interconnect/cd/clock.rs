//! Native FIFO, command, read and DMA callbacks at CPU branch boundaries.
use super::super::{
    scheduler::Slot::{CdCommand as COMMAND, CdDma as DMA, CdFifo as FIFO, CdRead as READ},
    Interconnect,
};
use crate::{
    machine::{
        bus::Bus,
        cd_drive::{Activity, Delivery, Drive},
        cd_media::{DataSector, Media},
        cd_queue::Admission,
    },
    Result,
};

// durationToCycles(797us), with unit schedule scale and integer truncation.
const FIFO_CYCLES: u64 = 797 * 33_868_800 / 1_000_000;

pub(in crate::machine::interconnect) struct Controller {
    drive: Drive,
    media: Option<Media>,
    seekp_command: SeekPCommand,
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum SeekPCommand {
    Other,
    Executing,
    Cleared,
}

impl Interconnect {
    /// Attach an explicit paused drive and unit-scale native controller clock.
    /// The selected cycle is the caller's CPU phase. FIFO and Pause/Init/Seek
    /// continuations execute at branch boundaries. ReadN/S and Play require
    /// owned media; host-wait timing is not implemented.
    pub fn configure_cd_controller(&mut self, drive: Drive, cycle: u64) -> Result<()> {
        if self.cd_controller.is_some()
            || self.dma.cd.active()
            || !self.cd_host()?.idle()
            || drive.activity() != Activity::Paused
        {
            return Err(
                "CD controller requires an idle host, paused drive and no prior controller".into(),
            );
        }
        self.scheduler.attach(cycle)?;
        self.cd_command_muted = drive.muted();
        self.cd_controller = Some(Controller {
            drive,
            media: None,
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
        self.cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?;
        Ok(self.scheduler.deadline(FIFO))
    }

    pub fn cd_command_deadline(&self) -> Result<Option<u64>> {
        self.cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?;
        Ok(self.scheduler.deadline(COMMAND))
    }

    /// Attach owned raw-sector backing before any scheduled work starts.
    pub fn configure_cd_media(&mut self, media: Media) -> Result<()> {
        let controller = self
            .cd_controller
            .as_mut()
            .ok_or("CD controller is not configured")?;
        if controller.media.is_some()
            || controller.drive.activity() != Activity::Paused
            || [FIFO, COMMAND, READ, DMA]
                .into_iter()
                .any(|slot| self.scheduler.deadline(slot).is_some())
        {
            return Err(
                "CD media requires a paused controller without media or scheduled work".into(),
            );
        }
        controller.media = Some(media);
        Ok(())
    }

    pub fn cd_read_deadline(&self) -> Result<Option<u64>> {
        self.cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?;
        Ok(self.scheduler.deadline(READ))
    }

    pub fn cd_dma_deadline(&self) -> Result<Option<u64>> {
        self.cd_controller
            .as_ref()
            .ok_or("CD controller is not configured")?;
        Ok(self.scheduler.deadline(DMA))
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
            let destination = address + at as u32;
            let physical = destination & 0x001f_ffff;
            self.ram
                .write(physical, crate::machine::bus::Width::Byte, u32::from(*byte))?;
            if let Some(observation) = &mut self.dma_observation {
                observation.record(crate::machine::dma::RamAccess {
                    channel: 3,
                    width: crate::machine::bus::Width::Byte,
                    address: destination,
                    physical_address: physical,
                    value: u32::from(*byte),
                    from_ram: false,
                });
            }
        }
        channel.advance_reference_registers();
        self.dma.cd = channel;
        self.scheduler.schedule(
            DMA,
            self.reference_access.unwrap_or(self.scheduler.cycle()),
            (bytes.len() / divisor) as u64,
        )
    }

    pub(in crate::machine::interconnect) fn dispatch_cd(
        &mut self,
        slot: super::super::scheduler::Slot,
        cycle: u64,
    ) -> Result<()> {
        let mut controller = self
            .cd_controller
            .take()
            .ok_or("scheduled CD callback has no controller")?;
        let result = match slot {
            FIFO => self.service_clocked_cd_fifo(&mut controller, cycle),
            COMMAND => self.service_clocked_cd_command(&mut controller, cycle),
            READ => self.service_clocked_cd_read(&mut controller, cycle),
            _ => Err("invalid CD controller callback".into()),
        };
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
                self.scheduler.schedule(READ, cycle, delay)?;
            }
            if read {
                if controller.seekp_command == SeekPCommand::Executing {
                    self.scheduler.cancel(COMMAND);
                    // Queue clear retains opcode22. A selected callback still
                    // invokes it, but a later Read does not cancel it again.
                    controller.seekp_command = SeekPCommand::Cleared;
                }
                self.scheduler
                    .schedule(READ, cycle, 20_000 * 33_868_800 / 1_000_000)?;
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
                return self
                    .scheduler
                    .schedule(COMMAND, cycle, micros * 33_868_800 / 1_000_000);
            }
        }
        self.schedule_clocked_cd_fifo_if_space(cycle)
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
                self.scheduler.schedule(COMMAND, cycle, delay)?;
            }
            if let Some(response) = response {
                self.respond_clocked_cd(response.interrupt, &response.bytes)?;
                self.schedule_clocked_cd_fifo_if_space(cycle)?;
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
            self.scheduler.schedule(COMMAND, cycle, delay)?;
        }
        if let Some(response) = response {
            self.respond_clocked_cd(response.interrupt, &response.bytes)?;
            self.schedule_clocked_cd_fifo_if_space(cycle)?;
        }
        Ok(())
    }

    fn service_clocked_cd_read(&mut self, controller: &mut Controller, cycle: u64) -> Result<()> {
        match controller.drive.activity() {
            Activity::PreparingRead { .. } | Activity::PreparingPlay { .. } => {
                let delay = controller.drive.read_seek_delay_cycles()?;
                controller.drive.complete(cycle as u32)?;
                return self.scheduler.schedule(READ, cycle, delay);
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
                    .read_data_sector(controller.drive.next_lba())?,
            )
        } else {
            None
        };
        let delivery = match raw.as_ref() {
            Some(DataSector::Complete(bytes)) => controller.drive.sector(Some(bytes))?,
            Some(DataSector::Retained(bytes)) => controller.drive.retained_sector(bytes)?,
            None => controller.drive.sector(None)?,
        };
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
                if matches!(
                    self.enqueue_cd_audio(&bytes, cycle)?,
                    Admission::Pending { .. }
                ) {
                    return Err(
                        "CD controller: pending audio host-wait timing is not implemented".into(),
                    );
                }
            }
            Delivery::Filtered => {}
        }
        self.scheduler
            .schedule(READ, cycle, controller.drive.read_period_cycles())
    }

    fn service_clocked_cd_play(&mut self, controller: &mut Controller, cycle: u64) -> Result<()> {
        let raw = if let Some(lba) = controller.drive.play_media_lba()? {
            Some(
                controller
                    .media
                    .as_mut()
                    .ok_or("CD Play requires configured media")?
                    .read_audio_sector(lba)?,
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
            if matches!(
                self.enqueue_cd_cdda(&bytes, cycle)?,
                Admission::Pending { .. }
            ) {
                return Err(
                    "CD controller: pending audio host-wait timing is not implemented".into(),
                );
            }
        }
        if let Some(response) = delivery.response {
            self.respond_clocked_cd(response.interrupt, &response.bytes)?;
        }
        if controller.drive.status() & 0xe0 == 0x80 {
            self.scheduler
                .schedule(READ, cycle, controller.drive.read_period_cycles())?;
        }
        Ok(())
    }

    fn schedule_clocked_cd_fifo_if_space(&mut self, cycle: u64) -> Result<()> {
        if !self.cd_host()?.response_full() {
            self.scheduler.schedule(FIFO, cycle, FIFO_CYCLES)?;
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
        if self.cd_controller.is_some() {
            self.scheduler.schedule(
                FIFO,
                self.reference_access.unwrap_or(self.scheduler.cycle()),
                FIFO_CYCLES,
            )?;
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
