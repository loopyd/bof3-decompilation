//! Outer bank stream consumed by SLUS_004.22@0x80164064 and 0x801642F4.

use std::collections::{HashMap, VecDeque};

use serde::{Deserialize, Serialize};

use crate::{Error, Result};

const HEADER: usize = 6;
const WINDOW: usize = 2048;
const MAX_REFERENCE: usize = 273;
const MAX_LITERAL: usize = 271;
const MAX_OUTPUT: usize = 1024 * 1024;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub decoded_bytes: usize,
    pub consumed_bytes: usize,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Decoded {
    pub bytes: Vec<u8>,
    pub info: Info,
}

struct Reader<'a> {
    bytes: &'a [u8],
    position: usize,
}

impl<'a> Reader<'a> {
    fn take(&mut self, count: usize) -> Result<&'a [u8]> {
        let end = self
            .position
            .checked_add(count)
            .ok_or_else(|| Error::Codec("outer-bank source offset overflows".into()))?;
        let bytes = self.bytes.get(self.position..end).ok_or_else(|| {
            Error::Codec(format!(
                "outer-bank source is truncated at byte {}",
                self.position
            ))
        })?;
        self.position = end;
        Ok(bytes)
    }

    fn byte(&mut self) -> Result<u8> {
        Ok(self.take(1)?[0])
    }
}

/// Decode bounded original output without inventing pre-destination RAM history.
pub fn decode(packed: &[u8]) -> Result<Decoded> {
    let header = packed
        .get(..HEADER)
        .ok_or_else(|| Error::Codec("outer-bank source lacks its six-byte header".into()))?;
    let declared = u32::from_le_bytes(header[..4].try_into().unwrap()) as usize;
    if declared == 0 || declared > MAX_OUTPUT {
        return Err(Error::Codec(format!(
            "outer-bank declared output must contain 1 to {MAX_OUTPUT} bytes"
        )));
    }
    let mut reader = Reader {
        bytes: packed,
        position: HEADER,
    };
    let mut output = Vec::with_capacity(declared);
    loop {
        let control = reader.byte()?;
        if control == 0xff {
            break;
        }
        let literal = control & 0x10 != 0;
        let displacement = if literal {
            0
        } else {
            (((i32::from(control & 0xe0) | -256) << 3) | i32::from(reader.byte()?)) as isize
        };
        let mut count = if literal {
            32 - usize::from(control & 31)
        } else {
            usize::from(control & 15) + 3
        };
        if count == if literal { 16 } else { 18 } {
            count += usize::from(reader.byte()?);
        }
        if count > declared - output.len() {
            return Err(Error::Codec(
                "outer-bank token exceeds its declared output".into(),
            ));
        }
        if literal {
            output.extend_from_slice(reader.take(count)?);
            continue;
        }
        let source = output
            .len()
            .checked_add_signed(displacement)
            .ok_or_else(|| {
                Error::Codec("outer-bank reference needs unknown pre-destination RAM".into())
            })?;
        for step in 0..count {
            // The original forward byte copy observes earlier writes on overlap.
            output.push(output[source + step]);
        }
    }
    if output.len() != declared {
        return Err(Error::Codec(format!(
            "outer-bank header declares {declared} bytes; terminator follows {} decoded bytes",
            output.len()
        )));
    }
    Ok(Decoded {
        info: Info {
            decoded_bytes: output.len(),
            consumed_bytes: reader.position,
        },
        bytes: output,
    })
}

#[derive(Default)]
struct Matches {
    positions: HashMap<[u8; 3], VecDeque<usize>>,
    inserted: usize,
}

impl Matches {
    fn before(&mut self, values: &[u8], position: usize) {
        while self.inserted < position {
            let offset = self.inserted;
            if let Some(key) = values.get(offset..offset + 3) {
                self.positions
                    .entry(key.try_into().unwrap())
                    .or_default()
                    .push_back(offset);
            }
            if offset >= WINDOW {
                let expired = offset - WINDOW;
                let key: [u8; 3] = values[expired..expired + 3].try_into().unwrap();
                if let Some(positions) = self.positions.get_mut(&key) {
                    positions.pop_front();
                    if positions.is_empty() {
                        self.positions.remove(&key);
                    }
                }
            }
            self.inserted += 1;
        }
    }

    fn longest(&self, values: &[u8], position: usize) -> (usize, usize) {
        let Some(key) = values.get(position..position + 3) else {
            return (0, 0);
        };
        let Some(candidates) = self.positions.get::<[u8; 3]>(&key.try_into().unwrap()) else {
            return (0, 0);
        };
        let limit = MAX_REFERENCE.min(values.len() - position);
        let mut best = (0, 0);
        for &source in candidates.iter().rev() {
            let mut length = 3;
            while length < limit && values[source + length] == values[position + length] {
                length += 1;
            }
            if length > best.1 {
                best = (position - source, length);
            }
            if length == limit {
                break;
            }
        }
        best
    }
}

fn literal(encoded: &mut Vec<u8>, values: &[u8]) {
    if values.is_empty() {
        return;
    }
    if values.len() < 16 {
        encoded.push((32 - values.len()) as u8);
    } else {
        encoded.extend_from_slice(&[0x10, (values.len() - 16) as u8]);
    }
    encoded.extend_from_slice(values);
}

/// Preserve unchanged streams, opaque header bytes and trailing allocation bytes.
/// This byte inverse does not validate sprite tables or authorize logical edits.
pub fn encode(values: &[u8], original: &[u8]) -> Result<Vec<u8>> {
    let decoded = decode(original)?;
    if decoded.bytes == values {
        return Ok(original.to_vec());
    }
    if values.len() != decoded.bytes.len() {
        return Err(Error::Codec(
            "outer-bank edit changes its decoded allocation".into(),
        ));
    }
    let mut encoded = original[..HEADER].to_vec();
    let mut matches = Matches::default();
    let mut position = 0;
    let mut pending = 0;
    while position < values.len() {
        matches.before(values, position);
        let (distance, length) = matches.longest(values, position);
        if length >= 3 {
            literal(&mut encoded, &values[pending..position]);
            let displacement = -(distance as isize);
            encoded.extend_from_slice(&[
                (((displacement >> 3) & 0xe0) as u8) | ((length - 3).min(15) as u8),
                displacement as u8,
            ]);
            if length >= 18 {
                encoded.push((length - 18) as u8);
            }
            position += length;
            pending = position;
            continue;
        }
        position += 1;
        if position - pending == MAX_LITERAL {
            literal(&mut encoded, &values[pending..position]);
            pending = position;
        }
    }
    literal(&mut encoded, &values[pending..]);
    encoded.push(0xff);
    if encoded.len() > original.len() {
        return Err(Error::Codec(format!(
            "outer-bank edit needs {} bytes; original allocation is {}",
            encoded.len(),
            original.len()
        )));
    }
    let mut output = original.to_vec();
    output[..encoded.len()].copy_from_slice(&encoded);
    if decode(&output)?.bytes != values {
        return Err(Error::Codec(
            "outer-bank inverse differs from the requested bytes".into(),
        ));
    }
    Ok(output)
}
