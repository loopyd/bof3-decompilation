use std::fmt;
use std::fs::{self, File};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

const MAGIC: &[u8; 8] = b"MATH_TBL";
const SECTOR_SIZE: u64 = 0x800;
const TOC_ENTRY_SIZE: u64 = 0x10;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Entry {
    pub size: u32,
    pub offset: u64,
    pub ram_ptr: u32,
    pub first4: u32,
    pub file_type: u16,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct PackEntry {
    pub path: PathBuf,
    pub file_type: u16,
    pub ram_ptr: u32,
}

#[derive(Clone, Debug, Default, Eq, PartialEq)]
pub struct PackFolderOptions {
    pub default_type: Option<u16>,
    pub include_patterns: Vec<String>,
    pub exclude_patterns: Vec<String>,
}

#[derive(Debug, Deserialize, Serialize)]
struct Manifest {
    archive_version: u32,
    entries: Vec<ManifestEntry>,
    version: u32,
}

#[derive(Debug, Deserialize, Serialize)]
struct ManifestEntry {
    first4: u32,
    index: usize,
    name: String,
    ram_ptr: u32,
    size: u32,
    #[serde(rename = "type")]
    file_type: u16,
}

#[derive(Debug)]
pub enum Error {
    Io(std::io::Error),
    InvalidArchive(&'static str),
    EntryOutOfRange { index: usize, count: usize },
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Io(error) => write!(f, "{error}"),
            Self::InvalidArchive(message) => write!(f, "invalid EMI archive: {message}"),
            Self::EntryOutOfRange { index, count } => {
                write!(
                    f,
                    "entry index {index} is out of range (archive has {count} entries)"
                )
            }
        }
    }
}

impl std::error::Error for Error {}

impl From<std::io::Error> for Error {
    fn from(error: std::io::Error) -> Self {
        Self::Io(error)
    }
}

pub struct Archive {
    source: PathBuf,
    version: u32,
    entries: Vec<Entry>,
}

impl Archive {
    pub fn open(path: impl AsRef<Path>) -> Result<Self, Error> {
        let source = path.as_ref().to_path_buf();
        let mut file = File::open(&source)?;
        let file_len = file.metadata()?.len();
        let count = read_u32(&mut file)? as usize;
        let version = read_u32(&mut file)?;
        let mut magic = [0; 8];
        file.read_exact(&mut magic)?;
        if &magic != MAGIC {
            return Err(Error::InvalidArchive("bad magic"));
        }

        let toc_size = (count as u64)
            .checked_mul(TOC_ENTRY_SIZE)
            .ok_or(Error::InvalidArchive("entry table overflows"))?;
        let toc_end = 0x10_u64
            .checked_add(toc_size)
            .ok_or(Error::InvalidArchive("entry table overflows"))?;
        if toc_end > SECTOR_SIZE || toc_end > file_len {
            return Err(Error::InvalidArchive(
                "entry table does not fit before data",
            ));
        }

        let mut offset = SECTOR_SIZE;
        let mut entries = Vec::with_capacity(count);
        for _ in 0..count {
            let size = read_u32(&mut file)?;
            let ram_ptr = read_u32(&mut file)?;
            let first4 = read_u32(&mut file)?;
            let file_type = read_u16(&mut file)?;
            let _padding = read_u16(&mut file)?;
            let end = offset
                .checked_add(size as u64)
                .ok_or(Error::InvalidArchive("entry range overflows"))?;
            if end > file_len {
                return Err(Error::InvalidArchive("entry extends beyond end of file"));
            }
            entries.push(Entry {
                size,
                offset,
                ram_ptr,
                first4,
                file_type,
            });
            offset = align_sector(end)?;
        }

        Ok(Self {
            source,
            version,
            entries,
        })
    }

    pub fn version(&self) -> u32 {
        self.version
    }

    pub fn entries(&self) -> &[Entry] {
        &self.entries
    }

    pub fn extract_entry(&self, index: usize, path: impl AsRef<Path>) -> Result<(), Error> {
        let entry = self.entries.get(index).ok_or(Error::EntryOutOfRange {
            index,
            count: self.entries.len(),
        })?;
        let mut source = File::open(&self.source)?;
        source.seek(SeekFrom::Start(entry.offset))?;
        let mut output = create_output(path.as_ref())?;
        let copied = std::io::copy(&mut source.take(entry.size as u64), &mut output)?;
        if copied != entry.size as u64 {
            return Err(Error::InvalidArchive(
                "entry was truncated while extracting",
            ));
        }
        output.flush()?;
        Ok(())
    }

    pub fn manifest_json(&self, typed: bool) -> String {
        let manifest = Manifest {
            archive_version: self.version,
            entries: self
                .entries
                .iter()
                .enumerate()
                .map(|(index, entry)| ManifestEntry {
                    first4: entry.first4,
                    index,
                    name: format!(
                        "{index}.{}",
                        if typed {
                            type_extension(entry.file_type)
                        } else {
                            "bin"
                        }
                    ),
                    ram_ptr: entry.ram_ptr,
                    size: entry.size,
                    file_type: entry.file_type,
                })
                .collect(),
            version: 1,
        };
        serde_json::to_string_pretty(&manifest).expect("manifest model is serializable")
    }

    pub fn extract_all(&self, directory: impl AsRef<Path>, typed: bool) -> Result<(), Error> {
        fs::create_dir_all(directory.as_ref())?;
        for (index, entry) in self.entries.iter().enumerate() {
            let extension = if typed {
                type_extension(entry.file_type)
            } else {
                "bin"
            };
            self.extract_entry(
                index,
                directory.as_ref().join(format!("{index}.{extension}")),
            )?;
        }
        Ok(())
    }
}

pub fn pack(
    output: impl AsRef<Path>,
    inputs: &[PackEntry],
    archive_version: u32,
) -> Result<(), Error> {
    if inputs.is_empty() || inputs.len() > 255 {
        return Err(Error::InvalidArchive("pack requires 1 through 255 entries"));
    }
    let toc_end = 0x10_u64
        .checked_add((inputs.len() as u64) * TOC_ENTRY_SIZE)
        .ok_or(Error::InvalidArchive("entry table overflows"))?;
    if toc_end > SECTOR_SIZE {
        return Err(Error::InvalidArchive(
            "entry table does not fit before data",
        ));
    }

    struct Prepared<'a> {
        input: &'a PackEntry,
        size: u32,
        first4: u32,
    }
    let mut prepared = Vec::with_capacity(inputs.len());
    for input in inputs {
        let size = u32::try_from(fs::metadata(&input.path)?.len())
            .map_err(|_| Error::InvalidArchive("entry exceeds 4 GiB"))?;
        let mut file = File::open(&input.path)?;
        let mut first = [0; 4];
        let count = file.read(&mut first)?;
        first[count..].fill(0);
        prepared.push(Prepared {
            input,
            size,
            first4: u32::from_le_bytes(first),
        });
    }

    let mut out = create_output(output.as_ref())?;
    out.write_all(&(prepared.len() as u32).to_le_bytes())?;
    out.write_all(&archive_version.to_le_bytes())?;
    out.write_all(MAGIC)?;
    for entry in &prepared {
        out.write_all(&entry.size.to_le_bytes())?;
        out.write_all(&entry.input.ram_ptr.to_le_bytes())?;
        out.write_all(&entry.first4.to_le_bytes())?;
        out.write_all(&entry.input.file_type.to_le_bytes())?;
        out.write_all(&0_u16.to_le_bytes())?;
    }
    pad_to_sector(&mut out)?;
    for entry in prepared {
        let mut input = File::open(&entry.input.path)?;
        let copied = std::io::copy(&mut input, &mut out)?;
        if copied != u64::from(entry.size) {
            return Err(Error::InvalidArchive("entry changed while packing"));
        }
        pad_to_sector(&mut out)?;
    }
    out.flush()?;
    Ok(())
}

pub fn pack_manifest(
    output: impl AsRef<Path>,
    manifest_path: impl AsRef<Path>,
) -> Result<(), Error> {
    let manifest_path = manifest_path.as_ref();
    let root = manifest_path.parent().unwrap_or(Path::new("."));
    pack_manifest_from(output, root, manifest_path)
}

pub fn pack_manifest_from(
    output: impl AsRef<Path>,
    folder: impl AsRef<Path>,
    manifest_path: impl AsRef<Path>,
) -> Result<(), Error> {
    let value: serde_json::Value = serde_json::from_slice(&fs::read(manifest_path.as_ref())?)
        .map_err(|_| Error::InvalidArchive("invalid manifest JSON"))?;
    let entries = value
        .get("entries")
        .and_then(serde_json::Value::as_array)
        .ok_or(Error::InvalidArchive("manifest entries must be an array"))?;
    let archive_version = value
        .get("archive_version")
        .and_then(serde_json::Value::as_u64)
        .and_then(|value| u32::try_from(value).ok())
        .unwrap_or(0);
    let root = folder.as_ref();
    let inputs: Vec<_> = entries
        .iter()
        .filter_map(|entry| {
            let entry = entry.as_object()?;
            let name = entry.get("name")?.as_str()?;
            if name.is_empty() {
                return None;
            }
            Some(PackEntry {
                path: root.join(name),
                file_type: json_u16(entry.get("type")),
                ram_ptr: json_u32(entry.get("ram_ptr")),
            })
        })
        .collect();
    if inputs.is_empty() {
        return Err(Error::InvalidArchive("manifest has no named entries"));
    }
    pack(output, &inputs, archive_version)
}

pub fn pack_folder(
    output: impl AsRef<Path>,
    folder: impl AsRef<Path>,
    options: &PackFolderOptions,
) -> Result<(), Error> {
    let mut inputs = Vec::new();
    for entry in fs::read_dir(folder.as_ref())? {
        let entry = entry?;
        if !entry.file_type()?.is_file() {
            continue;
        }
        let name = entry.file_name();
        let name = name.to_string_lossy();
        if !options.include_patterns.is_empty()
            && !options
                .include_patterns
                .iter()
                .any(|pattern| glob_match(pattern, &name))
        {
            continue;
        }
        if options
            .exclude_patterns
            .iter()
            .any(|pattern| glob_match(pattern, &name))
        {
            continue;
        }
        let guessed = guess_type(entry.path());
        inputs.push(PackEntry {
            path: entry.path(),
            file_type: guessed.or(options.default_type).unwrap_or(255),
            ram_ptr: 0,
        });
    }
    inputs.sort_by(|left, right| left.path.cmp(&right.path));
    pack(output, &inputs, 0)
}

pub fn guess_type(path: impl AsRef<Path>) -> Option<u16> {
    let extension = path.as_ref().extension();
    let Some(extension) = extension else {
        return Some(0);
    };
    match extension.to_string_lossy().to_ascii_lowercase().as_str() {
        "img" | "tim" => Some(3),
        "vh" => Some(6),
        "vb" => Some(7),
        "seq" | "mid" => Some(10),
        "bin" | "dat" => Some(0),
        _ => None,
    }
}

pub fn glob_match(pattern: &str, text: &str) -> bool {
    fn matches(pattern: &[u8], text: &[u8]) -> bool {
        match pattern.split_first() {
            None => text.is_empty(),
            Some((&b'*', rest)) => {
                matches(rest, text) || (!text.is_empty() && matches(pattern, &text[1..]))
            }
            Some((&b'?', rest)) => !text.is_empty() && matches(rest, &text[1..]),
            Some((&byte, rest)) => text.first() == Some(&byte) && matches(rest, &text[1..]),
        }
    }
    matches(pattern.as_bytes(), text.as_bytes())
}

pub fn type_extension(file_type: u16) -> &'static str {
    match file_type {
        3 => "img",
        6 => "vh",
        7 => "vb",
        10 => "seq",
        _ => "bin",
    }
}

fn create_output(path: &Path) -> Result<File, Error> {
    if let Some(parent) = path
        .parent()
        .filter(|parent| !parent.as_os_str().is_empty())
    {
        fs::create_dir_all(parent)?;
    }
    Ok(File::create(path)?)
}

fn json_u32(value: Option<&serde_json::Value>) -> u32 {
    value
        .and_then(serde_json::Value::as_u64)
        .and_then(|value| u32::try_from(value).ok())
        .unwrap_or(0)
}

fn json_u16(value: Option<&serde_json::Value>) -> u16 {
    value
        .and_then(serde_json::Value::as_u64)
        .and_then(|value| u16::try_from(value).ok())
        .unwrap_or(0)
}

fn read_u32(reader: &mut impl Read) -> Result<u32, Error> {
    let mut bytes = [0; 4];
    reader.read_exact(&mut bytes)?;
    Ok(u32::from_le_bytes(bytes))
}

fn read_u16(reader: &mut impl Read) -> Result<u16, Error> {
    let mut bytes = [0; 2];
    reader.read_exact(&mut bytes)?;
    Ok(u16::from_le_bytes(bytes))
}

fn align_sector(value: u64) -> Result<u64, Error> {
    value
        .checked_add(SECTOR_SIZE - 1)
        .map(|value| value & !(SECTOR_SIZE - 1))
        .ok_or(Error::InvalidArchive("aligned entry range overflows"))
}

fn pad_to_sector(file: &mut File) -> Result<(), Error> {
    let position = file.stream_position()?;
    let aligned = align_sector(position)?;
    let padding = usize::try_from(aligned - position).unwrap();
    if padding != 0 {
        file.write_all(&vec![0; padding])?;
    }
    Ok(())
}
