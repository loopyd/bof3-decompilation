//! Read-only Mode-2/2352 disc access. ISO lengths count 2048-byte logical blocks;
//! extracting XA retains 2336 bytes per sector, including subheaders and EDC.

use crate::{archive::XaImage, Result};
use std::{
    collections::{BTreeMap, BTreeSet},
    fs::File,
    io::{Read, Seek, SeekFrom},
    path::Path,
};

pub const RAW_SECTOR_BYTES: u64 = 2352;

#[derive(Clone, Debug)]
pub struct DiscFile {
    pub path: String,
    pub lba: u32,
    pub logical_bytes: u32,
}

impl DiscFile {
    pub fn sector_count(&self) -> usize {
        (self.logical_bytes as usize).div_ceil(2048)
    }
}

pub struct DiscImage {
    file: File,
    sectors: u64,
    files: BTreeMap<String, DiscFile>,
}

impl DiscImage {
    pub fn open(path: &Path) -> Result<Self> {
        let file = File::open(path)?;
        let bytes = file.metadata()?.len();
        if bytes == 0 || !bytes.is_multiple_of(RAW_SECTOR_BYTES) {
            return Err("disc: expected whole 2352-byte raw sectors".into());
        }
        let mut image = Self {
            file,
            sectors: bytes / RAW_SECTOR_BYTES,
            files: BTreeMap::new(),
        };
        let descriptor = image.read_data(16, 2048)?;
        if &descriptor[..7] != b"\x01CD001\x01" || both16(&descriptor, 128)? != 2048 {
            return Err("disc: unsupported ISO primary descriptor or logical block size".into());
        }
        let root = record(&descriptor[156..])?;
        if !root.directory {
            return Err("disc: ISO root record is not a directory".into());
        }
        image.directory(root.lba, root.bytes, "", &mut BTreeSet::new(), 0)?;
        Ok(image)
    }

    pub fn files(&self) -> &BTreeMap<String, DiscFile> {
        &self.files
    }

    pub fn read_file(&mut self, path: &str) -> Result<Vec<u8>> {
        let entry = self
            .files
            .get(path)
            .ok_or_else(|| format!("disc file not found: {path}"))?
            .clone();
        self.read_data(entry.lba, entry.logical_bytes as usize)
    }

    pub fn read_xa(&mut self, path: &str) -> Result<XaImage> {
        let entry = self
            .files
            .get(path)
            .ok_or_else(|| format!("disc file not found: {path}"))?
            .clone();
        let count = entry.sector_count();
        self.check_extent(entry.lba, count)?;
        let mut bytes = Vec::with_capacity(count * 2336);
        for index in 0..count {
            let sector = self.sector(u64::from(entry.lba) + index as u64)?;
            bytes.extend_from_slice(&sector[16..]);
        }
        XaImage::from_bytes(bytes)
    }

    fn check_extent(&self, lba: u32, count: usize) -> Result<()> {
        if u64::from(lba) + count as u64 > self.sectors {
            return Err(format!("disc extent at LBA {lba} ({count} sectors) exceeds track").into());
        }
        Ok(())
    }

    /// Read an original Mode-2 sector, including sync and position header.
    pub fn read_sector(&mut self, lba: u32) -> Result<[u8; 2352]> {
        self.sector(u64::from(lba))
    }
    fn sector(&mut self, lba: u64) -> Result<[u8; 2352]> {
        if lba >= self.sectors {
            return Err(format!("disc LBA {lba} exceeds track").into());
        }
        self.file.seek(SeekFrom::Start(lba * RAW_SECTOR_BYTES))?;
        let mut sector = [0; 2352];
        self.file.read_exact(&mut sector)?;
        if sector[..12] != [0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0]
            || sector[15] != 2
            || sector[16..20] != sector[20..24]
        {
            return Err(
                format!("disc LBA {lba}: invalid sync, mode, or duplicated subheader").into(),
            );
        }
        Ok(sector)
    }

    fn read_data(&mut self, lba: u32, bytes: usize) -> Result<Vec<u8>> {
        let count = bytes.div_ceil(2048);
        self.check_extent(lba, count)?;
        let mut result = Vec::with_capacity(count * 2048);
        for index in 0..count {
            let sector = self.sector(u64::from(lba) + index as u64)?;
            if sector[18] & 0x20 != 0 {
                return Err("disc: form-2 extent requires XA sector extraction".into());
            }
            result.extend_from_slice(&sector[24..2072]);
        }
        result.truncate(bytes);
        Ok(result)
    }

    fn directory(
        &mut self,
        lba: u32,
        size: u32,
        prefix: &str,
        visited: &mut BTreeSet<u32>,
        depth: usize,
    ) -> Result<()> {
        if depth > 32 || !visited.insert(lba) {
            return Err("disc: cyclic, shared, or excessively deep directory".into());
        }
        let bytes = self.read_data(lba, size as usize)?;
        let mut pos = 0;
        while pos < bytes.len() {
            let length = usize::from(bytes[pos]);
            if length == 0 {
                pos = (pos / 2048 + 1) * 2048;
                continue;
            }
            if length > bytes.len() - pos || pos % 2048 + length > 2048 {
                return Err("disc: truncated or cross-sector directory record".into());
            }
            let entry = record(&bytes[pos..pos + length])?;
            pos += length;
            if entry.name == [0] || entry.name == [1] {
                continue;
            }
            let text = std::str::from_utf8(&entry.name)?;
            let name = text.split(';').next().ok_or("disc: empty ISO filename")?;
            if name.is_empty() || matches!(name, "." | "..") || name.contains(['/', '\\']) {
                return Err("disc: unsupported ISO filename".into());
            }
            let path = if prefix.is_empty() {
                name.to_owned()
            } else {
                format!("{prefix}/{name}")
            };
            self.check_extent(entry.lba, (entry.bytes as usize).div_ceil(2048))?;
            if entry.directory {
                self.directory(entry.lba, entry.bytes, &path, visited, depth + 1)?;
            } else if self
                .files
                .insert(
                    path.clone(),
                    DiscFile {
                        path,
                        lba: entry.lba,
                        logical_bytes: entry.bytes,
                    },
                )
                .is_some()
            {
                return Err("disc: duplicate file identity".into());
            }
        }
        Ok(())
    }
}

struct Record {
    lba: u32,
    bytes: u32,
    directory: bool,
    name: Vec<u8>,
}

fn record(bytes: &[u8]) -> Result<Record> {
    if bytes.len() < 34
        || bytes[0] < 34
        || usize::from(bytes[0]) > bytes.len()
        || 33 + usize::from(bytes[32]) > usize::from(bytes[0])
    {
        return Err("disc: malformed ISO directory record".into());
    }
    if bytes[1] != 0 || bytes[25] & 0x80 != 0 || bytes[26] != 0 || bytes[27] != 0 {
        return Err(
            "disc: extended, multi-extent, or ISO-interleaved files are unsupported".into(),
        );
    }
    Ok(Record {
        lba: both32(bytes, 2)?,
        bytes: both32(bytes, 10)?,
        directory: bytes[25] & 2 != 0,
        name: bytes[33..33 + usize::from(bytes[32])].to_vec(),
    })
}

fn both16(bytes: &[u8], at: usize) -> Result<u16> {
    let little = u16::from_le_bytes(bytes[at..at + 2].try_into().unwrap());
    let big = u16::from_be_bytes(bytes[at + 2..at + 4].try_into().unwrap());
    if little != big {
        return Err("disc: conflicting ISO byte-order copies".into());
    }
    Ok(little)
}

fn both32(bytes: &[u8], at: usize) -> Result<u32> {
    let little = u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap());
    let big = u32::from_be_bytes(bytes[at + 4..at + 8].try_into().unwrap());
    if little != big {
        return Err("disc: conflicting ISO byte-order copies".into());
    }
    Ok(little)
}
