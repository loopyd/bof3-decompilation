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

// Original AREA024 entry4: directory offsets and consumed program fields.
// Program header byte 0 and unreferenced offset words stay unresolved.
const AREA024_ANIMATION_METADATA_RANGES: &[(usize, usize)] = &[
    (0, 140),
    (141, 5),
    (189, 5),
    (211, 5),
    (233, 5),
    (271, 5),
    (299, 25),
    (441, 25),
    (583, 45),
    (833, 25),
    (1_001, 5),
    (1_023, 5),
    (1_045, 17),
    (1_167, 5),
    (1_209, 25),
    (1_381, 17),
    (1_529, 17),
    (1_711, 9),
    (1_779, 9),
    (1_851, 5),
    (1_879, 5),
    (1_911, 5),
    (1_949, 5),
    (1_987, 5),
    (2_019, 5),
    (2_067, 5),
    (2_119, 13),
    (2_361, 23),
    (2_691, 15),
    (2_916, 46),
    (2_963, 5),
    (2_995, 5),
    (3_017, 5),
    (3_035, 5),
    (3_053, 5),
    (3_085, 25),
    (3_187, 25),
    (3_295, 9),
    (3_327, 5),
    (3_345, 5),
    (3_373, 17),
    (3_475, 5),
    (3_493, 5),
    (3_515, 17),
    (3_597, 17),
    (3_665, 17),
    (3_733, 5),
    (3_761, 5),
    (3_799, 9),
    (3_847, 5),
    (3_879, 13),
    (4_121, 23),
    (4_515, 11),
    (4_680, 20),
    (4_701, 5),
    (4_729, 5),
    (4_751, 5),
    (4_773, 5),
    (4_801, 5),
    (4_829, 25),
    (5_007, 25),
    (5_135, 5),
    (5_157, 21),
    (5_365, 11),
    (5_504, 18),
    (5_523, 5),
    (5_561, 5),
    (5_599, 5),
    (5_627, 5),
    (5_659, 5),
    (5_701, 25),
    (5_909, 25),
    (6_131, 13),
    (6_239, 5),
    (6_272, 8),
    (6_281, 5),
    (6_293, 51),
    (6_773, 5),
    (6_791, 45),
    (6_838, 2),
    (7_228, 12),
    (7_241, 17),
    (7_383, 27),
    (7_637, 9),
    (7_720, 12),
    (7_733, 13),
    (7_861, 23),
    (8_045, 9),
    (8_116, 2),
    (8_119, 29),
    (8_260, 12),
    (8_273, 13),
    (8_455, 31),
    (8_855, 9),
    (8_996, 2),
    (8_999, 5),
    (9_012, 32),
    (9_045, 5),
    (9_063, 5),
    (9_081, 5),
    (9_103, 5),
    (9_121, 19),
    (9_175, 17),
    (9_217, 5),
    (9_235, 5),
    (9_253, 5),
    (9_265, 5),
    (9_277, 5),
    (9_289, 5),
    (9_307, 5),
    (9_324, 6),
    (9_331, 47),
    (9_491, 47),
    (9_671, 47),
    (9_840, 2),
    (9_843, 5),
    (9_856, 4),
    (9_861, 9),
    (9_943, 9),
    (10_016, 2),
    (10_019, 17),
    (10_220, 4),
    (10_225, 5),
    (10_263, 5),
    (10_304, 2),
    (10_307, 9),
    (10_408, 2),
    (10_411, 9),
    (10_564, 8),
    (10_573, 39),
    (10_667, 31),
    (10_741, 35),
    (10_825, 39),
    (10_924, 2),
    (10_927, 29),
    (11_288, 2),
    (11_291, 25),
];

// Referenced original cells only; counts and XYUV fields exclude packed flags and tails.
const AREA024_REFERENCED_CELL_OFFSETS: &[usize] = &[
    146, 194, 216, 238, 276, 324, 345, 366, 382, 403, 419, 466, 487, 503, 524, 545, 566, 628, 649,
    675, 696, 722, 753, 784, 815, 858, 879, 900, 921, 957, 978, 1_006, 1_028, 1_062, 1_093, 1_119,
    1_140, 1_172, 1_234, 1_260, 1_281, 1_307, 1_333, 1_359, 1_398, 1_429, 1_465, 1_491, 1_546,
    1_587, 1_628, 1_669, 1_720, 1_751, 1_788, 1_819, 1_856, 1_884, 1_916, 1_954, 1_992, 2_024,
    2_072, 2_132, 2_208, 2_284, 2_384, 2_460, 2_536, 2_607, 2_648, 2_706, 2_782, 2_848, 2_968,
    3_000, 3_022, 3_040, 3_058, 3_110, 3_131, 3_142, 3_153, 3_164, 3_175, 3_212, 3_223, 3_234,
    3_250, 3_266, 3_282, 3_304, 3_315, 3_332, 3_350, 3_390, 3_401, 3_422, 3_453, 3_480, 3_498,
    3_532, 3_548, 3_559, 3_585, 3_614, 3_625, 3_636, 3_652, 3_682, 3_693, 3_704, 3_720, 3_738,
    3_766, 3_808, 3_819, 3_852, 3_892, 3_968, 4_044, 4_144, 4_220, 4_296, 4_372, 4_443, 4_526,
    4_602, 4_706, 4_734, 4_756, 4_778, 4_806, 4_854, 4_890, 4_916, 4_937, 4_958, 4_979, 5_032,
    5_048, 5_064, 5_080, 5_101, 5_117, 5_140, 5_178, 5_209, 5_265, 5_301, 5_332, 5_378, 5_439,
    5_528, 5_566, 5_604, 5_632, 5_664, 5_726, 5_762, 5_793, 5_824, 5_850, 5_876, 5_934, 5_965,
    5_991, 6_032, 6_063, 6_104, 6_144, 6_175, 6_211, 6_244, 6_286, 6_344, 6_350, 6_361, 6_442,
    6_493, 6_524, 6_555, 6_596, 6_627, 6_668, 6_719, 6_765, 6_778, 6_840, 6_851, 6_877, 6_928,
    6_969, 7_015, 7_036, 7_057, 7_088, 7_134, 7_221, 7_258, 7_289, 7_320, 7_351, 7_410, 7_441,
    7_472, 7_508, 7_564, 7_600, 7_646, 7_682, 7_746, 7_787, 7_823, 7_884, 7_920, 7_956, 7_992,
    8_018, 8_054, 8_085, 8_148, 8_154, 8_165, 8_191, 8_202, 8_228, 8_244, 8_286, 8_342, 8_398,
    8_486, 8_542, 8_598, 8_659, 8_720, 8_781, 8_817, 8_864, 8_930, 9_004, 9_050, 9_068, 9_086,
    9_108, 9_140, 9_146, 9_157, 9_168, 9_192, 9_198, 9_204, 9_210, 9_222, 9_240, 9_258, 9_270,
    9_282, 9_294, 9_312, 9_378, 9_384, 9_390, 9_401, 9_412, 9_423, 9_429, 9_440, 9_461, 9_472,
    9_483, 9_538, 9_544, 9_550, 9_556, 9_567, 9_578, 9_594, 9_620, 9_636, 9_652, 9_663, 9_718,
    9_724, 9_730, 9_736, 9_752, 9_768, 9_789, 9_805, 9_816, 9_827, 9_833, 9_848, 9_870, 9_906,
    9_952, 9_983, 10_036, 10_082, 10_128, 10_174, 10_230, 10_268, 10_316, 10_362, 10_420, 10_491,
    10_612, 10_618, 10_624, 10_630, 10_636, 10_642, 10_648, 10_654, 10_660, 10_698, 10_704, 10_710,
    10_716, 10_722, 10_728, 10_734, 10_776, 10_782, 10_788, 10_794, 10_800, 10_806, 10_812, 10_818,
    10_864, 10_870, 10_881, 10_887, 10_893, 10_899, 10_905, 10_911, 10_917, 10_956, 11_017, 11_063,
    11_104, 11_140, 11_201, 11_247, 11_316, 11_322, 11_328, 11_339, 11_345, 11_361,
];

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
        (4, 198_656, 0, 0x800d_3800, 11_368)
            if sha256_hex(data)
                == "1b36c8e96987f4c2256daa9e24e477d90c73616242be2588906525a4c8dc2c99" =>
        {
            let mut ranges: Vec<_> = AREA024_ANIMATION_METADATA_RANGES
            .iter()
            .map(|&(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US AREA024 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
            })
            .collect();
            let evidence = "original US AREA024 entry 4 referenced cell counts and signed X/Y, unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD8C; quad-path numeric fields only, no rectangle-helper behavior claim; packed flags, tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim";
            for &cell in AREA024_REFERENCED_CELL_OFFSETS {
                ranges.push(NontextRange {
                    offset: cell,
                    length: 1,
                    evidence,
                });
                for part in 0..usize::from(data[cell]) {
                    ranges.push(NontextRange {
                        offset: cell + 2 + 5 * part,
                        length: 4,
                        evidence,
                    });
                }
            }
            ranges.sort_by_key(|range| range.offset);
            Some(ranges)
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
