use std::collections::HashSet;
use std::fmt;
use std::fs::{self, File};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::{Component, Path, PathBuf};

pub mod checksum;
pub mod cue;
pub mod metadata;

const ISO_SECTOR_SIZE: u64 = 2048;
const RAW_SECTOR_SIZE: u64 = 2352;
const RAW_USER_DATA_OFFSET: u64 = 24;
const PVD_SECTOR: u64 = 16;
const LICENSE_SIZE: usize = 28_032;
const RAW_XA_SIZE: usize = 2_336;

#[derive(Debug)]
pub enum Error {
    Io(std::io::Error),
    InvalidImage(&'static str),
    InvalidImageDetail(String),
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Io(error) => write!(f, "{error}"),
            Self::InvalidImage(message) => write!(f, "invalid disc image: {message}"),
            Self::InvalidImageDetail(message) => write!(f, "invalid disc image: {message}"),
        }
    }
}

impl std::error::Error for Error {}

impl From<std::io::Error> for Error {
    fn from(error: std::io::Error) -> Self {
        Self::Io(error)
    }
}

#[derive(Clone, Copy, Debug)]
struct SectorLayout {
    size: u64,
    data_offset: u64,
}

pub struct Image {
    file: File,
    file_len: u64,
    layout: SectorLayout,
    root: DirectoryRecord,
    cue_path: Option<PathBuf>,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Entry {
    pub path: PathBuf,
    pub lba: u32,
    pub size: u32,
    pub is_directory: bool,
    pub is_xa: bool,
    pub xa: Option<XaAttributes>,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct XaAttributes {
    pub group_id: u16,
    pub user_id: u16,
    pub attributes: u16,
    pub file_number: u8,
}

#[derive(Clone, Debug)]
struct DirectoryRecord {
    extent: u32,
    size: u32,
    flags: u8,
    name: Vec<u8>,
    is_xa: bool,
    xa: Option<XaAttributes>,
}

impl Image {
    pub fn open(path: impl AsRef<Path>) -> Result<Self, Error> {
        let cue_path = path
            .as_ref()
            .extension()
            .is_some_and(|extension| extension.eq_ignore_ascii_case("cue"))
            .then(|| path.as_ref().to_path_buf());
        let image_path = resolve_image_path(path.as_ref())?;
        let mut file = File::open(image_path)?;
        let file_len = file.metadata()?.len();
        let layout = detect_layout(&mut file, file_len)?;
        let pvd = read_sector(&mut file, file_len, layout, PVD_SECTOR)?;
        if pvd[0] != 1 || &pvd[1..6] != b"CD001" || pvd[6] != 1 {
            return Err(Error::InvalidImage(
                "missing ISO9660 primary volume descriptor",
            ));
        }
        let root = parse_record(&pvd[156..])?
            .ok_or(Error::InvalidImage("missing root directory record"))?;
        Ok(Self {
            file,
            file_len,
            layout,
            root,
            cue_path,
        })
    }

    pub fn extract(mut self, output: impl AsRef<Path>) -> Result<Vec<PathBuf>, Error> {
        fs::create_dir_all(output.as_ref())?;
        let mut extracted = Vec::new();
        if let Some(license) = self.read_license()? {
            let path = output.as_ref().join("license_data.dat");
            fs::write(&path, license)?;
            extracted.push(path);
        }
        let mut visited = HashSet::new();
        let root = self.root.clone();
        self.extract_directory(&root, output.as_ref(), &mut visited, &mut extracted)?;
        if let Some(cue_path) = &self.cue_path {
            extracted.extend(cue::extract_audio_tracks(
                &cue::read(cue_path)?,
                output.as_ref(),
            )?);
        }
        extracted.sort();
        Ok(extracted)
    }

    fn read_license(&mut self) -> Result<Option<Vec<u8>>, Error> {
        if self.layout.size != RAW_SECTOR_SIZE {
            return Ok(None);
        }
        let mut license = Vec::with_capacity(LICENSE_SIZE);
        for sector in 0..u64::try_from(LICENSE_SIZE.div_ceil(RAW_XA_SIZE)).unwrap() {
            let offset = sector
                .checked_mul(RAW_SECTOR_SIZE)
                .and_then(|value| value.checked_add(16))
                .ok_or(Error::InvalidImage("license offset overflows"))?;
            self.file.seek(SeekFrom::Start(offset))?;
            let mut bytes = [0; RAW_XA_SIZE];
            self.file.read_exact(&mut bytes)?;
            license.extend_from_slice(&bytes);
        }
        license.truncate(LICENSE_SIZE);
        Ok(Some(license))
    }

    pub fn entries(&mut self) -> Result<Vec<Entry>, Error> {
        let mut entries = Vec::new();
        let mut visited = HashSet::new();
        let root = self.root.clone();
        self.collect_entries(&root, Path::new(""), &mut visited, &mut entries)?;
        entries.sort_by(|left, right| left.path.cmp(&right.path));
        Ok(entries)
    }

    fn collect_entries(
        &mut self,
        directory: &DirectoryRecord,
        parent: &Path,
        visited: &mut HashSet<(u32, u32)>,
        entries: &mut Vec<Entry>,
    ) -> Result<(), Error> {
        if !visited.insert((directory.extent, directory.size)) {
            return Ok(());
        }
        let bytes = self.read_extent(directory.extent, directory.size)?;
        let mut cursor = 0;
        while cursor < bytes.len() {
            let length = bytes[cursor] as usize;
            if length == 0 {
                cursor = ((cursor / ISO_SECTOR_SIZE as usize) + 1) * ISO_SECTOR_SIZE as usize;
                continue;
            }
            let end = cursor
                .checked_add(length)
                .ok_or(Error::InvalidImage("directory record range overflows"))?;
            if end > bytes.len() {
                return Err(Error::InvalidImage("directory record is truncated"));
            }
            if let Some(record) = parse_record(&bytes[cursor..end])? {
                if record.name != [0] && record.name != [1] {
                    let path = parent.join(normalized_name(&record.name)?);
                    let is_directory = record.flags & 0x02 != 0;
                    entries.push(Entry {
                        path: path.clone(),
                        lba: record.extent,
                        size: record.size,
                        is_directory,
                        is_xa: record.is_xa,
                        xa: record.xa,
                    });
                    if is_directory {
                        self.collect_entries(&record, &path, visited, entries)?;
                    }
                }
            }
            cursor = end;
        }
        Ok(())
    }

    fn extract_directory(
        &mut self,
        directory: &DirectoryRecord,
        output: &Path,
        visited: &mut HashSet<(u32, u32)>,
        extracted: &mut Vec<PathBuf>,
    ) -> Result<(), Error> {
        if !visited.insert((directory.extent, directory.size)) {
            return Ok(());
        }
        let bytes = self.read_extent(directory.extent, directory.size)?;
        let mut cursor = 0;
        while cursor < bytes.len() {
            let length = bytes[cursor] as usize;
            if length == 0 {
                cursor = ((cursor / ISO_SECTOR_SIZE as usize) + 1) * ISO_SECTOR_SIZE as usize;
                continue;
            }
            let end = cursor
                .checked_add(length)
                .ok_or(Error::InvalidImage("directory record range overflows"))?;
            if end > bytes.len() {
                return Err(Error::InvalidImage("directory record is truncated"));
            }
            if let Some(record) = parse_record(&bytes[cursor..end])? {
                if record.name != [0] && record.name != [1] {
                    let name = normalized_name(&record.name)?;
                    let path = output.join(name);
                    ensure_beneath(output, &path)?;
                    if record.flags & 0x02 != 0 {
                        fs::create_dir_all(&path)?;
                        self.extract_directory(&record, &path, visited, extracted)?;
                    } else {
                        self.extract_file(&record, &path)?;
                        extracted.push(path);
                    }
                }
            }
            cursor = end;
        }
        Ok(())
    }

    fn extract_file(&mut self, record: &DirectoryRecord, path: &Path) -> Result<(), Error> {
        let bytes = if record.is_xa && self.layout.size == RAW_SECTOR_SIZE {
            self.read_xa_extent(record.extent, record.size)?
        } else {
            self.read_extent(record.extent, record.size)?
        };
        let mut output = File::create(path)?;
        output.write_all(&bytes)?;
        output.flush()?;
        Ok(())
    }

    fn read_xa_extent(&mut self, extent: u32, logical_size: u32) -> Result<Vec<u8>, Error> {
        let sectors = u64::from(logical_size).div_ceil(RAW_XA_SIZE as u64);
        let mut bytes = Vec::with_capacity(sectors as usize * RAW_XA_SIZE);
        for sector_index in 0..sectors {
            let sector = u64::from(extent)
                .checked_add(sector_index)
                .ok_or(Error::InvalidImage("extent overflows"))?;
            let offset = sector
                .checked_mul(RAW_SECTOR_SIZE)
                .and_then(|value| value.checked_add(16))
                .ok_or(Error::InvalidImage("XA sector offset overflows"))?;
            self.file.seek(SeekFrom::Start(offset))?;
            let mut sector_bytes = [0; RAW_XA_SIZE];
            self.file.read_exact(&mut sector_bytes)?;
            bytes.extend_from_slice(&sector_bytes);
        }
        Ok(bytes)
    }

    fn read_extent(&mut self, extent: u32, size: u32) -> Result<Vec<u8>, Error> {
        let sectors = u64::from(size).div_ceil(ISO_SECTOR_SIZE);
        let mut bytes = Vec::with_capacity(size as usize);
        for sector_index in 0..sectors {
            let sector = u64::from(extent)
                .checked_add(sector_index)
                .ok_or(Error::InvalidImage("extent overflows"))?;
            bytes.extend_from_slice(&read_sector(
                &mut self.file,
                self.file_len,
                self.layout,
                sector,
            )?);
        }
        bytes.truncate(size as usize);
        Ok(bytes)
    }
}

fn resolve_image_path(path: &Path) -> Result<PathBuf, Error> {
    if !path.exists() {
        return Err(Error::InvalidImageDetail(format!(
            "input not found: {}",
            path.display()
        )));
    }
    if !path
        .extension()
        .is_some_and(|extension| extension.eq_ignore_ascii_case("cue"))
    {
        return Ok(path.to_path_buf());
    }
    let sheet = cue::read(path)?;
    let data_track = sheet
        .tracks
        .iter()
        .find(|track| track.mode != cue::TrackMode::Audio)
        .ok_or(Error::InvalidImage("cue sheet has no data track"))?;
    if data_track.index01 != 0 {
        return Err(Error::InvalidImage(
            "data track with nonzero INDEX 01 is not supported yet",
        ));
    }
    let resolved = data_track.file.clone();
    if !resolved.exists() {
        return Err(Error::InvalidImageDetail(format!(
            "cue data file not found: {}",
            resolved.display()
        )));
    }
    Ok(resolved)
}

fn detect_layout(file: &mut File, file_len: u64) -> Result<SectorLayout, Error> {
    for layout in [
        SectorLayout {
            size: ISO_SECTOR_SIZE,
            data_offset: 0,
        },
        SectorLayout {
            size: RAW_SECTOR_SIZE,
            data_offset: RAW_USER_DATA_OFFSET,
        },
        SectorLayout {
            size: RAW_SECTOR_SIZE,
            data_offset: 16,
        },
    ] {
        if let Ok(sector) = read_sector(file, file_len, layout, PVD_SECTOR) {
            if sector[0] == 1 && &sector[1..6] == b"CD001" && sector[6] == 1 {
                return Ok(layout);
            }
        }
    }
    Err(Error::InvalidImage(
        "unsupported sector layout or missing ISO9660 PVD",
    ))
}

fn read_sector(
    file: &mut File,
    file_len: u64,
    layout: SectorLayout,
    sector: u64,
) -> Result<[u8; ISO_SECTOR_SIZE as usize], Error> {
    let offset = sector
        .checked_mul(layout.size)
        .and_then(|value| value.checked_add(layout.data_offset))
        .ok_or(Error::InvalidImage("sector offset overflows"))?;
    let end = offset
        .checked_add(ISO_SECTOR_SIZE)
        .ok_or(Error::InvalidImage("sector range overflows"))?;
    if end > file_len {
        return Err(Error::InvalidImage("sector extends beyond image"));
    }
    file.seek(SeekFrom::Start(offset))?;
    let mut bytes = [0; ISO_SECTOR_SIZE as usize];
    file.read_exact(&mut bytes)?;
    Ok(bytes)
}

fn parse_record(bytes: &[u8]) -> Result<Option<DirectoryRecord>, Error> {
    let Some(&length) = bytes.first() else {
        return Ok(None);
    };
    if length == 0 {
        return Ok(None);
    }
    let length = length as usize;
    if length > bytes.len() || length < 34 {
        return Err(Error::InvalidImage(
            "invalid ISO9660 directory record length",
        ));
    }
    let name_len = bytes[32] as usize;
    let name_end = 33_usize
        .checked_add(name_len)
        .ok_or(Error::InvalidImage("directory name range overflows"))?;
    if name_end > length {
        return Err(Error::InvalidImage("directory name is truncated"));
    }
    let extent = u32::from_le_bytes(bytes[2..6].try_into().unwrap());
    let extent_be = u32::from_be_bytes(bytes[6..10].try_into().unwrap());
    let size = u32::from_le_bytes(bytes[10..14].try_into().unwrap());
    let size_be = u32::from_be_bytes(bytes[14..18].try_into().unwrap());
    if extent != extent_be || size != size_be {
        return Err(Error::InvalidImage("ISO9660 both-endian fields disagree"));
    }
    let system_use = 33 + name_len + usize::from(name_len.is_multiple_of(2));
    let xa = parse_xa_attributes(&bytes[system_use..length]);
    let is_xa = xa.is_some_and(|attributes| {
        let kind = (attributes.attributes >> 8) as u8;
        attributes.file_number != 0 || (kind & 0x10 != 0 && kind & 0x08 == 0)
    });
    Ok(Some(DirectoryRecord {
        extent,
        size,
        flags: bytes[25],
        name: bytes[33..name_end].to_vec(),
        is_xa,
        xa,
    }))
}

fn parse_xa_attributes(bytes: &[u8]) -> Option<XaAttributes> {
    if bytes.len() < 14 || &bytes[6..8] != b"XA" {
        return None;
    }
    Some(XaAttributes {
        group_id: u16::from_be_bytes(bytes[0..2].try_into().ok()?),
        user_id: u16::from_be_bytes(bytes[2..4].try_into().ok()?),
        attributes: u16::from_be_bytes(bytes[4..6].try_into().ok()?),
        file_number: bytes[8],
    })
}

fn normalized_name(raw: &[u8]) -> Result<String, Error> {
    let decoded = std::str::from_utf8(raw)
        .map_err(|_| Error::InvalidImage("non-UTF-8 ISO9660 identifier"))?;
    let name = decoded
        .split(';')
        .next()
        .unwrap_or(decoded)
        .trim_end_matches('.');
    if name.is_empty()
        || name == "."
        || name == ".."
        || Path::new(name)
            .components()
            .any(|component| !matches!(component, Component::Normal(_)))
    {
        return Err(Error::InvalidImage("unsafe ISO9660 identifier"));
    }
    Ok(name.to_string())
}

fn ensure_beneath(root: &Path, path: &Path) -> Result<(), Error> {
    if !path.starts_with(root) {
        return Err(Error::InvalidImage("output path escapes extraction root"));
    }
    Ok(())
}
