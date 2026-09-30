//! Immutable media snapshots; XA keeps every multiplexed and unrelated sector.

pub mod disc;
pub(crate) mod packing;

use crate::Result;
use emi_ex_v2::image::ArchiveImage;
use std::path::Path;

pub const XA_SECTOR_BYTES: usize = 2336;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct XaSector {
    pub index: usize,
    pub file: u8,
    pub channel: u8,
    pub submode: u8,
    pub coding: u8,
}

impl XaSector {
    pub fn is_audio(&self) -> bool {
        self.submode & 0x04 != 0
    }
}

#[derive(Clone, Debug)]
pub struct XaImage {
    bytes: Vec<u8>,
    sectors: Vec<XaSector>,
}

impl XaImage {
    pub fn from_bytes(bytes: Vec<u8>) -> Result<Self> {
        if bytes.is_empty() || !bytes.len().is_multiple_of(XA_SECTOR_BYTES) {
            return Err("XA image must contain whole 2336-byte extracted sectors".into());
        }
        let mut sectors = Vec::with_capacity(bytes.len() / XA_SECTOR_BYTES);
        for (index, sector) in bytes.as_chunks::<XA_SECTOR_BYTES>().0.iter().enumerate() {
            if sector[..4] != sector[4..8] {
                return Err(format!("XA sector {index}: duplicate subheaders disagree").into());
            }
            sectors.push(XaSector {
                index,
                file: sector[0],
                channel: sector[1],
                submode: sector[2],
                coding: sector[3],
            });
        }
        Ok(Self { bytes, sectors })
    }

    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    pub fn sectors(&self) -> &[XaSector] {
        &self.sectors
    }

    pub fn sector(&self, index: usize) -> Result<&[u8]> {
        if index >= self.sectors.len() {
            return Err(format!(
                "XA sector {index} out of range ({} sectors)",
                self.sectors.len()
            )
            .into());
        }
        Ok(&self.bytes[index * XA_SECTOR_BYTES..(index + 1) * XA_SECTOR_BYTES])
    }
}

pub enum MediaImage {
    Emi(ArchiveImage),
    Xa(XaImage),
}

impl MediaImage {
    pub fn read(path: &Path) -> Result<Self> {
        let bytes = std::fs::read(path)?;
        match path
            .extension()
            .and_then(|s| s.to_str())
            .map(str::to_ascii_lowercase)
            .as_deref()
        {
            Some("emi") => Ok(Self::Emi(ArchiveImage::from_bytes(bytes)?)),
            Some("str" | "xa") => Ok(Self::Xa(XaImage::from_bytes(bytes)?)),
            _ => Err(format!("{}: expected EMI, STR, or XA media", path.display()).into()),
        }
    }

    pub fn bytes(&self) -> &[u8] {
        match self {
            Self::Emi(image) => image.bytes(),
            Self::Xa(image) => image.bytes(),
        }
    }
}
