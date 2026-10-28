//! Original-US file identity used by coverage and loader-specific payload rules.

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

pub struct XaSource {
    pub file: &'static File,
    expected_audio_bodies: usize,
    expected_empty_payloads: usize,
}

pub struct XaBody {
    pub offset: usize,
    pub length: usize,
    pub evidence: &'static str,
}

impl XaSource {
    /// Identify encoded audio and empty user payloads; original use remains separate.
    pub fn bodies(&self, data: &[u8]) -> Result<Vec<XaBody>> {
        if data.len() != self.file.size || sha256_hex(data) != self.file.sha256 {
            return Err(Error::Invalid(
                "XA body classification requires the exact original source".into(),
            ));
        }
        if data.len() % 2336 != 0 {
            return Err(Error::Invalid(
                "original US XA sector extent disagrees".into(),
            ));
        }
        let mut bodies = Vec::new();
        let mut audio_count = 0;
        let mut empty_count = 0;
        for (index, sector) in data.chunks_exact(2336).enumerate() {
            if sector[2] & 0x2e != 0x24 {
                if sector[..4] == sector[4..8]
                    && sector[2] & 0x2e == 0
                    && sector[3] == 0
                    && sector[8..2056].iter().all(|byte| *byte == 0)
                {
                    bodies.push(XaBody {
                        offset: index * 2336 + 8,
                        length: 2048,
                        evidence: "original US empty Form 1 user payload; all 2048 bytes zero; original purpose and consumption unverified",
                    });
                    empty_count += 1;
                }
                continue;
            }
            let valid_groups = sector[8..2312].chunks_exact(128).all(|group| {
                group[..4] == group[4..8]
                    && group[8..12] == group[12..16]
                    && group[4..12]
                        .iter()
                        .all(|&parameter| parameter & 15 <= 12 && parameter >> 4 < 4)
            });
            if sector[..4] != sector[4..8] || sector[3] & 0xfa != 0 || !valid_groups {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed four-bit format"
                )));
            }
            bodies.push(XaBody {
                offset: index * 2336 + 8,
                length: 2304,
                evidence: "original US XA encoded body; supported Form 2 four-bit header, verified parameter copies and active parameters; no claim of game selection, arrival or playback",
            });
            audio_count += 1;
        }
        if audio_count != self.expected_audio_bodies || empty_count != self.expected_empty_payloads
        {
            return Err(Error::Invalid(
                "original US XA body count disagrees with reviewed profile".into(),
            ));
        }
        Ok(bodies)
    }
}

/// Canonical XA sources with reviewed encoded audio or empty Form 1 user payloads.
pub fn xa_sources(executable: &[u8]) -> Result<Vec<XaSource>> {
    if sha256_hex(executable) != US_EXECUTABLE_SHA256 {
        return Ok(Vec::new());
    }
    let original = disc()?;
    let mut streams = Vec::new();
    for (path, expected_audio_bodies, expected_empty_payloads) in [
        ("BIN/SCE_XA/S_XA00.STR", 11_302, 0),
        ("BIN/BMAG_XA/MAGIC00.STR", 14_856, 0),
        ("BIN/SCE_XA/VOICE.STR", 297, 3239),
    ] {
        let file = original
            .files
            .iter()
            .find(|file| file.path == path)
            .ok_or_else(|| {
                Error::Invalid(format!(
                    "original US XA source missing from manifest: {path}"
                ))
            })?;
        streams.push(XaSource {
            file,
            expected_audio_bodies,
            expected_empty_payloads,
        });
    }
    Ok(streams)
}
