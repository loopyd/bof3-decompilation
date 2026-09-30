//! Owned EMI images for preservation-aware consumers. No output is published here.

use crate::{parse_layout, Entry, Error};
use std::collections::BTreeSet;
use std::io::Cursor;

/// An immutable snapshot: metadata and payloads always describe the same bytes.
#[derive(Clone, Debug)]
pub struct ArchiveImage {
    bytes: Vec<u8>,
    version: u32,
    entries: Vec<Entry>,
}

impl ArchiveImage {
    pub fn from_bytes(bytes: Vec<u8>) -> Result<Self, Error> {
        let (version, entries) = parse_layout(&mut Cursor::new(&bytes), bytes.len() as u64)?;
        Ok(Self {
            bytes,
            version,
            entries,
        })
    }

    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    pub fn version(&self) -> u32 {
        self.version
    }

    pub fn entries(&self) -> &[Entry] {
        &self.entries
    }

    pub fn entry(&self, index: usize) -> Result<&[u8], Error> {
        let entry = self.entries.get(index).ok_or(Error::EntryOutOfRange {
            index,
            count: self.entries.len(),
        })?;
        let start = entry.offset as usize;
        Ok(&self.bytes[start..start + entry.size as usize])
    }

    /// Replace payloads without relocating entries or altering unrelated bytes.
    ///
    /// Growth may consume only the entry's existing sector-alignment space, not
    /// a later entry or opaque trailer. Shrinking leaves former payload bytes as
    /// preserved padding. Changes that alter the rounded allocation are rejected.
    /// An unchanged payload preserves even a stale cached first word verbatim.
    pub fn replace_entries(&self, replacements: &[(usize, &[u8])]) -> Result<Self, Error> {
        let mut seen = BTreeSet::new();
        for &(index, payload) in replacements {
            let original = self.entry(index)?;
            if !seen.insert(index) {
                return Err(Error::InvalidArchive("duplicate replacement entry"));
            }
            if original == payload {
                continue;
            }
            let size = u32::try_from(payload.len())
                .map_err(|_| Error::InvalidArchive("entry exceeds 4 GiB"))?;
            let entry = &self.entries[index];
            if crate::align_sector(u64::from(size))? != crate::align_sector(u64::from(entry.size))?
            {
                return Err(Error::InvalidArchive(
                    "replacement changes sector allocation; relocation required",
                ));
            }
            if entry.offset + u64::from(size) > self.bytes.len() as u64 {
                return Err(Error::InvalidArchive(
                    "replacement exceeds available image bytes",
                ));
            }
        }
        let mut bytes = self.bytes.clone();
        for &(index, payload) in replacements {
            if self.entry(index)? == payload {
                continue;
            }
            let entry = &self.entries[index];
            let start = entry.offset as usize;
            bytes[start..start + payload.len()].copy_from_slice(payload);
            let table = 0x10 + index * 0x10;
            bytes[table..table + 4].copy_from_slice(&(payload.len() as u32).to_le_bytes());
            let mut first = [0; 4];
            let count = payload.len().min(4);
            first[..count].copy_from_slice(&payload[..count]);
            bytes[table + 8..table + 12].copy_from_slice(&first);
        }
        Self::from_bytes(bytes)
    }
}
