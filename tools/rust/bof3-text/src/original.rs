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

/// Numeric fields qualified against the complete original US executable.
pub fn executable_nontext_ranges(data: &[u8]) -> Vec<NontextRange> {
    if data.len() != 1_445_888 || sha256_hex(data) != US_EXECUTABLE_SHA256 {
        return Vec::new();
    }
    vec![NontextRange {
        offset: 955_708,
        length: 32,
        evidence: "original US SLUS 0x8017F53C..0x8017F55C unsigned width/height pairs; bounded byte-pair consumer 0x8014C7F0..0x8014C818; exact executable identity; stored numeric representation only",
    }]
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

// Individually checked original AREA000 entry4 fields; excluded bytes stay unresolved.
const AREA000_ANIMATION_METADATA_RANGES: &[(usize, usize)] = &[
    (0, 122),
    (123, 5),
    (155, 5),
    (177, 5),
    (199, 5),
    (227, 5),
    (265, 25),
    (483, 25),
    (692, 16),
    (709, 5),
    (747, 5),
    (769, 5),
    (791, 5),
    (819, 5),
    (857, 25),
    (979, 25),
    (1_101, 17),
    (1_224, 14),
    (1_239, 5),
    (1_277, 5),
    (1_305, 5),
    (1_347, 5),
    (1_375, 5),
    (1_413, 25),
    (1_595, 25),
    (1_792, 14),
    (1_807, 5),
    (1_845, 5),
    (1_867, 5),
    (1_889, 5),
    (1_917, 5),
    (1_955, 25),
    (2_087, 25),
    (2_220, 16),
    (2_237, 5),
    (2_259, 5),
    (2_281, 5),
    (2_303, 5),
    (2_325, 5),
    (2_347, 25),
    (2_459, 25),
    (2_581, 5),
    (2_604, 14),
    (2_619, 5),
    (2_657, 5),
    (2_679, 5),
    (2_707, 5),
    (2_735, 5),
    (2_763, 25),
    (2_915, 25),
    (3_072, 14),
    (3_087, 5),
    (3_115, 5),
    (3_137, 5),
    (3_155, 5),
    (3_173, 5),
    (3_195, 21),
    (3_301, 21),
    (3_392, 6),
    (3_399, 25),
    (3_581, 25),
    (3_793, 25),
    (4_096, 14),
    (4_111, 5),
    (4_149, 5),
    (4_171, 5),
    (4_199, 5),
    (4_221, 5),
    (4_249, 25),
    (4_371, 25),
    (4_528, 14),
    (4_543, 5),
    (4_565, 5),
    (4_583, 5),
    (4_601, 5),
    (4_619, 5),
    (4_641, 25),
    (4_779, 25),
    (4_952, 16),
    (4_969, 33),
    (5_107, 33),
    (5_209, 5),
    (5_227, 5),
    (5_245, 9),
    (5_283, 9),
    (5_325, 13),
    (5_367, 13),
    (5_416, 14),
    (5_431, 5),
    (5_443, 5),
    (5_461, 5),
    (5_479, 5),
    (5_497, 5),
    (5_509, 25),
    (5_637, 27),
    (5_760, 32),
    (5_793, 5),
    (5_811, 5),
    (5_829, 5),
    (5_851, 5),
    (5_869, 19),
    (5_923, 17),
    (5_965, 5),
    (5_983, 5),
    (6_001, 5),
    (6_013, 5),
    (6_025, 5),
    (6_037, 5),
    (6_055, 5),
    (6_072, 4),
    (6_077, 41),
    (6_289, 41),
    (6_508, 4),
    (6_513, 5),
    (6_573, 5),
    (6_612, 4),
    (6_617, 13),
    (6_673, 25),
    (6_788, 4),
    (6_793, 21),
    (6_895, 19),
    (6_964, 4),
    (6_969, 5),
    (7_037, 5),
    (7_096, 4),
    (7_101, 5),
    (7_133, 33),
    (7_404, 2),
    (7_407, 101),
    (8_784, 6),
    (8_791, 17),
    (8_833, 19),
    (8_877, 17),
    (8_920, 2),
    (8_923, 105),
    (9_956, 2),
    (9_959, 21),
    (10_236, 4),
    (10_241, 5),
    (10_279, 29),
    (10_472, 2),
    (10_475, 5),
    (10_496, 2),
    (10_499, 5),
    (10_540, 2),
    (10_543, 5),
];

const AREA000_REFERENCED_CELL_OFFSETS: &[usize] = &[
    128, 160, 182, 204, 232, 290, 321, 362, 388, 419, 445, 508, 534, 565, 591, 622, 653, 714, 752,
    774, 796, 824, 882, 898, 914, 930, 946, 962, 1_004, 1_020, 1_036, 1_052, 1_068, 1_084, 1_118,
    1_144, 1_170, 1_196, 1_244, 1_282, 1_310, 1_352, 1_380, 1_438, 1_459, 1_485, 1_511, 1_542,
    1_568, 1_620, 1_651, 1_677, 1_708, 1_734, 1_765, 1_812, 1_850, 1_872, 1_894, 1_922, 1_980,
    1_996, 2_012, 2_033, 2_049, 2_070, 2_112, 2_128, 2_149, 2_165, 2_181, 2_197, 2_242, 2_264,
    2_286, 2_308, 2_330, 2_372, 2_383, 2_399, 2_410, 2_426, 2_442, 2_484, 2_500, 2_511, 2_527,
    2_543, 2_564, 2_586, 2_624, 2_662, 2_684, 2_712, 2_740, 2_788, 2_809, 2_830, 2_851, 2_872,
    2_893, 2_940, 2_961, 2_982, 3_003, 3_029, 3_050, 3_092, 3_120, 3_142, 3_160, 3_178, 3_216,
    3_242, 3_263, 3_284, 3_322, 3_338, 3_354, 3_370, 3_424, 3_450, 3_476, 3_502, 3_528, 3_554,
    3_606, 3_637, 3_668, 3_699, 3_730, 3_761, 3_818, 3_864, 3_910, 3_956, 4_002, 4_048, 4_116,
    4_154, 4_176, 4_204, 4_226, 4_274, 4_290, 4_306, 4_322, 4_338, 4_354, 4_396, 4_412, 4_428,
    4_459, 4_480, 4_511, 4_548, 4_570, 4_588, 4_606, 4_624, 4_666, 4_687, 4_703, 4_724, 4_740,
    4_756, 4_804, 4_820, 4_846, 4_872, 4_898, 4_919, 5_002, 5_018, 5_029, 5_040, 5_051, 5_067,
    5_078, 5_089, 5_140, 5_146, 5_152, 5_158, 5_164, 5_170, 5_186, 5_197, 5_214, 5_232, 5_254,
    5_265, 5_292, 5_303, 5_338, 5_344, 5_355, 5_380, 5_391, 5_402, 5_436, 5_448, 5_466, 5_484,
    5_502, 5_534, 5_545, 5_556, 5_572, 5_593, 5_609, 5_664, 5_675, 5_686, 5_697, 5_718, 5_739,
    5_798, 5_816, 5_834, 5_856, 5_888, 5_894, 5_905, 5_916, 5_940, 5_946, 5_952, 5_958, 5_970,
    5_988, 6_006, 6_018, 6_030, 6_042, 6_060, 6_118, 6_144, 6_180, 6_216, 6_257, 6_330, 6_356,
    6_382, 6_423, 6_464, 6_520, 6_578, 6_630, 6_651, 6_698, 6_719, 6_745, 6_766, 6_814, 6_830,
    6_846, 6_862, 6_878, 6_914, 6_930, 6_946, 6_974, 7_042, 7_106, 7_166, 7_192, 7_223, 7_254,
    7_285, 7_311, 7_342, 7_373, 7_508, 7_559, 7_610, 7_661, 7_712, 7_763, 7_814, 7_865, 7_916,
    7_967, 8_018, 8_069, 8_120, 8_171, 8_222, 8_273, 8_324, 8_375, 8_426, 8_477, 8_528, 8_579,
    8_630, 8_681, 8_732, 8_808, 8_814, 8_820, 8_826, 8_852, 8_858, 8_864, 8_870, 8_894, 8_900,
    8_906, 8_912, 9_028, 9_064, 9_100, 9_136, 9_172, 9_208, 9_244, 9_280, 9_316, 9_352, 9_388,
    9_424, 9_460, 9_496, 9_532, 9_563, 9_594, 9_630, 9_666, 9_702, 9_738, 9_774, 9_810, 9_846,
    9_882, 9_918, 9_980, 10_031, 10_082, 10_133, 10_184, 10_246, 10_308, 10_344, 10_385, 10_426,
    10_480, 10_504, 10_548,
];

const AREA001_ANIMATION_METADATA_RANGES: &[(usize, usize)] = &[
    (0, 92),
    (93, 5),
    (155, 5),
    (207, 5),
    (249, 5),
    (311, 5),
    (369, 25),
    (631, 25),
    (903, 25),
    (1_331, 17),
    (1_653, 21),
    (2_056, 46),
    (2_103, 5),
    (2_135, 5),
    (2_157, 5),
    (2_175, 5),
    (2_193, 5),
    (2_225, 25),
    (2_327, 25),
    (2_435, 9),
    (2_467, 5),
    (2_485, 5),
    (2_513, 17),
    (2_615, 5),
    (2_633, 5),
    (2_655, 17),
    (2_737, 17),
    (2_805, 17),
    (2_873, 5),
    (2_901, 5),
    (2_939, 9),
    (2_987, 5),
    (3_019, 13),
    (3_261, 23),
    (3_655, 11),
    (3_820, 16),
    (3_837, 5),
    (3_889, 5),
    (3_931, 5),
    (3_963, 5),
    (3_991, 5),
    (4_039, 25),
    (4_277, 25),
    (4_525, 5),
    (4_556, 20),
    (4_577, 5),
    (4_605, 5),
    (4_627, 5),
    (4_649, 5),
    (4_677, 5),
    (4_705, 25),
    (4_883, 25),
    (5_011, 5),
    (5_033, 21),
    (5_241, 11),
    (5_380, 18),
    (5_399, 5),
    (5_437, 5),
    (5_475, 5),
    (5_503, 5),
    (5_535, 5),
    (5_577, 25),
    (5_785, 25),
    (6_007, 13),
    (6_115, 5),
    (6_148, 24),
    (6_173, 5),
    (6_201, 5),
    (6_223, 5),
    (6_245, 5),
    (6_273, 5),
    (6_295, 25),
    (6_477, 25),
    (6_629, 9),
    (6_671, 5),
    (6_699, 5),
    (6_727, 17),
    (6_845, 17),
    (6_984, 14),
    (6_999, 5),
    (7_047, 5),
    (7_069, 5),
    (7_091, 5),
    (7_129, 5),
    (7_157, 25),
    (7_299, 25),
    (7_456, 20),
    (7_477, 5),
    (7_535, 5),
    (7_573, 5),
    (7_595, 5),
    (7_637, 5),
    (7_685, 25),
    (7_917, 25),
    (8_139, 13),
    (8_247, 9),
    (8_319, 5),
    (8_360, 18),
    (8_379, 5),
    (8_427, 5),
    (8_469, 5),
    (8_497, 5),
    (8_545, 5),
    (8_593, 21),
    (8_759, 11),
    (8_849, 11),
    (8_944, 4),
    (8_949, 49),
    (9_357, 5),
    (9_396, 14),
    (9_411, 5),
    (9_423, 5),
    (9_441, 5),
    (9_459, 5),
    (9_477, 5),
    (9_489, 25),
    (9_617, 27),
    (9_740, 32),
    (9_773, 5),
    (9_791, 5),
    (9_809, 5),
    (9_831, 5),
    (9_849, 19),
    (9_903, 17),
    (9_945, 5),
    (9_963, 5),
    (9_981, 5),
    (9_993, 5),
    (10_005, 5),
    (10_017, 5),
    (10_035, 5),
    (10_052, 2),
    (10_055, 33),
    (10_364, 2),
    (10_367, 57),
    (10_796, 2),
    (10_799, 53),
    (11_312, 2),
    (11_315, 57),
    (11_808, 4),
    (11_813, 5),
    (11_873, 5),
    (11_912, 2),
    (11_915, 5),
];

const AREA001_REFERENCED_CELL_OFFSETS: &[usize] = &[
    98, 160, 212, 254, 316, 394, 440, 476, 517, 553, 594, 656, 692, 738, 774, 820, 856, 928, 984,
    1_035, 1_086, 1_167, 1_248, 1_348, 1_424, 1_500, 1_576, 1_674, 1_750, 1_826, 1_902, 1_978,
    2_108, 2_140, 2_162, 2_180, 2_198, 2_250, 2_271, 2_282, 2_293, 2_304, 2_315, 2_352, 2_363,
    2_374, 2_390, 2_406, 2_422, 2_444, 2_455, 2_472, 2_490, 2_530, 2_541, 2_562, 2_593, 2_620,
    2_638, 2_672, 2_688, 2_699, 2_725, 2_754, 2_765, 2_776, 2_792, 2_822, 2_833, 2_844, 2_860,
    2_878, 2_906, 2_948, 2_959, 2_992, 3_032, 3_108, 3_184, 3_284, 3_360, 3_436, 3_512, 3_583,
    3_666, 3_742, 3_842, 3_894, 3_936, 3_968, 3_996, 4_064, 4_100, 4_141, 4_172, 4_203, 4_234,
    4_302, 4_338, 4_369, 4_410, 4_451, 4_492, 4_530, 4_582, 4_610, 4_632, 4_654, 4_682, 4_730,
    4_766, 4_792, 4_813, 4_834, 4_855, 4_908, 4_924, 4_940, 4_956, 4_977, 4_993, 5_016, 5_054,
    5_085, 5_141, 5_177, 5_208, 5_254, 5_315, 5_404, 5_442, 5_480, 5_508, 5_540, 5_602, 5_638,
    5_669, 5_700, 5_726, 5_752, 5_810, 5_841, 5_867, 5_908, 5_939, 5_980, 6_020, 6_051, 6_087,
    6_120, 6_178, 6_206, 6_228, 6_250, 6_278, 6_320, 6_351, 6_372, 6_403, 6_424, 6_455, 6_502,
    6_518, 6_544, 6_565, 6_581, 6_602, 6_638, 6_654, 6_676, 6_704, 6_744, 6_765, 6_791, 6_817,
    6_862, 6_883, 6_909, 6_945, 7_004, 7_052, 7_074, 7_096, 7_134, 7_182, 7_203, 7_224, 7_240,
    7_261, 7_277, 7_324, 7_345, 7_361, 7_382, 7_418, 7_439, 7_482, 7_540, 7_578, 7_600, 7_642,
    7_710, 7_746, 7_782, 7_813, 7_849, 7_880, 7_942, 7_973, 8_009, 8_040, 8_071, 8_102, 8_152,
    8_183, 8_214, 8_256, 8_287, 8_324, 8_384, 8_432, 8_474, 8_502, 8_550, 8_614, 8_640, 8_671,
    8_702, 8_770, 8_806, 8_860, 8_896, 8_998, 9_029, 9_060, 9_091, 9_117, 9_148, 9_179, 9_215,
    9_241, 9_262, 9_288, 9_324, 9_362, 9_416, 9_428, 9_446, 9_464, 9_482, 9_514, 9_525, 9_536,
    9_552, 9_573, 9_589, 9_644, 9_655, 9_666, 9_677, 9_698, 9_719, 9_778, 9_796, 9_814, 9_836,
    9_868, 9_874, 9_885, 9_896, 9_920, 9_926, 9_932, 9_938, 9_950, 9_968, 9_986, 9_998, 10_010,
    10_022, 10_040, 10_088, 10_144, 10_195, 10_251, 10_307, 10_424, 10_445, 10_466, 10_492, 10_513,
    10_539, 10_560, 10_586, 10_607, 10_638, 10_669, 10_700, 10_731, 10_762, 10_852, 10_903, 10_954,
    11_005, 11_056, 11_107, 11_158, 11_209, 11_260, 11_372, 11_403, 11_434, 11_465, 11_496, 11_527,
    11_558, 11_589, 11_620, 11_651, 11_682, 11_713, 11_744, 11_775, 11_820, 11_878, 11_920,
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

fn resident_animation_field_ranges(
    data: &[u8],
    metadata: &[(usize, usize)],
    cells: &[usize],
    metadata_evidence: &'static str,
    cell_evidence: &'static str,
) -> Vec<NontextRange> {
    let mut ranges: Vec<_> = metadata
        .iter()
        .map(|&(offset, length)| NontextRange {
            offset,
            length,
            evidence: metadata_evidence,
        })
        .collect();
    for &cell in cells {
        ranges.push(NontextRange {
            offset: cell,
            length: 1,
            evidence: cell_evidence,
        });
        for part in 0..usize::from(data[cell]) {
            ranges.push(NontextRange {
                offset: cell + 2 + 5 * part,
                length: 4,
                evidence: cell_evidence,
            });
        }
    }
    ranges.sort_by_key(|range| range.offset);
    ranges
}

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
            Some(resident_animation_field_ranges(
                data,
                AREA024_ANIMATION_METADATA_RANGES,
                AREA024_REFERENCED_CELL_OFFSETS,
                "original US AREA024 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
                "original US AREA024 entry 4 referenced cell counts and signed X/Y, unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD8C; quad-path numeric fields only, no rectangle-helper behavior claim; packed flags, tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
            ))
        }
        (4, 198_656, 0, 0x800d_3800, 10_592)
            if sha256_hex(data)
                == "da67be8010dae83675d3c7e1731a236f9199cf93fb10a1d24e8c3ea8c6bfcf5f" =>
        {
            Some(resident_animation_field_ranges(
                data,
                AREA000_ANIMATION_METADATA_RANGES,
                AREA000_REFERENCED_CELL_OFFSETS,
                "original US AREA000 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
                "original US AREA000 entry 4 referenced cell counts and signed X/Y, unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD8C; quad-path numeric fields only, no rectangle-helper behavior claim; packed flags, tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
            ))
        }
        (4, 198_656, 0, 0x800d_3800, 11_968)
            if sha256_hex(data)
                == "3e4732e29ef64cd14c0eb1716cc6347c98ded808cafa2c21bc5c8bc10f1e3c41" =>
        {
            Some(resident_animation_field_ranges(
                data,
                AREA001_ANIMATION_METADATA_RANGES,
                AREA001_REFERENCED_CELL_OFFSETS,
                "original US AREA001 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
                "original US AREA001 entry 4 referenced cell counts and signed X/Y, unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD8C; quad-path numeric fields only, no rectangle-helper behavior claim; packed flags, tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
            ))
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
