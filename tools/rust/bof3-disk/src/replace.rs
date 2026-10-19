use std::fs::{self, OpenOptions};
use std::io::{self, Read, Seek, SeekFrom, Write};
use std::path::{Component, Path};

use crate::{parse_record, Error, Image, ISO_SECTOR_SIZE};

mod sector;

/// Copy an image and replace one ordinary ISO9660 file without moving any extent.
pub fn file(input: &Path, output: &Path, path: &Path, replacement: &Path) -> Result<usize, Error> {
    if output
        .extension()
        .is_some_and(|ext| ext.eq_ignore_ascii_case("cue"))
    {
        return Err(Error::InvalidInput(
            "output must be an image, not a CUE sheet",
        ));
    }
    let mut image = Image::open(input)?;
    let record = image.lookup_replacement(path)?;
    let size = fs::metadata(replacement)?.len();
    if size != u64::from(record.size) || size > 64 * 1024 * 1024 {
        return Err(Error::InvalidInput(
            "replacement must match the ISO file size and fit within 64 MiB",
        ));
    }
    let bytes = fs::read(replacement)?;
    if bytes.len() as u64 != size {
        return Err(Error::InvalidInput(
            "replacement size changed while reading",
        ));
    }
    let start = u64::from(record.extent);
    let end = start + u64::from(record.size).div_ceil(ISO_SECTOR_SIZE);
    if start < 18 || record.size == 0 {
        return Err(Error::InvalidInput(
            "empty or reserved extents cannot be replaced",
        ));
    }
    let root_start = u64::from(image.root.extent);
    let root_end = root_start + u64::from(image.root.size).div_ceil(ISO_SECTOR_SIZE);
    if start < root_end && root_start < end {
        return Err(Error::InvalidInput("file overlaps the root directory"));
    }
    for entry in image.entries()? {
        if entry.path == path {
            continue;
        }
        let other_start = u64::from(entry.lba);
        let other_end = other_start + u64::from(entry.size).div_ceil(ISO_SECTOR_SIZE);
        if start < other_end && other_start < end {
            return Err(Error::InvalidInput(
                "file shares sectors with another extent",
            ));
        }
    }
    let mut patches = Vec::new();
    for (index, chunk) in bytes.chunks(ISO_SECTOR_SIZE as usize).enumerate() {
        let lba = start + index as u64;
        let offset = lba * image.layout.size;
        let mut original = vec![0; image.layout.size as usize];
        if offset + image.layout.size > image.file_len {
            return Err(Error::InvalidImage("replacement extent exceeds image"));
        }
        image.file.seek(SeekFrom::Start(offset))?;
        image.file.read_exact(&mut original)?;
        if image.layout.size == 2352 {
            sector::validate(&original)?;
        }
        let mut edited = original.clone();
        let data = image.layout.data_offset as usize;
        edited[data..data + chunk.len()].copy_from_slice(chunk);
        if edited == original {
            continue;
        }
        if image.layout.size == 2352 {
            sector::repair(&mut edited);
        }
        patches.push((offset, edited));
    }
    // Validation completes before creating a destination. Existing paths, including
    // symlinks and hard links to an input, fail at create_new.
    let mut destination = OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(output)?;
    image.file.seek(SeekFrom::Start(0))?;
    io::copy(&mut image.file, &mut destination)?;
    for (offset, bytes) in &patches {
        destination.seek(SeekFrom::Start(*offset))?;
        destination.write_all(bytes)?;
    }
    destination.flush()?;
    Ok(patches.len())
}

impl Image {
    fn lookup_replacement(&mut self, path: &Path) -> Result<crate::DirectoryRecord, Error> {
        let names = path
            .components()
            .map(|component| match component {
                Component::Normal(name) => name
                    .to_str()
                    .ok_or(Error::InvalidInput("ISO path must be UTF-8")),
                _ => Err(Error::InvalidInput(
                    "ISO path must be relative with ordinary components",
                )),
            })
            .collect::<Result<Vec<_>, _>>()?;
        if names.is_empty() {
            return Err(Error::InvalidInput("ISO path is empty"));
        }
        let mut directory = self.root.clone();
        for (index, name) in names.iter().enumerate() {
            let bytes = self.read_extent(directory.extent, directory.size)?;
            let mut cursor = 0;
            let mut matches = Vec::new();
            while cursor < bytes.len() {
                let length = bytes[cursor] as usize;
                if length == 0 {
                    cursor = (cursor / 2048 + 1) * 2048;
                    continue;
                }
                let end = cursor + length;
                if end > bytes.len() {
                    return Err(Error::InvalidImage("directory record is truncated"));
                }
                let row = &bytes[cursor..end];
                if let Some(record) = parse_record(row)? {
                    if record.name != [0]
                        && record.name != [1]
                        && crate::normalized_name(&record.name)? == *name
                    {
                        if row[1] != 0 || row[26] != 0 || row[27] != 0 || record.flags & !0x03 != 0
                        {
                            return Err(Error::InvalidInput("extended-attribute, interleaved, associated or multi-extent records cannot be replaced"));
                        }
                        matches.push(record);
                    }
                }
                cursor = end;
            }
            if matches.len() != 1 {
                return Err(Error::InvalidInput(
                    "ISO path must identify exactly one file",
                ));
            }
            directory = matches.pop().unwrap();
            let last = index + 1 == names.len();
            if last {
                if directory.flags & 2 != 0 || directory.is_xa {
                    return Err(Error::InvalidInput(
                        "directories and XA files cannot be replaced",
                    ));
                }
                return Ok(directory);
            }
            if directory.flags & 2 == 0 {
                return Err(Error::InvalidInput("ISO path ancestor is not a directory"));
            }
        }
        unreachable!()
    }
}
