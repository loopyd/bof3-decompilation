use std::collections::BTreeSet;
use std::fs::{self, File};
use std::io::Read;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::{Error, Result, digest::sha256_hex};

pub const RELEASE: &str = "SLUS_004.22";
pub const QUALIFICATION: &str = include_str!("../data/us-disc.json");
pub const MAX_FILE_BYTES: u64 = 128 * 1024 * 1024;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct SourceFile {
    pub path: String,
    pub size: u64,
    pub sha256: String,
    pub emi_entries: Option<usize>,
}

#[derive(Deserialize)]
pub struct Qualification {
    pub release: String,
    pub project_sha256: String,
    pub files: Vec<SourceFile>,
}

pub struct Input {
    pub path: PathBuf,
    pub is_tree: bool,
    pub files: Vec<SourceFile>,
}

pub fn qualification() -> Result<Qualification> {
    Ok(serde_json::from_str(QUALIFICATION)?)
}

pub fn read_bounded(path: &Path, limit: u64) -> Result<Vec<u8>> {
    let metadata = fs::symlink_metadata(path)?;
    if !metadata.is_file() || metadata.file_type().is_symlink() || metadata.len() > limit {
        return Err(Error::Invalid(format!(
            "{} must be an ordinary file of at most {limit} bytes",
            path.display()
        )));
    }
    let mut bytes = Vec::new();
    File::open(path)?.take(limit + 1).read_to_end(&mut bytes)?;
    if bytes.len() as u64 > limit || metadata.len() != bytes.len() as u64 {
        return Err(Error::Invalid(format!(
            "{} changed size or exceeds its bound",
            path.display()
        )));
    }
    Ok(bytes)
}

pub fn qualify(path: &Path, source: Option<&str>) -> Result<Input> {
    let path = fs::canonicalize(path)?;
    let known = qualification()?;
    if known.release != RELEASE {
        return Err(Error::Invalid(
            "invalid built-in qualification release".into(),
        ));
    }
    if path.is_dir() {
        if source.is_some() {
            return Err(Error::Invalid(
                "--source applies only to an individual EMI".into(),
            ));
        }
        check_inventory(&path, &known.files)?;
        for file in &known.files {
            verify_source(&path.join(&file.path), file)?;
        }
        return Ok(Input {
            path,
            is_tree: true,
            files: known.files,
        });
    }
    let bytes = read_bounded(&path, MAX_FILE_BYTES)?;
    let hash = sha256_hex(&bytes);
    let candidates: Vec<_> = known
        .files
        .into_iter()
        .filter(|file| {
            file.emi_entries.is_some()
                && file.sha256 == hash
                && source.is_none_or(|name| name == file.path)
        })
        .collect();
    let file = match candidates.as_slice() {
        [file] => file.clone(),
        [] => {
            return Err(Error::UnsupportedRelease(format!(
                "{} is not a qualified original US EMI (sha256 {hash}); use the original SLUS_004.22 corpus or a bound export package",
                path.display()
            )));
        }
        files => {
            let matching: Vec<_> = files
                .iter()
                .filter(|file| path.ends_with(&file.path))
                .collect();
            if let [file] = matching.as_slice() {
                (*file).clone()
            } else {
                return Err(Error::Invalid(format!(
                    "identical original bytes have several source locators; select --source from {}",
                    files
                        .iter()
                        .map(|file| file.path.as_str())
                        .collect::<Vec<_>>()
                        .join(", ")
                )));
            }
        }
    };
    Ok(Input {
        path,
        is_tree: false,
        files: vec![file],
    })
}

fn check_inventory(root: &Path, files: &[SourceFile]) -> Result<()> {
    let names: BTreeSet<_> = files.iter().map(|file| Path::new(&file.path)).collect();
    let generated_dumps: BTreeSet<_> = files
        .iter()
        .filter(|file| file.emi_entries.is_some())
        .map(|file| Path::new(&file.path).with_extension(""))
        .collect();
    let mut directories = vec![PathBuf::new()];
    let mut visited = 0usize;
    while let Some(directory) = directories.pop() {
        for entry in fs::read_dir(root.join(&directory))? {
            let entry = entry?;
            visited += 1;
            if visited > 65_536 {
                return Err(Error::Invalid(
                    "source inventory exceeds 65536 entries".into(),
                ));
            }
            let relative = directory.join(entry.file_name());
            let kind = entry.file_type()?;
            if kind.is_dir() {
                if !generated_dumps.contains(&relative) {
                    directories.push(relative);
                }
                continue;
            }
            if !kind.is_file() {
                return Err(Error::Invalid(format!(
                    "source inventory contains a symlink or special file: {}",
                    relative.display()
                )));
            }
            let generated_root_file = directory.as_os_str().is_empty()
                && (relative == Path::new("license_data.dat")
                    || relative.extension().is_some_and(|extension| {
                        extension.eq_ignore_ascii_case("xml")
                            || extension.eq_ignore_ascii_case("wav")
                    }));
            if !names.contains(relative.as_path()) && !generated_root_file {
                return Err(Error::UnsupportedRelease(format!(
                    "unexpected input {}; the qualified US inventory changed",
                    relative.display()
                )));
            }
        }
    }
    Ok(())
}

pub fn verify_source(path: &Path, expected: &SourceFile) -> Result<Vec<u8>> {
    let bytes = read_bounded(path, MAX_FILE_BYTES)?;
    if bytes.len() as u64 != expected.size || sha256_hex(&bytes) != expected.sha256 {
        return Err(Error::UnsupportedRelease(format!(
            "{} differs from qualified original {}",
            path.display(),
            expected.path
        )));
    }
    Ok(bytes)
}

impl Input {
    pub fn source_path(&self, source: &SourceFile) -> PathBuf {
        if self.is_tree {
            self.path.join(&source.path)
        } else {
            self.path.clone()
        }
    }
}
