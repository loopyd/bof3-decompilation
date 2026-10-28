//! Owned raw-sector backing extents. Drive TOC metadata selects valid positions.
use crate::Result;
use std::{
    fs::File,
    io::{Read, Seek, SeekFrom},
};

pub struct Extent {
    pub file: File,
    pub first_lba: i32,
    pub file_sector: u64,
    pub sectors: u32,
}

pub struct Media {
    extents: Vec<Extent>,
}

impl Media {
    /// Bind ordered, nonoverlapping raw 2352-byte program extents. Offsets and
    /// lengths are explicit; this does not parse a CUE or infer a TOC.
    pub fn new(extents: Vec<Extent>) -> Result<Self> {
        if !(1..=99).contains(&extents.len()) {
            return Err("CD media requires 1..99 raw backing extents".into());
        }
        let mut previous_end = 0;
        for extent in &extents {
            let end = i64::from(extent.first_lba) + i64::from(extent.sectors);
            let file_end = extent
                .file_sector
                .checked_add(u64::from(extent.sectors))
                .and_then(|sectors| sectors.checked_mul(2352))
                .ok_or("CD media extent overflow")?;
            if extent.first_lba < previous_end
                || extent.sectors == 0
                || end > i64::from(i32::MAX)
                || file_end > extent.file.metadata()?.len()
            {
                return Err(
                    "CD media requires ordered nonnegative extents within their files".into(),
                );
            }
            previous_end = end as i32;
        }
        Ok(Self { extents })
    }

    pub(crate) fn read_sector(&mut self, lba: i32) -> Result<[u8; 2352]> {
        let extent = self
            .extents
            .iter_mut()
            .find(|extent| {
                lba >= extent.first_lba
                    && i64::from(lba) < i64::from(extent.first_lba) + i64::from(extent.sectors)
            })
            .ok_or("CD media position has no backing extent")?;
        let sector = extent.file_sector + (lba - extent.first_lba) as u64;
        extent.file.seek(SeekFrom::Start(sector * 2352))?;
        let mut bytes = [0; 2352];
        extent.file.read_exact(&mut bytes)?;
        Ok(bytes)
    }
}
