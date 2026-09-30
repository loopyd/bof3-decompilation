//! Clocked FIFO/RAM service using an explicit reference-emulator transfer rate.
//! This does not schedule voice frames or establish CPU/DMA bus arbitration.
use super::{interconnect::Interconnect, spu_transfer::Mode};
use crate::Result;
use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Model {
    /// DuckStation's TRANSFER_TICKS_PER_HALFWORD: 16 system-clock ticks.
    /// Control applies at the next advance call; not hardware latency evidence.
    EmulatorReference,
}

#[derive(Debug, Serialize)]
pub struct Clock {
    model: Model,
    ticks: u64,
    halfwords: u64,
    remaining: u32,
}

impl Clock {
    pub fn new(model: Model) -> Self {
        Self {
            model,
            ticks: 0,
            halfwords: 0,
            remaining: 16,
        }
    }

    pub fn ticks(&self) -> u64 {
        self.ticks
    }

    pub fn halfwords(&self) -> u64 {
        self.halfwords
    }

    fn active(bus: &Interconnect) -> bool {
        let transfer = bus.spu_transfer();
        match transfer.mode() {
            Mode::Stopped => false,
            Mode::ManualWrite | Mode::DmaWrite => transfer.fifo_halfwords() != 0,
            Mode::DmaRead => transfer.fifo_halfwords() < super::spu_transfer::FIFO_HALFWORDS,
        }
    }

    /// Advance system-clock ticks, preserving fractional transfer progress.
    /// Call at each CPU/device boundary; writes are ordered before this call.
    /// An error stops at the failing boundary; callers must not replay elapsed
    /// ticks. DMA FIFO handshakes retain Interconnect's untimed bus model.
    pub fn advance(&mut self, bus: &mut Interconnect, ticks: u32) -> Result<()> {
        let end = self
            .ticks
            .checked_add(u64::from(ticks))
            .ok_or("SPU clock overflow")?;
        if ticks == 0 {
            return Ok(());
        }
        bus.service_spu(0)?;
        let mut left = ticks;
        while Self::active(bus) {
            if left < self.remaining {
                self.remaining -= left;
                self.ticks = end;
                return Ok(());
            }
            left -= self.remaining;
            self.ticks += u64::from(self.remaining);
            self.remaining = 16;
            self.halfwords += bus.service_spu(1)? as u64;
        }
        // No credit accumulates during idle time for future FIFO data.
        self.remaining = 16;
        self.ticks = end;
        Ok(())
    }
}
