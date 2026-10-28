//! Native primary DATA backing and independently selected raw CDDA extents.
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
    data: File,
    data_cache: Option<[u8; 2352]>,
    audio: Vec<Extent>,
}

pub(crate) enum DataSector {
    Complete([u8; 2352]),
    Retained([u8; 2352]),
}

impl Media {
    /// Bind the primary raw DATA file and ordered CDDA program extents. DATA
    /// uses global LBAs in that file, independently of TOC track selection.
    /// CDDA offsets are explicit; this does not parse a CUE or infer a TOC.
    pub fn new(data: File, audio: Vec<Extent>) -> Result<Self> {
        let data_bytes = data.metadata()?.len();
        if data_bytes == 0 || !data_bytes.is_multiple_of(2352) || audio.len() > 99 {
            return Err(
                "CD media requires whole raw DATA sectors and at most 99 CDDA extents".into(),
            );
        }
        let mut previous_end = 0;
        for extent in &audio {
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
        Ok(Self {
            data,
            data_cache: None,
            audio,
        })
    }

    pub(crate) fn read_data_sector(&mut self, lba: i32) -> Result<DataSector> {
        if lba < 0 {
            return Err("CD DATA backing requires a nonnegative global LBA".into());
        }
        self.data.seek(SeekFrom::Start(lba as u64 * 2352))?;
        let mut bytes = [0; 2352];
        match self.data.read(&mut bytes)? {
            2352 => {
                self.data_cache = Some(bytes);
                Ok(DataSector::Complete(bytes))
            }
            0 => self
                .data_cache
                .map(DataSector::Retained)
                .ok_or_else(|| "CD DATA EOF requires a previously defined complete buffer".into()),
            _ => Err("CD DATA partial positive reads are not implemented".into()),
        }
    }

    pub(crate) fn read_audio_sector(&mut self, lba: i32) -> Result<[u8; 2352]> {
        let extent = self
            .audio
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
