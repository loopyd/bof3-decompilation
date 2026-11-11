//! Owned reset ROM layout and loading bounds.
use super::Image;
use crate::{machine::image::firmware, Result};

pub(super) const ENTRY: u32 = 0xbfc0_0000;
const PAYLOAD: usize = 0x1000;

impl Image {
    pub(super) fn validate_reset(&self) -> Result<()> {
        let start = self.entry("audio_reset_start")?;
        let end = self.symbol("audio_reset_end")?;
        if self.symbol("audio_reset_payload")? != ENTRY + PAYLOAD as u32
            || end & 3 != 0
            || end
                .checked_sub(start)
                .is_none_or(|n| n == 0 || n > PAYLOAD as u32)
            || end > self.symbol("audio_sequence_scheduler_load")?
            || self.bytes.len() & 3 != 0
            || self.bytes.len() > firmware::ROM_BYTES - PAYLOAD
        {
            return Err("owned driver: reset ROM layout is inconsistent".into());
        }
        Ok(())
    }

    /// Build the same immutable owned ROM used by cold production startup.
    /// Native references can execute it from reset without a retail BIOS,
    /// executable handoff, imported RAM or capture-time guest writes.
    pub fn reset_rom(&self) -> Result<Vec<u8>> {
        let base = self.symbol("audio_kernel_image")?;
        let start = (self.symbol("audio_reset_start")? - base) as usize;
        let end = (self.symbol("audio_reset_end")? - base) as usize;
        let mut rom = vec![0; firmware::ROM_BYTES];
        rom[..end - start].copy_from_slice(&self.bytes[start..end]);
        rom[PAYLOAD..PAYLOAD + self.bytes.len()].copy_from_slice(&self.bytes);
        Ok(rom)
    }
}
