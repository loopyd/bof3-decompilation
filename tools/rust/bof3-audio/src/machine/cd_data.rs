//! Explicitly delivered CD data blocks, separate from drive/sector scheduling.
use crate::Result;

#[derive(Clone, Copy, Debug)]
pub enum Model {
    /// Inspected emulator: clearing BFRD rewinds a partial block; exhaustion
    /// clears request/readiness. Overread and queue-overrun behaviour is excluded.
    EmulatorReference,
}

pub struct Fifo {
    bytes: Vec<u8>,
    cursor: usize,
    requested: bool,
    model: Model,
}
impl Fifo {
    pub fn new(model: Model) -> Self {
        Self {
            bytes: Vec::new(),
            cursor: 0,
            requested: false,
            model,
        }
    }
    pub fn present(&mut self, bytes: &[u8]) -> Result<()> {
        if !matches!(bytes.len(), 2048 | 2340) {
            return Err("CD data: expected an explicit 2048- or 2340-byte output block".into());
        }
        if self.cursor < self.bytes.len() {
            return Err(
                "CD data: unread block replacement/overrun requires drive queue scheduling".into(),
            );
        }
        self.bytes = bytes.to_vec();
        self.cursor = 0;
        self.requested = false;
        Ok(())
    }
    /// Select a new block at a serialized INT1 delivery boundary. An old block
    /// which is not currently requested is no longer selected. Locked reads and queued
    /// interrupt/sector overruns require a separate scheduler and still reject.
    /// Returns the number of previously selected unread bytes displaced.
    pub fn select_for_interrupt(&mut self, bytes: &[u8]) -> Result<usize> {
        if !matches!(bytes.len(), 2048 | 2340) {
            return Err("CD data: expected 2048- or 2340-byte INT1 block".into());
        }
        if self.requested {
            return Err("CD data: cannot replace a requested block at INT1 delivery".into());
        }
        let displaced = self.remaining();
        self.bytes = bytes.to_vec();
        self.cursor = 0;
        Ok(displaced)
    }
    pub fn request(&mut self, active: bool) -> Result<()> {
        if active && self.cursor == self.bytes.len() {
            return Err("CD data: request without an available output block".into());
        }
        if !active {
            match self.model {
                Model::EmulatorReference => self.cursor = 0,
            }
        }
        self.requested = active;
        Ok(())
    }
    pub fn ready(&self) -> bool {
        self.requested && self.cursor < self.bytes.len()
    }
    pub fn reset(&mut self) -> usize {
        let discarded = self.remaining();
        self.bytes.clear();
        self.cursor = 0;
        self.requested = false;
        discarded
    }
    pub fn remaining(&self) -> usize {
        self.bytes.len() - self.cursor
    }
    pub fn read(&mut self, count: usize) -> Result<u32> {
        if !matches!(count, 1 | 2 | 4) || !self.requested || count > self.remaining() {
            return Err(
                "CD data: inactive request or out-of-range read; overread padding is unverified"
                    .into(),
            );
        }
        let mut bytes = [0; 4];
        bytes[..count].copy_from_slice(&self.bytes[self.cursor..self.cursor + count]);
        self.cursor += count;
        if self.cursor == self.bytes.len() {
            self.bytes.clear();
            self.cursor = 0;
            self.requested = false;
        }
        Ok(u32::from_le_bytes(bytes))
    }
}
