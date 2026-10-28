//! Original-US file identity used by coverage and loader-specific payload rules.

use std::collections::BTreeSet;
use std::sync::OnceLock;

use serde::Deserialize;

use crate::digest::sha256_hex;
use crate::models::{Error, Result};

pub const DISC: &str = include_str!("../../../../config/original-us-disc.json");
pub const US_EXECUTABLE_SHA256: &str =
    "0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a";

#[derive(Deserialize)]
pub struct Disc {
    pub release: String,
    pub project_sha256: String,
    pub files: Vec<File>,
}

#[derive(Deserialize)]
pub struct File {
    pub path: String,
    pub size: usize,
    pub sha256: String,
    pub emi_entries: Option<usize>,
}

pub fn disc() -> Result<&'static Disc> {
    static ORIGINAL: OnceLock<std::result::Result<Disc, String>> = OnceLock::new();
    ORIGINAL
        .get_or_init(|| serde_json::from_str(DISC).map_err(|error| error.to_string()))
        .as_ref()
        .map_err(|error| Error::Invalid(format!("original US disc qualification: {error}")))
}

/// Whole-archive identity, independent of the input's path or regional label.
pub fn archive(data: &[u8]) -> Result<bool> {
    let original = disc()?;
    let digest = sha256_hex(data);
    Ok(original
        .files
        .iter()
        .any(|file| file.emi_entries.is_some() && file.size == data.len() && file.sha256 == digest))
}

pub struct XaStarts {
    pub file: &'static File,
    pub sectors: Vec<usize>,
}

/// Requested starts from original selector 0x80163744, without playback intervals.
pub fn xa_starts(executable: &[u8]) -> Result<Vec<XaStarts>> {
    if sha256_hex(executable) != US_EXECUTABLE_SHA256 {
        return Ok(Vec::new());
    }
    let original = disc()?;
    let mut streams = Vec::new();
    for (path, table, words, stride, count) in [
        ("BIN/SCE_XA/S_XA00.STR", 0x8018_39c4, 15, 8, 11),
        ("BIN/BMAG_XA/MAGIC00.STR", 0x8018_32c4, 896, 16, 880),
        ("BIN/SCE_XA/VOICE.STR", 0x8018_3298, 10, 16, 5),
    ] {
        let offset = table - 0x8009_6000;
        let halfword = |index: usize| {
            u16::from_le_bytes(
                executable[offset + index * 2..offset + index * 2 + 2]
                    .try_into()
                    .unwrap(),
            )
        };
        let mut position = 0;
        let mut channel = 0;
        let mut sectors = BTreeSet::new();
        while position + 1 < words {
            let word = halfword(position);
            let start = usize::from(word & 0x7fff);
            let end = usize::from(halfword(position + 1) & 0x7fff);
            if end <= start || channel >= stride || !sectors.insert(start * stride + channel) {
                return Err(Error::Invalid(
                    "original US XA start table disagrees with reviewed profile".into(),
                ));
            }
            if word & 0x8000 != 0 {
                channel += 1;
                position += 2;
            } else {
                position += 1;
            }
        }
        if sectors.len() != count {
            return Err(Error::Invalid(
                "original US XA start count disagrees with reviewed profile".into(),
            ));
        }
        let file = original
            .files
            .iter()
            .find(|file| file.path == path)
            .ok_or_else(|| {
                Error::Invalid(format!(
                    "original US XA source missing from manifest: {path}"
                ))
            })?;
        streams.push(XaStarts {
            file,
            sectors: sectors.into_iter().collect(),
        });
    }
    Ok(streams)
}
