//! Independent SEP sequence ranges. Event translation is a separate validation gate.

pub mod clock;
pub mod editing;
pub mod events;
pub mod loops;
pub mod midi;
pub mod termination;
pub mod timeline;
pub mod timing;

use crate::Result;
use serde::Serialize;

#[derive(Clone, Debug, Serialize)]
pub struct SequenceSet {
    pub version: u16,
    pub sequences: Vec<Sequence>,
    pub trailing_offset: usize,
    pub trailing_bytes: usize,
    pub event_validation: &'static str,
}

#[derive(Clone, Debug, Serialize)]
pub struct Sequence {
    pub sequence_index: usize,
    pub sequence_id: u16,
    pub resolution: u16,
    pub tempo_us: u32,
    pub time_numerator: u8,
    pub time_denominator_power: u8,
    pub data_offset: usize,
    pub data_bytes: usize,
}

impl SequenceSet {
    pub fn parse(bytes: &[u8]) -> Result<Self> {
        if bytes.len() < 6 || bytes[..4] != *b"pQES" {
            return Err("SEP: missing header or invalid magic".into());
        }
        let version = be16(bytes, 4);
        if version != 0 {
            return Err(format!("SEP: unsupported version {version}").into());
        }
        let mut sequences = Vec::new();
        let mut pos = 6;
        while pos < bytes.len() {
            // An all-zero suffix is retained container padding, never another sequence.
            if bytes[pos..].iter().all(|&b| b == 0) {
                break;
            }
            if bytes.len() - pos < 13 {
                return Err(format!("SEP: truncated sequence header at byte {pos}").into());
            }
            let data_bytes =
                u32::from_be_bytes(bytes[pos + 9..pos + 13].try_into().unwrap()) as usize;
            let data_offset = pos + 13;
            if data_bytes == 0 || data_bytes > bytes.len() - data_offset {
                return Err(format!("SEP: invalid sequence length at byte {pos}").into());
            }
            sequences.push(Sequence {
                sequence_index: sequences.len(),
                sequence_id: be16(bytes, pos),
                resolution: be16(bytes, pos + 2),
                tempo_us: u32::from_be_bytes([0, bytes[pos + 4], bytes[pos + 5], bytes[pos + 6]]),
                time_numerator: bytes[pos + 7],
                time_denominator_power: bytes[pos + 8],
                data_offset,
                data_bytes,
            });
            pos = data_offset + data_bytes;
        }
        if sequences.is_empty() {
            return Err("SEP: no independent sequences".into());
        }
        Ok(Self {
            version,
            sequences,
            trailing_offset: pos,
            trailing_bytes: bytes.len() - pos,
            event_validation: "not_checked",
        })
    }
}

fn be16(bytes: &[u8], at: usize) -> u16 {
    u16::from_be_bytes([bytes[at], bytes[at + 1]])
}
