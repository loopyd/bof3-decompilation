//! US image mode 2, consumed by SLUS_004.22@0x8014E8FC.

use crate::{Error, Result};

const WINDOW: usize = 512;
const START: usize = 0x1ee;
const MAX_OUTPUT: usize = 1024 * 512 * 2;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Decoded {
    pub bytes: Vec<u8>,
    pub overshoot: Vec<u8>,
    pub consumed: usize,
}

struct History {
    bytes: [Option<u8>; WINDOW],
    cursor: usize,
}

impl History {
    fn new() -> Self {
        let mut bytes = [None; WINDOW];
        bytes[..START].fill(Some(0));
        Self {
            bytes,
            cursor: START,
        }
    }

    fn push(&mut self, value: u8) {
        self.bytes[self.cursor] = Some(value);
        self.cursor = (self.cursor + 1) & (WINDOW - 1);
    }

    fn from_prefix(prefix: &[u8]) -> Self {
        let mut history = Self::new();
        let skip = prefix.len().saturating_sub(WINDOW);
        history.cursor = (START + skip) & (WINDOW - 1);
        for value in &prefix[skip..] {
            history.push(*value);
        }
        history
    }

    fn match_length(&self, offset: usize, target: &[u8]) -> usize {
        for (step, expected) in target.iter().take(18).enumerate() {
            let source = (offset + step) & (WINDOW - 1);
            let written = (source + WINDOW - self.cursor) & (WINDOW - 1);
            let value = if written < step {
                Some(target[written])
            } else {
                self.bytes[source]
            };
            if value != Some(*expected) {
                return step;
            }
        }
        target.len().min(18)
    }

    fn best_match(&self, target: &[u8]) -> (usize, usize) {
        let mut best = (0, 0);
        for offset in 0..WINDOW {
            let length = self.match_length(offset, target);
            if length > best.1 {
                best = (offset, length);
                if length == target.len().min(18) {
                    break;
                }
            }
        }
        best
    }
}

fn output_bound(size: usize) -> Result<()> {
    if size == 0 || size > MAX_OUTPUT {
        return Err(Error::Codec(format!(
            "mode-2 output must contain 1 to {MAX_OUTPUT} bytes"
        )));
    }
    Ok(())
}

/// Retain final-token writes beyond the declared VRAM length separately.
pub fn decode(packed: &[u8], output_bytes: usize) -> Result<Decoded> {
    output_bound(output_bytes)?;
    if packed.len() < 4 {
        return Err(Error::Codec(
            "mode-2 source lacks its four-byte prefix".into(),
        ));
    }
    let mut history = History::new();
    let mut output = Vec::with_capacity(output_bytes + 17);
    let mut cursor = 4;
    let mut read = || -> Result<u8> {
        let value = packed
            .get(cursor)
            .copied()
            .ok_or_else(|| Error::Codec(format!("mode-2 source is truncated at byte {cursor}")))?;
        cursor += 1;
        Ok(value)
    };
    while output.len() < output_bytes {
        let flags = read()?;
        for bit in 0..8 {
            if output.len() >= output_bytes {
                break;
            }
            if flags & (1 << bit) != 0 {
                let value = read()?;
                output.push(value);
                history.push(value);
            } else {
                let low = usize::from(read()?);
                let high = usize::from(read()?);
                let offset = low | ((high & 0xf0) << 4);
                let length = (high & 15) + 3;
                for step in 0..length {
                    let source = (offset + step) & (WINDOW - 1);
                    let value = history.bytes[source].ok_or_else(|| {
                        Error::Codec(format!(
                            "mode-2 reference reads uninitialized scratch byte {source:#x} at output byte {}",
                            output.len()
                        ))
                    })?;
                    output.push(value);
                    history.push(value);
                }
            }
        }
    }
    let overshoot = output.split_off(output_bytes);
    Ok(Decoded {
        bytes: output,
        overshoot,
        consumed: cursor,
    })
}

#[derive(Default)]
struct Encoder {
    bytes: Vec<u8>,
    flag: usize,
    bit: u8,
}

impl Encoder {
    fn token(&mut self, literal: bool, bytes: &[u8]) {
        if self.bit == 0 {
            self.flag = self.bytes.len();
            self.bytes.push(0);
        }
        if literal {
            self.bytes[self.flag] |= 1 << self.bit;
        }
        self.bytes.extend_from_slice(bytes);
        self.bit = (self.bit + 1) & 7;
    }

    fn reference(&mut self, offset: usize, length: usize) {
        self.token(
            false,
            &[
                offset as u8,
                ((offset >> 4) & 0xf0) as u8 | (length - 3) as u8,
            ],
        );
    }

    fn prefix(&mut self, target: &[u8]) {
        let mut history = History::new();
        let mut position = 0;
        while position < target.len() {
            let (offset, length) = history.best_match(&target[position..]);
            let consumed = if length >= 3 {
                self.reference(offset, length);
                length
            } else {
                self.token(true, &target[position..position + 1]);
                1
            };
            for value in &target[position..position + consumed] {
                history.push(*value);
            }
            position += consumed;
        }
    }
}

/// Preserve unchanged streams, opaque prefix/trailing bytes, and final overshoot.
pub fn encode(values: &[u8], original: &[u8]) -> Result<Vec<u8>> {
    let decoded = decode(original, values.len())?;
    if decoded.bytes == values {
        return Ok(original.to_vec());
    }
    let mut encoder = Encoder::default();
    if decoded.overshoot.is_empty() {
        encoder.prefix(values);
    } else {
        let mut extended = values.to_vec();
        extended.extend_from_slice(&decoded.overshoot);
        let mut final_reference = None;
        for start in (values.len().saturating_sub(17)..values.len()).rev() {
            let remaining = &extended[start..];
            if !(3..=18).contains(&remaining.len()) {
                continue;
            }
            let history = History::from_prefix(&values[..start]);
            let (offset, length) = history.best_match(remaining);
            if length == remaining.len() {
                final_reference = Some((start, offset, length));
                break;
            }
        }
        let (start, offset, length) = final_reference.ok_or_else(|| {
            Error::Codec("mode-2 edit cannot preserve the original final-token overshoot".into())
        })?;
        encoder.prefix(&values[..start]);
        encoder.reference(offset, length);
    }
    let used = 4 + encoder.bytes.len();
    if used > original.len() {
        return Err(Error::Codec(format!(
            "mode-2 edit needs {used} bytes; original allocation is {}",
            original.len()
        )));
    }
    let mut output = original.to_vec();
    output[4..used].copy_from_slice(&encoder.bytes);
    let checked = decode(&output, values.len())?;
    if checked.bytes != values || checked.overshoot != decoded.overshoot {
        return Err(Error::Codec(
            "mode-2 encoded output or overshoot differs from the requested edit".into(),
        ));
    }
    Ok(output)
}
