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

// Original LOGO fresh-input frames 1..=200: return frontiers minus eight bytes.
// The excluded lookahead and all other frame bytes remain unresolved.
const CAPCOM_CONSUMED_PREFIX_BYTES: [usize; 200] = [
    7640, 7690, 7752, 7776, 7810, 7826, 7766, 7668, 7610, 7526, 7484, 7426, 7404, 7440, 7490, 7536,
    7564, 7534, 7622, 7688, 7700, 6704, 7122, 6880, 7080, 6718, 6758, 7266, 6816, 7500, 7442, 7328,
    7322, 7810, 8032, 7804, 7678, 7630, 7874, 6578, 6476, 6468, 6558, 6720, 6744, 6732, 6648, 6814,
    7118, 7036, 7278, 6894, 7190, 7194, 7276, 7704, 7406, 7292, 7786, 7226, 7132, 7182, 7030, 6928,
    6954, 6810, 7098, 6550, 6846, 7328, 7260, 7406, 6818, 7000, 7164, 7076, 7110, 6844, 6596, 6850,
    6642, 7602, 7980, 7348, 7264, 7178, 6746, 6796, 6710, 7072, 6526, 6952, 7090, 6660, 7948, 7054,
    7038, 7076, 6832, 7284, 6984, 7442, 7354, 7068, 7158, 7268, 7634, 7734, 7754, 7754, 7754, 7754,
    7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7754, 7744, 7730, 7720, 7706, 7684,
    7660, 7628, 7608, 7590, 7602, 7642, 7692, 7744, 7814, 7862, 7928, 7964, 8014, 6842, 6884, 6916,
    6950, 6986, 7034, 7070, 7100, 7144, 7170, 7202, 7232, 7260, 7284, 7302, 7326, 7348, 7364, 7378,
    7388, 7400, 7404, 7410, 7410, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040,
    8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 8040, 7998,
    7948, 7888, 7816, 7720, 7586, 7448, 7292, 7130,
];

fn capcom_consumed_prefix(sector: &[u8], start: usize) -> Result<Option<XaRange>> {
    let index = usize::from(u16::from_le_bytes([sector[12], sector[13]]));
    let count = usize::from(u16::from_le_bytes([sector[14], sector[15]]));
    let frame = u32::from_le_bytes([sector[16], sector[17], sector[18], sector[19]]) as usize;
    if !(4..=5).contains(&count) || index >= count || !(1..=231).contains(&frame) {
        return Err(Error::Invalid(
            "original US CAPCOM packet disagrees with reviewed frame structure".into(),
        ));
    }
    let Some(&prefix) = CAPCOM_CONSUMED_PREFIX_BYTES.get(frame - 1) else {
        return Ok(None);
    };
    if prefix > count * 2016 {
        return Err(Error::Invalid(
            "original US CAPCOM consumed prefix exceeds its source packets".into(),
        ));
    }
    let length = prefix.saturating_sub(index * 2016).min(2016);
    Ok((length != 0).then_some(XaRange {
        offset: start + 40,
        length,
        evidence: "original US CAPCOM coded prefix, frames 1..200; LOGO 0x801D7578 fresh-input return-zero frontier minus eight bytes for lookahead; exact source identity",
    }))
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
        let mut consumed_prefix_count = 0;
        let mut consumed_prefix_bytes = 0;
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
                    if let Some(prefix) = capcom_consumed_prefix(sector, start)? {
                        consumed_prefix_count += 1;
                        consumed_prefix_bytes += prefix.length;
                        ranges.push(prefix);
                    }
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
        let expected_prefixes = if self.expected_packet_headers == 0 {
            (0, 0)
        } else {
            (800, CAPCOM_CONSUMED_PREFIX_BYTES.iter().sum())
        };
        if (consumed_prefix_count, consumed_prefix_bytes) != expected_prefixes {
            return Err(Error::Invalid(
                "original US CAPCOM consumed prefixes disagree with reviewed profile".into(),
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
