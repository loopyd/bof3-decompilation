//! Eight-byte image records consumed by SLUS_004.22@0x8014E6D0.
//!
//! Callers qualify record locations and allocations. This owner does not select
//! bank roots, palettes, GPU texture modes, or destination coordinates.

use serde::Serialize;

use crate::{Error, Result, lzss, packed};

const MAX_BANK_BYTES: usize = 1024 * 1024;
const HEADER_BYTES: usize = 8;

#[derive(Clone, Debug, Serialize, PartialEq, Eq)]
pub struct Info {
    pub offset: usize,
    pub end: usize,
    pub width_words: u16,
    pub height: u16,
    pub mode: u32,
    /// Includes the eight-byte header, but excludes unconsumed allocation bytes.
    pub consumed_bytes: usize,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Decoded {
    pub info: Info,
    pub bytes: Vec<u8>,
    /// Final mode-2 token writes beyond the declared VRAM byte count.
    pub overshoot: Vec<u8>,
}

/// Decode one record in decoded-bank byte coordinates.
/// The caller supplies its qualified offset and containing extent. Trailing
/// bytes are not interpreted as padding or evidence of reusable allocation.
pub fn decode(source: &[u8], offset: usize, end: usize) -> Result<Decoded> {
    if source.len() > MAX_BANK_BYTES {
        return Err(Error::Codec(format!(
            "image source exceeds its {MAX_BANK_BYTES}-byte decoded-bank bound"
        )));
    }
    let record = source
        .get(offset..end)
        .ok_or_else(|| Error::Codec("image record exceeds its source extent".into()))?;
    let header = record
        .get(..HEADER_BYTES)
        .ok_or_else(|| Error::Codec("image record lacks its eight-byte header".into()))?;
    let width_words = u16::from_le_bytes([header[0], header[1]]);
    let height = u16::from_le_bytes([header[2], header[3]]);
    let mode = u32::from_le_bytes([header[4], header[5], header[6], header[7]]);
    if !(1..=1024).contains(&width_words) || !(1..=512).contains(&height) {
        return Err(Error::Codec(
            "image dimensions must fit 1..=1024 VRAM words by 1..=512 rows".into(),
        ));
    }
    let output_bytes = usize::from(width_words) * usize::from(height) * 2;
    let body = &record[HEADER_BYTES..];
    let (bytes, overshoot, consumed) = match mode {
        1 => {
            let consumed = output_bytes.div_ceil(6) * 4;
            let stream = body.get(..consumed).ok_or_else(|| {
                Error::Codec("mode-1 image stream exceeds its record extent".into())
            })?;
            (packed::decode(stream, output_bytes)?, Vec::new(), consumed)
        }
        2 => {
            let decoded = lzss::decode(body, output_bytes)?;
            (decoded.bytes, decoded.overshoot, decoded.consumed)
        }
        _ => {
            let bytes = body
                .get(..output_bytes)
                .ok_or_else(|| Error::Codec("raw-word image exceeds its record extent".into()))?;
            (bytes.to_vec(), Vec::new(), output_bytes)
        }
    };
    Ok(Decoded {
        info: Info {
            offset,
            end,
            width_words,
            height,
            mode,
            consumed_bytes: HEADER_BYTES + consumed,
        },
        bytes,
        overshoot,
    })
}

/// Replace the declared image bytes within a caller-qualified allocation.
/// `allocation_end` must exclude opaque or shared records. Do not infer free
/// space from a following image or animation pointer. Without a proven reusable
/// tail, use `offset + decode(...).info.consumed_bytes` as the allocation end.
/// Preserve the header, bytes outside the allocation, unused mode-1 bits,
/// mode-2 prefix and final-token overshoot, and unconsumed trailing bytes.
pub fn encode(
    values: &[u8],
    original: &[u8],
    offset: usize,
    allocation_end: usize,
) -> Result<Vec<u8>> {
    let decoded = decode(original, offset, allocation_end)?;
    if values.len() != decoded.bytes.len() {
        return Err(Error::Codec(format!(
            "image edit contains {} bytes; expected {} from its original dimensions",
            values.len(),
            decoded.bytes.len()
        )));
    }
    if values == decoded.bytes {
        return Ok(original.to_vec());
    }
    let body_start = offset + HEADER_BYTES;
    let mut output = original.to_vec();
    match decoded.info.mode {
        1 => {
            let body_end = offset + decoded.info.consumed_bytes;
            let encoded = packed::encode(values, &original[body_start..body_end])?;
            output[body_start..body_end].copy_from_slice(&encoded);
        }
        2 => {
            let encoded = lzss::encode(values, &original[body_start..allocation_end])?;
            output[body_start..allocation_end].copy_from_slice(&encoded);
        }
        _ => output[body_start..body_start + values.len()].copy_from_slice(values),
    }
    let checked = decode(&output, offset, allocation_end)?;
    if checked.bytes != values || checked.overshoot != decoded.overshoot {
        return Err(Error::Codec(
            "encoded image bytes or final-token overshoot differ from the requested edit".into(),
        ));
    }
    Ok(output)
}
