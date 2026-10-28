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
    expected_packet_headers: usize,
}

pub struct XaRange {
    pub offset: usize,
    pub length: usize,
    pub evidence: &'static str,
}

impl XaSource {
    /// Identify reviewed metadata and payloads; original use remains separate.
    pub fn nontext_ranges(&self, data: &[u8]) -> Result<Vec<XaRange>> {
        if data.len() != self.file.size || sha256_hex(data) != self.file.sha256 {
            return Err(Error::Invalid(
                "XA classification requires the exact original source".into(),
            ));
        }
        if data.len() % 2336 != 0 {
            return Err(Error::Invalid(
                "original US XA sector extent disagrees".into(),
            ));
        }
        let mut ranges = Vec::new();
        let mut audio_count = 0;
        let mut empty_count = 0;
        let mut packet_count = 0;
        for (index, sector) in data.chunks_exact(2336).enumerate() {
            let start = index * 2336;
            if sector[..4] != sector[4..8] {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} has inconsistent subheaders"
                )));
            }
            ranges.push(XaRange {
                offset: start,
                length: 8,
                evidence: "original US Mode 2 duplicated sector subheader; exact source identity",
            });
            if sector[2] & 0x2e != 0x24 {
                let video =
                    self.expected_packet_headers != 0 && sector[2] & 0x2e == 2 && sector[3] == 0x80;
                if !video && (sector[2] & 0x2e != 0 || sector[3] != 0) {
                    return Err(Error::Invalid(format!(
                        "original US XA sector {index} disagrees with reviewed Form 1 profile"
                    )));
                }
                if video {
                    if sector[8..12] != [0x60, 0x01, 0x01, 0x80] {
                        return Err(Error::Invalid(format!(
                            "original US packet sector {index} disagrees with reviewed marker/type"
                        )));
                    }
                    ranges.push(XaRange {
                        offset: start + 8,
                        length: 4,
                        evidence: "original US CAPCOM packet marker/type; LOGO reader at 0x801D29B4/0x801D29C8; exact source identity; no video-body or arrival claim",
                    });
                    packet_count += 1;
                } else if sector[8..2056].iter().all(|byte| *byte == 0) {
                    ranges.push(XaRange {
                        offset: start + 8,
                        length: 2048,
                        evidence: "original US empty Form 1 user payload; all 2048 bytes zero; original purpose and consumption unverified",
                    });
                    empty_count += 1;
                }
                ranges.push(XaRange {
                    offset: start + 2056,
                    length: 280,
                    evidence: "original US Form 1 EDC and ECC P/Q fields; reviewed against disk sector owner; exact source identity",
                });
                continue;
            }
            let valid_groups = sector[8..2312].chunks_exact(128).all(|group| {
                group[..4] == group[4..8]
                    && group[8..12] == group[12..16]
                    && group[4..12]
                        .iter()
                        .all(|&parameter| parameter & 15 <= 12 && parameter >> 4 < 4)
            });
            if sector[3] & 0xfa != 0 || !valid_groups {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed four-bit format"
                )));
            }
            ranges.push(XaRange {
                offset: start + 8,
                length: 2304,
                evidence: "original US XA encoded body; supported Form 2 four-bit header, verified parameter copies and active parameters; no claim of game selection, arrival or playback",
            });
            if sector[2332..].iter().any(|byte| *byte != 0) {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed absent EDC"
                )));
            }
            ranges.push(XaRange {
                offset: start + 2332,
                length: 4,
                evidence: "original US Form 2 absent EDC field; stored zero, not a validated checksum; exact source identity",
            });
            audio_count += 1;
        }
        if audio_count != self.expected_audio_bodies
            || empty_count != self.expected_empty_payloads
            || packet_count != self.expected_packet_headers
        {
            return Err(Error::Invalid(
                "original US XA body count disagrees with reviewed profile".into(),
            ));
        }
        Ok(ranges)
    }
}

/// Canonical sources with reviewed sector metadata and bounded payload fields.
pub fn xa_sources(executable: &[u8]) -> Result<Vec<XaSource>> {
    if sha256_hex(executable) != US_EXECUTABLE_SHA256 {
        return Ok(Vec::new());
    }
    let original = disc()?;
    let mut streams = Vec::new();
    for (path, expected_audio_bodies, expected_empty_payloads, expected_packet_headers) in [
        ("BIN/SCE_XA/S_XA00.STR", 11_302, 0, 0),
        ("BIN/BMAG_XA/MAGIC00.STR", 14_856, 0, 0),
        ("BIN/SCE_XA/VOICE.STR", 297, 3239, 0),
        ("LOGO/CAPCOM30.STR", 144, 0, 1011),
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
            expected_packet_headers,
        });
    }
    Ok(streams)
}
