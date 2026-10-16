//! Raw upload placement follows SLUS producer 0x80162698 and consumer 0x80163378.

use crate::{Error, Result};
use serde::{Deserialize, Serialize};

pub const CHUNK_BYTES: usize = 0x800;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Upload {
    pub x_words: usize,
    pub y: usize,
    pub chunks_per_row: usize,
    pub chunks: usize,
    pub skipped_chunks: Vec<usize>,
}

impl Upload {
    pub fn from_argument(argument: u32, byte_count: usize) -> Result<Self> {
        if byte_count == 0 || !byte_count.is_multiple_of(CHUNK_BYTES) {
            return Err(Error::Invalid(format!(
                "raw VRAM upload has {byte_count} bytes; expected whole nonempty 0x800-byte chunks"
            )));
        }
        let mut value = Self {
            x_words: ((argument >> 24) & 0x3f) as usize * 32,
            y: ((argument >> 16) & 0x1f) as usize * 32,
            chunks_per_row: ((argument >> 8) & 0x3f) as usize,
            chunks: byte_count / CHUNK_BYTES,
            skipped_chunks: Vec::new(),
        };
        if value.chunks_per_row == 0 {
            return Err(Error::Invalid("raw VRAM row span is zero".into()));
        }
        for chunk in 0..value.chunks {
            let (x, y) = value.position(chunk);
            if x + 32 > 1024 || y + 32 > 512 {
                value.skipped_chunks.push(chunk);
            }
        }
        Ok(value)
    }

    pub fn position(&self, chunk: usize) -> (usize, usize) {
        (
            ((self.x_words / 32 + chunk % self.chunks_per_row) & 0xff) * 32,
            ((self.y / 32 + chunk / self.chunks_per_row) & 0x3f) * 32,
        )
    }

    pub fn width_words(&self) -> usize {
        self.chunks.min(self.chunks_per_row) * 32
    }
    pub fn height(&self) -> usize {
        self.chunks.div_ceil(self.chunks_per_row) * 32
    }

    pub fn linear_words(&self, bytes: &[u8]) -> Result<Vec<u16>> {
        self.validate_size(bytes.len())?;
        let width = self.width_words();
        let mut words = vec![0; width * self.height()];
        for (chunk, payload) in bytes.as_chunks::<CHUNK_BYTES>().0.iter().enumerate() {
            let cx = chunk % self.chunks_per_row * 32;
            let cy = chunk / self.chunks_per_row * 32;
            for row in 0..32 {
                for col in 0..32 {
                    let offset = (row * 32 + col) * 2;
                    words[(cy + row) * width + cx + col] =
                        u16::from_le_bytes([payload[offset], payload[offset + 1]]);
                }
            }
        }
        Ok(words)
    }

    pub fn encode_words(&self, words: &[u16], original: &[u8]) -> Result<Vec<u8>> {
        self.validate_size(original.len())?;
        let width = self.width_words();
        if words.len() != width * self.height() {
            return Err(Error::Invalid(
                "edited upload dimensions differ from the source".into(),
            ));
        }
        let mut encoded = original.to_vec();
        let occupied = self.chunks % self.chunks_per_row;
        if occupied != 0 && self.chunks > self.chunks_per_row {
            let row_start = self.height() - 32;
            for row in row_start..self.height() {
                if words[row * width + occupied * 32..(row + 1) * width]
                    .iter()
                    .any(|word| *word != 0)
                {
                    return Err(Error::Invalid(
                        "edit changes a hole outside the source upload".into(),
                    ));
                }
            }
        }
        for (chunk, payload) in encoded
            .as_chunks_mut::<CHUNK_BYTES>()
            .0
            .iter_mut()
            .enumerate()
        {
            let cx = chunk % self.chunks_per_row * 32;
            let cy = chunk / self.chunks_per_row * 32;
            for row in 0..32 {
                for col in 0..32 {
                    let offset = (row * 32 + col) * 2;
                    payload[offset..offset + 2]
                        .copy_from_slice(&words[(cy + row) * width + cx + col].to_le_bytes());
                }
            }
        }
        Ok(encoded)
    }

    fn validate_size(&self, byte_count: usize) -> Result<()> {
        if byte_count != self.chunks * CHUNK_BYTES {
            return Err(Error::Invalid("upload/source length mismatch".into()));
        }
        Ok(())
    }
}

pub fn indices(words: &[u16], bits: u8) -> Result<Vec<u8>> {
    if bits != 4 && bits != 8 {
        return Err(Error::Invalid("indexed pixels require 4 or 8 bits".into()));
    }
    let mask = (1u16 << bits) - 1;
    Ok(words
        .iter()
        .flat_map(|word| (0..16 / bits).map(move |slot| ((word >> (slot * bits)) & mask) as u8))
        .collect())
}

pub fn words_from_indices(indices: &[u8], bits: u8) -> Result<Vec<u16>> {
    if bits != 4 && bits != 8 {
        return Err(Error::Invalid("indexed pixels require 4 or 8 bits".into()));
    }
    let per_word = usize::from(16 / bits);
    if !indices.len().is_multiple_of(per_word)
        || indices
            .iter()
            .any(|value| u16::from(*value) >= 1u16 << bits)
    {
        return Err(Error::Invalid(format!(
            "indices are not representable at {bits} bits"
        )));
    }
    Ok(indices
        .chunks_exact(per_word)
        .map(|chunk| {
            chunk.iter().enumerate().fold(0, |word, (index, value)| {
                word | (u16::from(*value) << (index * usize::from(bits)))
            })
        })
        .collect())
}

/// Preview RGBA retains the difference between zero color and STP black.
/// STP blending still depends on the consumer's draw state and background.
pub fn color_rgba(word: u16) -> [u8; 4] {
    let expand = |shift| {
        let value = (word >> shift) & 31u16;
        ((value << 3) | (value >> 2)) as u8
    };
    [
        expand(0),
        expand(5),
        expand(10),
        if word == 0 { 0 } else { 255 },
    ]
}
