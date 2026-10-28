//! CD host/data/DMA3 wiring; service calls are transactions, not elapsed clocks.
use super::{error, Interconnect};
use crate::machine::{
    bus::{Bus, BusError, Width},
    cd_data,
    cd_drive::{Activity, AppliedCommand, Drive},
    cd_host,
    interrupts::Source,
};

impl Interconnect {
    /// Attach a fresh, explicitly configured XA context once. Predictor resets
    /// and command mute are coupled through apply_cd_drive_command.
    pub fn configure_cd_audio(
        &mut self,
        queue: crate::machine::cd_queue::Queue,
    ) -> crate::Result<()> {
        if self.cd_audio.is_some() {
            return Err("CD audio already configured".into());
        }
        self.cd_host()?;
        self.cd_audio = Some(queue);
        Ok(())
    }
    pub fn cd_audio(&self) -> crate::Result<&crate::machine::cd_queue::Queue> {
        self.cd_audio
            .as_ref()
            .ok_or_else(|| "CD audio is not configured".into())
    }
    pub fn enqueue_cd_audio(
        &mut self,
        sector: &[u8],
    ) -> crate::Result<crate::machine::cd_queue::Admission> {
        let host = self.cd_host.as_ref().ok_or("CD host is not configured")?;
        self.cd_audio
            .as_mut()
            .ok_or("CD audio is not configured")?
            .sector(sector, self.cd_command_muted, host.volume())
    }
    /// Submit one already-selected raw CDDA sector; drive selection is external.
    pub fn enqueue_cd_cdda(
        &mut self,
        sector: &[u8],
    ) -> crate::Result<crate::machine::cd_queue::Admission> {
        let host = self.cd_host.as_ref().ok_or("CD host is not configured")?;
        self.cd_audio
            .as_mut()
            .ok_or("CD audio is not configured")?
            .cdda_sector(sector, self.cd_command_muted, host.volume())
    }
    /// Complete a pending XA/CDDA submission after supplied output/wait boundaries.
    pub fn finish_cd_audio_submission(
        &mut self,
        outcome: crate::machine::cd_queue::WaitOutcome,
    ) -> crate::Result<crate::machine::cd_queue::Admission> {
        self.cd_audio
            .as_mut()
            .ok_or("CD audio is not configured")?
            .finish_submission(outcome)
    }
    /// One supplied SPU frame boundary. Drive gain and source mute were applied
    /// at sector admission. Failed SPU output retains the
    /// queued frame; the existing SPU engine may have advanced internal state.
    pub fn step_spu_cd_output(&mut self, external: [i16; 2]) -> crate::Result<[i16; 2]> {
        let cd = self.cd_audio()?.preview();
        let output = self.step_spu_output(super::SpuInputs { cd, external })?;
        self.cd_audio.as_mut().unwrap().consume();
        Ok(output)
    }
    /// Apply a captured command and its decoder/buffer effects at one supplied
    /// MCU boundary. Response publication and mechanical completion stay with
    /// the caller. Failed/unsupported commands leave drive and devices intact.
    /// A pending sector submission must finish first. Seek/Init during active
    /// DMA3 remain unsupported by this serialized service path.
    pub fn apply_cd_drive_command(
        &mut self,
        drive: &mut Drive,
        command: &cd_host::Command,
        cycle: u32,
    ) -> crate::Result<AppliedCommand> {
        self.cd_host()?;
        if self
            .cd_audio
            .as_ref()
            .is_some_and(|queue| queue.statistics().pending_frames != 0)
        {
            return Err(
                "CD drive: command during pending audio submission is not implemented".into(),
            );
        }
        let mut candidate = drive.clone();
        let response = candidate.command(command, cycle)?;
        let seek_reset = matches!(
            candidate.activity(),
            Activity::PreparingSeek { .. }
                | Activity::Seeking { .. }
                | Activity::PreparingRead { .. }
                | Activity::SeekingRead { .. }
        ) && candidate.activity() != drive.activity();
        let play_seek = matches!(
            candidate.activity(),
            Activity::PreparingPlay { .. } | Activity::SeekingPlay { .. }
        ) && candidate.activity() != drive.activity();
        let xa_predictors_reset = response.interrupt == 3
            && (matches!(command.opcode, 0x06 | 0x1b)
                || (command.opcode == 0x0e
                    && drive.mode() & 0x40 == 0
                    && candidate.mode() & 0x40 != 0));
        let audio_selection_released = seek_reset
            || xa_predictors_reset
            || (matches!(command.opcode, 0x09 | 0x0a | 0x0d) && response.interrupt == 3);
        let init = command.opcode == 0x0a && response.interrupt == 3;
        if (seek_reset || play_seek || init) && self.dma.cd.active() {
            return Err("CD drive: seek/reset during active DMA3 is not implemented".into());
        }
        if xa_predictors_reset {
            if let Some(queue) = self.cd_audio.as_mut() {
                queue.reset_predictors()?;
            }
        }
        self.cd_command_muted = candidate.muted();
        if audio_selection_released {
            if let Some(queue) = self.cd_audio.as_mut() {
                queue.release_selection();
            }
        }
        *drive = candidate;
        Ok(AppliedCommand {
            response,
            xa_predictors_reset,
            audio_selection_released,
        })
    }
    pub fn configure_cd_host(&mut self, volume: [u8; 4]) -> crate::Result<()> {
        if self.cd_host.is_some() {
            return Err("CD host already configured".into());
        }
        self.cd_host = Some(cd_host::Host::new(volume)?);
        Ok(())
    }
    pub fn configure_cd_host_reset(&mut self, model: cd_host::Model) -> crate::Result<()> {
        if self.cd_host.is_some() {
            return Err("CD host already configured".into());
        }
        self.cd_host = Some(cd_host::Host::from_reset(model)?);
        Ok(())
    }
    pub fn cd_host(&self) -> crate::Result<&cd_host::Host> {
        self.cd_host
            .as_ref()
            .ok_or_else(|| "CD host is not configured".into())
    }
    fn cd_mut(&mut self) -> crate::Result<&mut cd_host::Host> {
        self.cd_host
            .as_mut()
            .ok_or_else(|| "CD host is not configured".into())
    }
    pub fn configure_cd_data(&mut self, model: cd_data::Model) -> crate::Result<()> {
        self.cd_mut()?.configure_data(model)
    }
    pub fn present_cd_data(&mut self, bytes: &[u8]) -> crate::Result<()> {
        self.cd_mut()?.present_data(bytes)
    }
    pub fn deliver_cd_data(&mut self, status: u8, bytes: &[u8]) -> crate::Result<usize> {
        let displaced = self.cd_mut()?.deliver_data(status, bytes)?;
        self.update_cd_irq();
        Ok(displaced)
    }
    pub fn take_cd_command(&mut self) -> crate::Result<Option<cd_host::Command>> {
        Ok(self.cd_mut()?.take_command())
    }
    pub fn respond_cd(&mut self, interrupt: u8, bytes: &[u8]) -> crate::Result<()> {
        self.cd_mut()?.respond(interrupt, bytes)?;
        self.update_cd_irq();
        Ok(())
    }
    pub fn stage_cd_response(&mut self, bytes: &[u8]) -> crate::Result<()> {
        self.cd_mut()?.stage_response(bytes)
    }
    pub fn raise_cd_interrupt(&mut self, interrupt: u8) -> crate::Result<()> {
        self.cd_mut()?.raise_interrupt(interrupt)?;
        self.update_cd_irq();
        Ok(())
    }
    fn update_cd_irq(&mut self) {
        let active = self.cd_host.as_ref().is_some_and(cd_host::Host::irq_line);
        self.interrupts.set_line(Source::Cdrom, active);
    }

    /// Bounded DMA3 word service. Does not advance clocks, arbitrate channels or
    /// stall the CPU. The drive owner separately publishes each sector block.
    pub fn service_cd(&mut self, word_budget: usize) -> crate::Result<usize> {
        if self.dma.priority() & (1 << 15) == 0 || !self.dma.cd.active() {
            return Ok(0);
        }
        if self.cd_bus_control != 0x0002_0943 || self.common_delay != 0x1323 {
            return Err(
                "DMA3 requires the verified game DEV5_CTRL/COM_DELAY transfer configuration".into(),
            );
        }
        let mut words = 0;
        while words < word_budget {
            let requested = self.cd_host()?.data_ready();
            let Some(address) = self.dma.cd.next_word(requested) else {
                break;
            };
            if address >= 0x0080_0000 {
                self.dma.bus_error();
                self.interrupts.set_line(Source::Dma, self.dma.irq_line());
                return Err(format!(
                    "DMA3 address {address:#08x} is outside the RAM mirror region"
                )
                .into());
            }
            let word = self.cd_mut()?.read_data(4)?;
            self.ram.write(address & 0x001f_fffc, Width::Word, word)?;
            words += 1;
            if self.dma.cd.finish_word()? {
                self.dma.complete(3)?;
                self.interrupts.set_line(Source::Dma, self.dma.irq_line());
            }
        }
        Ok(words)
    }

    pub(super) fn is_cd_port(physical: u32) -> bool {
        (0x1f80_1800..0x1f80_1804).contains(&physical)
            || (0x1f80_10b0..0x1f80_10bc).contains(&physical)
            || matches!(physical, 0x1f80_1018 | 0x1f80_1020)
    }
    pub(super) fn read_cd_port(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        let physical = Self::physical(address);
        let result = if physical >= 0x1f80_1800 {
            if physical == 0x1f80_1802 && width == Width::Half {
                self.cd_mut().and_then(|h| h.read_data(2))
            } else if width == Width::Byte {
                self.cd_mut()
                    .and_then(|h| h.read((physical & 3) as u8).map(u32::from))
            } else {
                return Err(error(
                    address,
                    "CD registers require bytes; data port also supports halfword reads",
                ));
            }
        } else if physical >= 0x1f80_10b0 {
            Self::require_word_port(address, width)?;
            self.dma
                .cd
                .read(physical & 15)
                .map(|v| if width == Width::Half { v & 0xffff } else { v })
        } else {
            if width != Width::Word {
                return Err(error(address, "CD bus controls require word accesses"));
            }
            Ok(if physical == 0x1f80_1018 {
                self.cd_bus_control
            } else {
                self.common_delay
            })
        };
        result.map_err(|e| error(address, &e.to_string()))
    }
    pub(super) fn write_cd_port(
        &mut self,
        address: u32,
        width: Width,
        value: u32,
    ) -> Result<(), BusError> {
        let physical = Self::physical(address);
        let result = if physical >= 0x1f80_1800 {
            if width != Width::Byte {
                return Err(error(address, "CD host requires byte register writes"));
            }
            self.cd_mut()
                .and_then(|h| h.write((physical & 3) as u8, value as u8))
        } else if physical >= 0x1f80_10b0 {
            Self::require_word_port(address, width)?;
            self.dma.cd.write(physical & 15, value)
        } else {
            if width != Width::Word {
                return Err(error(address, "CD bus controls require word accesses"));
            }
            match (physical, value) {
                (0x1f80_1018, 0x0002_0843 | 0x0002_0943) => self.cd_bus_control = value,
                (0x1f80_1020, 0x1323 | 0x1325 | 0x1125 | 0x31125) => {
                    self.common_delay = value & 0xffff;
                }
                _ => return Err(error(address, "unverified CD bus-control configuration")),
            }
            Ok(())
        };
        result.map_err(|e| error(address, &e.to_string()))?;
        self.update_cd_irq();
        Ok(())
    }
}
