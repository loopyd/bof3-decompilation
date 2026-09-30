//! CD-ROM EDC for extracted Mode 2 / Form 2 sectors, not Form 1 ECC repair.

use crate::{archive::XA_SECTOR_BYTES, Result};
use serde::Serialize;

const END: usize = XA_SECTOR_BYTES - 4;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Status {
    Absent,
    Valid(u32),
}

pub fn checksum(bytes: &[u8]) -> u32 {
    let mut crc = 0u32;
    for &byte in bytes {
        crc ^= u32::from(byte);
        for _ in 0..8 {
            crc = (crc >> 1) ^ if crc & 1 != 0 { 0xd801_8001 } else { 0 };
        }
    }
    crc
}

fn shape(sector: &[u8]) -> Result<()> {
    if sector.len() != XA_SECTOR_BYTES || sector[2] & 0x20 == 0 {
        return Err("Form 2 EDC requires a 2336-byte extracted Form 2 sector".into());
    }
    if sector[..4] != sector[4..8] {
        return Err("Form 2 EDC: subheader copies disagree".into());
    }
    Ok(())
}

pub fn check_form2(sector: &[u8]) -> Result<Status> {
    shape(sector)?;
    let stored = u32::from_le_bytes(sector[END..].try_into()?);
    if stored == 0 {
        return Ok(Status::Absent);
    }
    let actual = checksum(&sector[..END]);
    if stored != actual {
        return Err(
            format!("Form 2 EDC mismatch: stored {stored:08x}, calculated {actual:08x}").into(),
        );
    }
    Ok(Status::Valid(stored))
}

/// Call only after validating the original sector. Keep an intentionally absent
/// EDC absent; otherwise update its checksum to cover the edited payload.
pub(crate) fn update_form2(sector: &mut [u8], original: Status) -> Result<Status> {
    shape(sector)?;
    let value = if original == Status::Absent {
        0
    } else {
        checksum(&sector[..END])
    };
    sector[END..].copy_from_slice(&value.to_le_bytes());
    check_form2(sector)
}
