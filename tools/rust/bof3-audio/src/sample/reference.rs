//! Sample selection in the identified US VAB note-on path, not generic VAB rules.
use crate::Result;
use serde::Serialize;

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct Resolution {
    pub encoded_reference: i16,
    /// The tone selector stores only the low byte in its selected-sample list.
    pub selected_byte: u8,
    /// One-based sample allocation whose start address the runtime reads.
    pub sample_id: u16,
    pub address_table_program: usize,
    pub address_table_offset: usize,
}

pub fn resolve_us_pcm(reference: i16, declared_samples: u16) -> Result<Resolution> {
    if declared_samples > 255 {
        return Err(
            "sample reference: VAB sample count exceeds the verified byte-sized runtime table"
                .into(),
        );
    }
    let byte = reference as u8;
    if byte == 255 {
        return Err(format!("sample reference {reference}: runtime byte 255 selects SPU noise; PCM SoundFont mapping is unsupported").into());
    }
    // Original signed (byte - 1) / 2 truncates toward zero; the even branch
    // then reads +14. Thus zero reads exactly the same slot as reference two.
    let sample_id = if byte == 0 { 2 } else { u16::from(byte) };
    if sample_id > declared_samples {
        return Err(format!("invalid sample reference {reference}: runtime byte {byte} addresses allocation {sample_id} outside 1..={declared_samples}; adjacent SPU media context is required").into());
    }
    Ok(Resolution {
        encoded_reference: reference,
        selected_byte: byte,
        sample_id,
        address_table_program: usize::from(sample_id - 1) / 2,
        address_table_offset: if sample_id % 2 == 1 { 12 } else { 14 },
    })
}
