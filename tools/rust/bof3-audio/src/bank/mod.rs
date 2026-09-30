//! VAB metadata. Header IDs and one-based sample numbers are not runtime bank IDs.

pub mod extraction;
pub(crate) mod manifest;
pub mod packing;

use crate::Result;
use serde::Serialize;

#[derive(Clone, Debug, Serialize)]
pub struct Bank {
    pub version: u32,
    pub header_id: u32,
    pub declared_file_bytes: u32,
    pub declared_programs: u16,
    pub declared_tones: u16,
    pub declared_samples: u16,
    pub volume: u8,
    pub pan: u8,
    /// Size-table entry zero is a transferred prefix before sample one.
    pub body_prefix_bytes: usize,
    pub programs: Vec<Program>,
    pub samples: Vec<Sample>,
    pub diagnostics: Vec<String>,
}

#[derive(Clone, Debug, Serialize)]
pub struct Program {
    pub program: u8,
    pub tone_block: usize,
    pub volume: u8,
    pub priority: u8,
    pub mode: u8,
    pub pan: u8,
    pub tones: Vec<Tone>,
}

#[derive(Clone, Debug, Serialize)]
pub struct Tone {
    pub index: usize,
    pub priority: u8,
    pub mode: u8,
    pub volume: u8,
    pub pan: u8,
    pub center: u8,
    pub shift: u8,
    pub key_min: u8,
    pub key_max: u8,
    pub vibrato_width: u8,
    pub vibrato_time: u8,
    pub portamento_width: u8,
    pub portamento_time: u8,
    pub bend_min: u8,
    pub bend_max: u8,
    pub adsr1: u16,
    pub adsr2: u16,
    pub program_reference: i16,
    pub sample_reference: i16,
}

#[derive(Clone, Debug, Serialize)]
pub struct Sample {
    pub sample_id: u16,
    pub body_offset: usize,
    pub encoded_bytes: usize,
}

impl Bank {
    pub fn parse(bytes: &[u8]) -> Result<Self> {
        if bytes.len() < 0x820 || bytes[..4] != *b"pBAV" {
            return Err("VAB: missing header or invalid magic".into());
        }
        let version = le32(bytes, 4);
        if version != 7 {
            return Err(format!(
                "VAB: unsupported version {version}; verified BOF3 layout requires version 7"
            )
            .into());
        }
        let declared_programs = le16(bytes, 0x12);
        let declared_tones = le16(bytes, 0x14);
        let declared_samples = le16(bytes, 0x16);
        if declared_programs > 128 || declared_tones > 2048 || declared_samples > 255 {
            return Err("VAB: declared program/tone/sample counts exceed format limits".into());
        }
        let table = 0x820 + usize::from(declared_programs) * 512;
        if bytes.len() < table + 512 {
            return Err("VAB: truncated tone blocks or sample-size table".into());
        }
        let mut programs = Vec::new();
        let mut diagnostics = Vec::new();
        let mut total_tones = 0;
        for program in 0..128 {
            let attr = &bytes[0x20 + program * 16..][..16];
            let count = usize::from(attr[0]);
            if count == 0 {
                continue;
            }
            if count > 16 || programs.len() >= usize::from(declared_programs) {
                return Err(format!(
                    "VAB program {program}: invalid tone count or missing tone block"
                )
                .into());
            }
            let block = programs.len();
            let mut tones = Vec::new();
            for index in 0..count {
                let t = &bytes[0x820 + block * 512 + index * 32..][..32];
                let program_reference = le16(t, 20) as i16;
                let sample_reference = le16(t, 22) as i16;
                if program_reference != program as i16 {
                    diagnostics.push(format!("program {program} tone {index}: program reference {program_reference} disagrees with owning program"));
                }
                if sample_reference < 1 || sample_reference > declared_samples as i16 {
                    diagnostics.push(format!("program {program} tone {index}: noncanonical sample reference {sample_reference} outside 1..={declared_samples}; requires target-qualified runtime resolution"));
                }
                if t[6] > t[7] || t[7] > 127 {
                    diagnostics.push(format!(
                        "program {program} tone {index}: invalid MIDI key range {}..={}",
                        t[6], t[7]
                    ));
                }
                tones.push(Tone {
                    index,
                    priority: t[0],
                    mode: t[1],
                    volume: t[2],
                    pan: t[3],
                    center: t[4],
                    shift: t[5],
                    key_min: t[6],
                    key_max: t[7],
                    vibrato_width: t[8],
                    vibrato_time: t[9],
                    portamento_width: t[10],
                    portamento_time: t[11],
                    bend_min: t[12],
                    bend_max: t[13],
                    adsr1: le16(t, 16),
                    adsr2: le16(t, 18),
                    program_reference,
                    sample_reference,
                });
            }
            total_tones += count;
            programs.push(Program {
                program: program as u8,
                tone_block: block,
                volume: attr[1],
                priority: attr[2],
                mode: attr[3],
                pan: attr[4],
                tones,
            });
        }
        if programs.len() != usize::from(declared_programs)
            || total_tones != usize::from(declared_tones)
        {
            diagnostics.push(
                "declared program/tone totals disagree with active program attributes".into(),
            );
        }
        let mut samples = Vec::new();
        let body_prefix_bytes = usize::from(le16(bytes, table)) * 8;
        let mut body_offset = body_prefix_bytes;
        for sample_id in 1..=declared_samples {
            let encoded_bytes = usize::from(le16(bytes, table + usize::from(sample_id) * 2)) * 8;
            if encoded_bytes == 0 || !encoded_bytes.is_multiple_of(16) {
                diagnostics.push(format!("sample {sample_id}: size {encoded_bytes} does not contain nonempty whole SPU ADPCM blocks"));
            }
            samples.push(Sample {
                sample_id,
                body_offset,
                encoded_bytes,
            });
            body_offset += encoded_bytes;
        }
        Ok(Self {
            version,
            header_id: le32(bytes, 8),
            declared_file_bytes: le32(bytes, 12),
            declared_programs,
            declared_tones,
            declared_samples,
            volume: bytes[0x18],
            pan: bytes[0x19],
            body_prefix_bytes,
            programs,
            samples,
            diagnostics,
        })
    }
}

fn le16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn le32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap())
}
