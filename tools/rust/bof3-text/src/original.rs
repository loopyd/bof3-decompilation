//! Original-US file identity used by coverage and loader-specific payload rules.

use std::sync::OnceLock;

use serde::Deserialize;

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result};

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

pub struct NontextRange {
    pub offset: usize,
    pub length: usize,
    pub evidence: &'static str,
}

// Original LOGO fresh-input frames 1..=221: return frontiers minus eight bytes.
// The excluded lookahead and remaining tails stay unresolved.
const CAPCOM_CONSUMED_PREFIX_BYTES: [usize; 221] = [
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
    7948, 7888, 7816, 7720, 7586, 7448, 7292, 7130, 6944, 6766, 6574, 6808, 6572, 7706, 7336, 6954,
    6584, 7240, 6720, 6190, 7372, 6546, 5688, 4922, 3942, 3096, 2704, 2704, 2704,
];

// Frames 222..=231 contain packet-bound copies of frame 221's qualified prefix.
// This identifies stored format only; later decoder invocations remain unverified.
const CAPCOM_COPIED_PREFIX_BYTES: usize = 2704;

fn capcom_coded_prefix(sector: &[u8], start: usize) -> Result<Option<NontextRange>> {
    let index = usize::from(u16::from_le_bytes([sector[12], sector[13]]));
    let count = usize::from(u16::from_le_bytes([sector[14], sector[15]]));
    let frame = u32::from_le_bytes([sector[16], sector[17], sector[18], sector[19]]) as usize;
    if !(4..=5).contains(&count) || index >= count || !(1..=231).contains(&frame) {
        return Err(Error::Invalid(
            "original US CAPCOM packet disagrees with reviewed frame structure".into(),
        ));
    }
    let (prefix, evidence) = if frame <= CAPCOM_CONSUMED_PREFIX_BYTES.len() {
        (
            CAPCOM_CONSUMED_PREFIX_BYTES[frame - 1],
            "original US CAPCOM coded prefix, frames 1..221; LOGO 0x801D7578 fresh-input return-zero frontier minus eight bytes for lookahead; exact source identity",
        )
    } else {
        (
            CAPCOM_COPIED_PREFIX_BYTES,
            "original US CAPCOM stored coded prefix, frames 222..231; exact packet-bound copies of qualified frame 221 prefix, excluding eight lookahead bytes; no later decoder invocation claim; exact source identity",
        )
    };
    if prefix > count * 2016 {
        return Err(Error::Invalid(
            "original US CAPCOM coded prefix exceeds its source packets".into(),
        ));
    }
    let length = prefix.saturating_sub(index * 2016).min(2016);
    Ok((length != 0).then_some(NontextRange {
        offset: start + 40,
        length,
        evidence,
    }))
}

impl XaSource {
    /// Identify reviewed metadata and payloads; original use remains separate.
    pub fn nontext_ranges(&self, data: &[u8]) -> Result<Vec<NontextRange>> {
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
        let mut coded_prefix_count = 0;
        let mut coded_prefix_bytes = 0;
        for (index, sector) in data.chunks_exact(2336).enumerate() {
            let start = index * 2336;
            if sector[..4] != sector[4..8] {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} has inconsistent subheaders"
                )));
            }
            ranges.push(NontextRange {
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
                    ranges.push(NontextRange {
                        offset: start + 8,
                        length: 4,
                        evidence: "original US CAPCOM packet marker/type; LOGO reader at 0x801D29B4/0x801D29C8; exact source identity; no video-body or arrival claim",
                    });
                    packet_count += 1;
                    if let Some(prefix) = capcom_coded_prefix(sector, start)? {
                        coded_prefix_count += 1;
                        coded_prefix_bytes += prefix.length;
                        ranges.push(prefix);
                    }
                } else if sector[8..2056].iter().all(|byte| *byte == 0) {
                    ranges.push(NontextRange {
                        offset: start + 8,
                        length: 2048,
                        evidence: "original US empty Form 1 user payload; all 2048 bytes zero; original purpose and consumption unverified",
                    });
                    empty_count += 1;
                }
                ranges.push(NontextRange {
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
            ranges.push(NontextRange {
                offset: start + 8,
                length: 2304,
                evidence: "original US XA encoded body; supported Form 2 four-bit header, verified parameter copies and active parameters; no claim of game selection, arrival or playback",
            });
            if sector[2312..2332].iter().any(|byte| *byte != 0) {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed zero-byte profile"
                )));
            }
            ranges.push(NontextRange {
                offset: start + 2312,
                length: 20,
                evidence: "original US zero-filled Form 2 bytes between encoded sound groups and EDC; all 20 bytes zero; exact source identity; original purpose and consumption unverified",
            });
            if sector[2332..].iter().any(|byte| *byte != 0) {
                return Err(Error::Invalid(format!(
                    "original US XA sector {index} disagrees with reviewed absent EDC"
                )));
            }
            ranges.push(NontextRange {
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
            (
                892,
                CAPCOM_CONSUMED_PREFIX_BYTES.iter().sum::<usize>()
                    + 10 * CAPCOM_COPIED_PREFIX_BYTES,
            )
        };
        if (coded_prefix_count, coded_prefix_bytes) != expected_prefixes {
            return Err(Error::Invalid(
                "original US CAPCOM coded prefixes disagree with reviewed profile".into(),
            ));
        }
        Ok(ranges)
    }
}

// Original BPLU349 entry0: image headers, packed words, token streams and palettes.
// Mode-2 opaque prefixes and every other byte remain unresolved.
const BPLU349_GRAPHICS_RANGES: &[(usize, usize)] = &[
    (76, 4104),
    (4184, 8),
    (4196, 899),
    (5096, 8),
    (5108, 4685),
    (9796, 4104),
    (13904, 8),
    (13916, 1677),
    (15596, 8),
    (15608, 930),
    (16540, 8),
    (16552, 6324),
    (22880, 749),
    (23632, 8),
    (23644, 828),
    (24476, 4159),
    (28636, 6844),
    (35484, 318),
    (35808, 64),
    (110668, 4104),
    (114776, 13672),
    (128452, 5146),
    (133600, 8),
    (133612, 5181),
    (138796, 10940),
    (149740, 996),
    (150740, 7549),
    (158292, 8),
    (158304, 4919),
    (163224, 8),
    (163236, 3537),
    (166776, 8),
    (166788, 5259),
    (172048, 8),
    (172060, 6015),
    (178076, 8),
    (178088, 500),
    (178596, 128),
    (221272, 5480),
    (226756, 1382),
    (228140, 8),
    (228152, 4310),
    (232464, 8),
    (232476, 931),
    (233408, 8),
    (233420, 824),
    (234248, 1599),
    (235848, 2748),
    (238600, 1567),
    (240168, 8),
    (240180, 3803),
    (243984, 6844),
    (250832, 4104),
    (254940, 8),
    (254952, 1684),
    (256640, 4245),
    (260888, 8),
    (260900, 321),
    (261228, 64),
];

/// Recognized serialized graphics storage; no rasterized-text or display claim.
pub fn graphics_storage_ranges(
    entry: Entry,
    data: &[u8],
    original_us: bool,
) -> Option<Vec<NontextRange>> {
    if !original_us {
        return None;
    }
    match (entry.index, entry.offset, entry.transfer_mode, entry.ram_ptr, data.len()) {
        (0, 2048, 0, 0x8003_b800, 331_788)
            if sha256_hex(data)
                == "3633597f03d7f2129ff7d7fc1ad0fe7c2e6a0affb2562e789c4a0b97b5d14b3e" =>
        {
            Some(BPLU349_GRAPHICS_RANGES
            .iter()
            .map(|&(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US BPLU349 image headers, packed words, mode-2 tokens and 32-halfword palette records; consumers 0x8014E6D0, 0x8014E7F8, 0x8014E8FC and 0x80166C48; opaque prefixes excluded; exact source identity; no rasterized-text absence or runtime claim",
            })
            .collect())
        }
        (8, 737_280, 0, 0x8003_3e00, 5120)
            if sha256_hex(data)
                == "e9fcb1395884aff7dc732517fd685e9fb26fe87ccf78f757055e410cda8e9e18" =>
        {
            Some(vec![NontextRange {
                offset: 0,
                length: data.len(),
                evidence: "original US AREA024 entry 8 serialized palette/image words; staging 0x8014E284, descriptor 0x8014E22C and original SDK image-word profile 0x8017CE9C; exact source identity; no actual full-palette upload, sampling, display or text-absence claim",
            }])
        }
        (_, _, 0, 0x800e_3800, length)
            if length >= 4 && length == 4 + usize::from(data[0]) * 8 =>
        {
            let evidence = "original US counted sprite metadata; count byte at 0x800E3800 and eight-byte records at 0x800E3804; original consumer 0x8014DD3C uses numeric count/IDs and actor fields; header bytes 1..3 and record byte 3 excluded; no runtime selection, unique supplier or text-absence claim";
            let mut ranges = vec![NontextRange {
                offset: 0,
                length: 1,
                evidence,
            }];
            for index in 0..usize::from(data[0]) {
                let offset = 4 + index * 8;
                ranges.push(NontextRange {
                    offset,
                    length: 3,
                    evidence,
                });
                ranges.push(NontextRange {
                    offset: offset + 4,
                    length: 4,
                    evidence,
                });
            }
            Some(ranges)
        }
        _ => None,
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
