//! Original US image mode 1, consumed by SLUS_004.22@0x8014E7F8.
//!
//! Each little-endian word holds six five-bit byte values. Bits 15 and 31,
//! and values after the declared output length, retain their original bytes.

use crate::{Error, Result};

const SHIFTS: [u32; 6] = [0, 5, 10, 16, 21, 26];
const MAX_OUTPUT_BYTES: usize = 1024 * 512 * 2;

fn validate_length(packed: &[u8], output_bytes: usize) -> Result<()> {
    if output_bytes == 0 || output_bytes > MAX_OUTPUT_BYTES {
        return Err(Error::Codec(format!(
            "packed five-bit output must contain 1 to {MAX_OUTPUT_BYTES} bytes"
        )));
    }
    let expected = output_bytes.div_ceil(SHIFTS.len()) * 4;
    if packed.len() != expected {
        return Err(Error::Codec(format!(
            "packed five-bit source contains {} bytes; expected {expected} for {output_bytes} output bytes",
            packed.len()
        )));
    }
    Ok(())
}

/// Decode a qualified record's declared VRAM byte count, excluding padding.
pub fn decode(packed: &[u8], output_bytes: usize) -> Result<Vec<u8>> {
    validate_length(packed, output_bytes)?;
    let mut output = Vec::with_capacity(output_bytes);
    for bytes in packed.as_chunks::<4>().0 {
        let word = u32::from_le_bytes(*bytes);
        for shift in SHIFTS {
            if output.len() == output_bytes {
                break;
            }
            output.push(((word >> shift) & 31) as u8);
        }
    }
    Ok(output)
}

/// Encode representable edits without changing unused source bits or allocation.
pub fn encode(values: &[u8], original: &[u8]) -> Result<Vec<u8>> {
    validate_length(original, values.len())?;
    if values.iter().any(|value| *value > 31) {
        return Err(Error::Codec(
            "packed five-bit edits require byte values from 0 to 31".into(),
        ));
    }
    let mut output = original.to_vec();
    for (chunk, bytes) in output.as_chunks_mut::<4>().0.iter_mut().enumerate() {
        let mut word = u32::from_le_bytes(*bytes);
        for (slot, shift) in SHIFTS.into_iter().enumerate() {
            let Some(value) = values.get(chunk * SHIFTS.len() + slot) else {
                break;
            };
            word = (word & !(31 << shift)) | (u32::from(*value) << shift);
        }
        *bytes = word.to_le_bytes();
    }
    Ok(output)
}
