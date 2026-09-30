//! Immutable retail-size BIOS storage. Identification is not runtime acceptance.
use super::bus::{BusError, Width};
use crate::{digest::sha256_hex, Result};

pub const ROM_BYTES: usize = 512 * 1024;
pub const US_SHA256: &str = "11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef";

pub struct Image {
    bytes: Vec<u8>,
    sha256: String,
}
impl Image {
    pub fn from_bytes(bytes: Vec<u8>) -> Result<Self> {
        if bytes.len() != ROM_BYTES {
            return Err(
                "BIOS image must contain exactly 512 KiB; other layouts are unsupported".into(),
            );
        }
        let sha256 = sha256_hex(&bytes);
        Ok(Self { bytes, sha256 })
    }
    pub fn sha256(&self) -> &str {
        &self.sha256
    }
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }
    /// Only the documented physical, cached and uncached windows, no broad mask.
    pub fn offset(address: u32) -> Option<usize> {
        match address {
            0x1fc0_0000..=0x1fc7_ffff => Some((address - 0x1fc0_0000) as usize),
            0x9fc0_0000..=0x9fc7_ffff => Some((address - 0x9fc0_0000) as usize),
            0xbfc0_0000..=0xbfc7_ffff => Some((address - 0xbfc0_0000) as usize),
            _ => None,
        }
    }
    pub fn read(&self, address: u32, width: Width) -> std::result::Result<u32, BusError> {
        let fail = || BusError {
            address,
            detail: "unaligned or out-of-range BIOS ROM read".into(),
        };
        let offset = Self::offset(address).ok_or_else(fail)?;
        if offset & (width.bytes() - 1) != 0 || width.bytes() > ROM_BYTES - offset {
            return Err(fail());
        }
        let mut value = [0; 4];
        value[..width.bytes()].copy_from_slice(&self.bytes[offset..offset + width.bytes()]);
        Ok(u32::from_le_bytes(value))
    }
}
