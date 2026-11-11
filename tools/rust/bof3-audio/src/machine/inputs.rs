//! Read-only host input ABI for the owned guest dispatcher and media copies.
use super::bus::{BusError, Width};
use crate::Result;

pub const COMMANDS: u32 = 0x1f70_0000;
pub const MEDIA: u32 = 0x1f71_0000;
pub const MEDIA_BYTES: usize = 0x10000;

/// A supplied transaction, without guest memory or clock mutation.
pub struct Observation<'a> {
    pub sequence: u64,
    pub words: &'a [u32; 9],
    pub media: &'a [u8],
}

#[derive(Default)]
pub(crate) struct Inputs {
    command: Option<[u32; 9]>,
    read_mask: u16,
    sequence: u64,
    media: Vec<u8>,
}

pub(crate) fn contains(address: u32) -> bool {
    (COMMANDS..COMMANDS + 36).contains(&address)
        || (MEDIA..MEDIA + MEDIA_BYTES as u32).contains(&address)
}

impl Inputs {
    pub(crate) fn command(&mut self, words: [u32; 9]) -> Result<()> {
        if self.command.is_some() && self.read_mask != 0x1ff {
            return Err("owned input: previous command has not been read completely".into());
        }
        self.sequence = self
            .sequence
            .checked_add(1)
            .ok_or("owned input: sequence overflow")?;
        self.command = Some(words);
        self.read_mask = 0;
        Ok(())
    }

    pub(crate) fn media(&mut self, bytes: &[u8]) -> Result<()> {
        if bytes.is_empty() || bytes.len() > MEDIA_BYTES {
            return Err("owned input: media must contain 1..=65536 bytes".into());
        }
        self.media.clear();
        self.media.extend_from_slice(bytes);
        Ok(())
    }

    pub(crate) fn observation(&self) -> Option<Observation<'_>> {
        Some(Observation {
            sequence: self.sequence,
            words: self.command.as_ref()?,
            media: &self.media,
        })
    }

    pub(crate) fn read(
        &mut self,
        address: u32,
        width: Width,
    ) -> std::result::Result<u32, BusError> {
        let fail = |detail: &str| BusError {
            address,
            detail: detail.into(),
        };
        if (COMMANDS..COMMANDS + 36).contains(&address) {
            if width != Width::Word || address & 3 != 0 {
                return Err(fail("owned command input requires aligned word reads"));
            }
            let words = self
                .command
                .as_ref()
                .ok_or_else(|| fail("owned command input is empty"))?;
            let index = ((address - COMMANDS) / 4) as usize;
            self.read_mask |= 1 << index;
            return Ok(words[index]);
        }
        let offset = address
            .checked_sub(MEDIA)
            .ok_or_else(|| fail("outside owned media input"))? as usize;
        let end = offset
            .checked_add(width.bytes())
            .ok_or_else(|| fail("owned media input overflow"))?;
        if !(address as usize).is_multiple_of(width.bytes()) || end > self.media.len() {
            return Err(fail(
                "owned media input read exceeds supplied bytes or is unaligned",
            ));
        }
        let mut word = [0; 4];
        word[..width.bytes()].copy_from_slice(&self.media[offset..end]);
        Ok(u32::from_le_bytes(word))
    }
}
