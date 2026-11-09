//! Elapsed CPU cycles supplied by the execution owner, independent of device work.
use crate::Result;
use serde::Serialize;

#[derive(Debug, Default, Serialize)]
pub struct Clock {
    ticks: u64,
}

impl Clock {
    pub fn ticks(&self) -> u64 {
        self.ticks
    }

    pub fn advance(&mut self, ticks: u32) -> Result<()> {
        self.ticks = self
            .ticks
            .checked_add(u64::from(ticks))
            .ok_or("elapsed clock overflow")?;
        Ok(())
    }
}
