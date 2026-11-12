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

/// Reviewed numeric fields and instruction bodies in exact original executables.
pub fn executable_nontext_ranges(data: &[u8]) -> Vec<NontextRange> {
    if data.len() == 122_880
        && sha256_hex(data) == "21b3f791e450ff308a874e3691d0c0c88df428464f9ad08ea2c7a680a6c0dd2c"
    {
        return vec![
            NontextRange {
                offset: 3928,
                length: 1828,
                evidence: "original US LOGO 0x801CE758..0x801CEE7C; twelve complete instruction-only function bodies; original bytes match freshly compiled executable ELF function symbols and reviewed source boundaries; exact executable identity; no runtime execution claim",
            },
            NontextRange {
                offset: 9700,
                length: 32,
                evidence: "original US LOGO 0x801CFDE4..0x801CFE04; complete CdFlush wrapper from PsyQ libcd.a s_008.o executable section with CD_flush relocation to 0x801D141C; original caller at 0x801D1E84 and reviewed call/return boundaries; exact executable identity; no runtime execution or callee classification claim",
            },
            NontextRange {
                offset: 9756,
                length: 52,
                evidence: "original US LOGO 0x801CFE1C..0x801CFE50; complete CdComstr instruction body from PsyQ libcd.a s_010.o after all five table/local-text/rdata relocations; reviewed branches join at return and delay slot; SDK text padding excluded; exact executable identity; no runtime execution, table/string classification or complete indirect-reference claim",
            },
            NontextRange {
                offset: 10908,
                length: 32,
                evidence: "original US LOGO 0x801D029C..0x801D02BC; complete PsyQ libcd.a s_020.o executable section with CD_vol relocation to 0x801D1394; original typed callers at 0x801CED74 and 0x801CEDD0; exact executable identity; no runtime execution or complete callee claim",
            },
            NontextRange {
                offset: 15252,
                length: 136,
                evidence: "original US LOGO 0x801D1394..0x801D141C; complete CD_vol instruction body from PsyQ libcd.a bios_1.o; all fourteen HI16/LO16 relocations and external pointer words match original bytes; typed CdlATV argument through original CdMix caller; reviewed straight-line body ends with return and delay slot before CD_flush; exact executable identity; no runtime execution or pointer-storage classification claim",
            },
            NontextRange {
                offset: 50_568,
                length: 160,
                evidence: "original US LOGO 0x801D9D88..0x801D9E28; 32 command and eight interrupt little-endian pointer words target supported literal allocations, including twelve aliases of the shared question mark; original table-loaded diagnostic printf arguments and exact executable identity; stored representation only, no arbitrary-index bound or runtime execution claim",
            },
        ];
    }
    if data.len() != 1_445_888 || sha256_hex(data) != US_EXECUTABLE_SHA256 {
        return Vec::new();
    }
    vec![
        NontextRange {
            offset: 739_844,
            length: 8,
            evidence: "original US SLUS; complete instruction body 0x8014AA04..0x8014AA0C including return delay slot; complete original word and control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 740_040,
            length: 472,
            evidence: "original US SLUS; complete instruction body 0x8014AAC8..0x8014ACA0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 66 text relocations reconcile with original and captured resident bytes; compiler epilogue 0x8014AC90..0x8014ACA0 is unreachable from the original entry and receives no executable-path claim; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 740_512,
            length: 136,
            evidence: "original US SLUS; complete instruction body 0x8014ACA0..0x8014AD28 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 740_648,
            length: 224,
            evidence: "original US SLUS; complete instruction body 0x8014AD28..0x8014AE08 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 24 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 740_872,
            length: 148,
            evidence: "original US SLUS; complete instruction body 0x8014AE08..0x8014AE9C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_020,
            length: 68,
            evidence: "original US SLUS; complete instruction body 0x8014AE9C..0x8014AEE0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and its single text relocation reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_088,
            length: 224,
            evidence: "original US SLUS; complete instruction body 0x8014AEE0..0x8014AFC0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_408,
            length: 208,
            evidence: "original US SLUS; complete original instruction body 0x8014B020..0x8014B0F0 including return delay slot; original JALs at 0x8014ABB0 and 0x8014AD0C in qualified executable callers conditionally enter this body; complete original word and control-flow review and captured resident bytes reconcile; the compiled callee remains partial and is not the admission basis; exact whole executable identity; neighboring functions and data excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_616,
            length: 140,
            evidence: "original US SLUS; complete instruction body 0x8014B0F0..0x8014B17C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_756,
            length: 40,
            evidence: "original US SLUS; complete instruction body 0x8014B17C..0x8014B1A4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 741_796,
            length: 408,
            evidence: "original US SLUS; complete instruction body 0x8014B1A4..0x8014B33C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 48 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 742_204,
            length: 136,
            evidence: "original US SLUS; complete instruction body 0x8014B33C..0x8014B3C4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_228,
            length: 280,
            evidence: "original US SLUS; complete instruction body 0x8014B73C..0x8014B854 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_508,
            length: 40,
            evidence: "original US SLUS; complete instruction body 0x8014B854..0x8014B87C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_548,
            length: 52,
            evidence: "original US SLUS; complete instruction body 0x8014B87C..0x8014B8B0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_600,
            length: 80,
            evidence: "original US SLUS; complete instruction body 0x8014B8B0..0x8014B900 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 743_680,
            length: 84,
            evidence: "original US SLUS; complete instruction body 0x8014B900..0x8014B954 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 752_956,
            length: 316,
            evidence: "original US SLUS; complete instruction body 0x8014DD3C..0x8014DE78 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 753_916,
            length: 304,
            evidence: "original US SLUS; complete instruction body 0x8014E0FC..0x8014E22C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 818_968,
            length: 2_684,
            evidence: "original US SLUS; complete instruction body 0x8015DF18..0x8015E994 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 408 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_416,
            length: 132,
            evidence: "original US SLUS; complete instruction body 0x80161F58..0x80161FDC including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_548,
            length: 388,
            evidence: "original US SLUS; complete instruction body 0x80161FDC..0x80162160 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 56 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_936,
            length: 24,
            evidence: "original US SLUS; complete instruction body 0x80162160..0x80162178 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 835_960,
            length: 112,
            evidence: "original US SLUS; complete instruction body 0x80162178..0x801621E8 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 836_072,
            length: 72,
            evidence: "original US SLUS; complete instruction body 0x801621E8..0x80162230 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 836_144,
            length: 720,
            evidence: "original US SLUS; complete instruction body 0x80162230..0x80162500 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 70 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 836_864,
            length: 228,
            evidence: "original US SLUS; complete instruction body 0x80162500..0x801625E4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_092,
            length: 52,
            evidence: "original US SLUS; complete instruction body 0x801625E4..0x80162618 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_144,
            length: 128,
            evidence: "original US SLUS; complete instruction body 0x80162618..0x80162698 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_272,
            length: 248,
            evidence: "original US SLUS; complete instruction body 0x80162698..0x80162790 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 32 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 837_784,
            length: 344,
            evidence: "original US SLUS; complete instruction body 0x80162898..0x801629F0 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 42 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_128,
            length: 124,
            evidence: "original US SLUS; complete instruction body 0x801629F0..0x80162A6C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_252,
            length: 156,
            evidence: "original US SLUS; complete instruction body 0x80162A6C..0x80162B08 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_408,
            length: 268,
            evidence: "original US SLUS; complete instruction body 0x80162B08..0x80162C14 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 33 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_676,
            length: 196,
            evidence: "original US SLUS; complete instruction body 0x80162C14..0x80162CD8 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 20 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_872,
            length: 40,
            evidence: "original US SLUS; complete instruction body 0x80162CD8..0x80162D00 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_912,
            length: 24,
            evidence: "original US SLUS; complete instruction body 0x80162D00..0x80162D18 including return delay slot; complete original word and control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 838_936,
            length: 132,
            evidence: "original US SLUS; complete instruction body 0x80162D18..0x80162D9C including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 849_140,
            length: 124,
            evidence: "original US SLUS; complete instruction body 0x801654F4..0x80165570 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 849_396,
            length: 160,
            evidence: "original US SLUS; complete instruction body 0x801655F4..0x80165694 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 851_272,
            length: 188,
            evidence: "original US SLUS; complete instruction body 0x80165D48..0x80165E04 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; five bounded internal switch targets, with the separate address table excluded; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 855_216,
            length: 60,
            evidence: "original US SLUS; complete instruction body 0x80166CB0..0x80166CEC including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 905_244,
            length: 156,
            evidence: "original US SLUS; complete instruction body 0x8017301C..0x801730B8 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_004,
            length: 56,
            evidence: "original US SLUS; complete instruction body 0x8017F27C..0x8017F2B4 including return delay slot; complete original word and control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact whole executable identity; neighboring functions, tables and text allocations excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
        },
        NontextRange {
            offset: 955_708,
            length: 32,
            evidence: "original US SLUS 0x8017F53C..0x8017F55C unsigned width/height pairs; bounded byte-pair consumer 0x8014C7F0..0x8014C818; exact executable identity; stored numeric representation only",
        },
    ]
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

// Individually checked original AREA002 entry 4 fields; excluded bytes stay unresolved.
const AREA002_ANIMATION_METADATA_RANGES: &[(usize, usize)] = &[
    (0, 108),
    (109, 5),
    (157, 5),
    (179, 5),
    (201, 5),
    (239, 5),
    (267, 25),
    (409, 25),
    (551, 45),
    (801, 25),
    (969, 5),
    (991, 5),
    (1_013, 17),
    (1_135, 5),
    (1_177, 25),
    (1_349, 17),
    (1_497, 17),
    (1_679, 9),
    (1_747, 9),
    (1_819, 5),
    (1_847, 5),
    (1_879, 5),
    (1_917, 5),
    (1_955, 5),
    (1_987, 5),
    (2_035, 5),
    (2_087, 13),
    (2_329, 23),
    (2_659, 15),
    (2_884, 20),
    (2_905, 5),
    (2_967, 5),
    (3_019, 5),
    (3_061, 5),
    (3_123, 5),
    (3_181, 25),
    (3_443, 25),
    (3_715, 25),
    (4_143, 17),
    (4_465, 21),
    (4_868, 46),
    (4_915, 5),
    (4_947, 5),
    (4_969, 5),
    (4_987, 5),
    (5_005, 5),
    (5_037, 25),
    (5_139, 25),
    (5_247, 9),
    (5_279, 5),
    (5_297, 5),
    (5_325, 17),
    (5_427, 5),
    (5_445, 5),
    (5_467, 17),
    (5_549, 17),
    (5_617, 17),
    (5_685, 5),
    (5_713, 5),
    (5_751, 9),
    (5_799, 5),
    (5_831, 13),
    (6_073, 23),
    (6_467, 11),
    (6_632, 20),
    (6_653, 5),
    (6_681, 5),
    (6_703, 5),
    (6_725, 5),
    (6_753, 5),
    (6_781, 25),
    (6_959, 25),
    (7_087, 5),
    (7_109, 21),
    (7_317, 11),
    (7_456, 18),
    (7_475, 5),
    (7_513, 5),
    (7_551, 5),
    (7_579, 5),
    (7_611, 5),
    (7_653, 25),
    (7_861, 25),
    (8_083, 13),
    (8_191, 5),
    (8_224, 24),
    (8_249, 5),
    (8_277, 5),
    (8_299, 5),
    (8_321, 5),
    (8_349, 5),
    (8_371, 25),
    (8_553, 25),
    (8_705, 9),
    (8_747, 5),
    (8_775, 5),
    (8_803, 17),
    (8_921, 17),
    (9_060, 20),
    (9_081, 5),
    (9_139, 5),
    (9_177, 5),
    (9_199, 5),
    (9_241, 5),
    (9_289, 25),
    (9_521, 25),
    (9_743, 13),
    (9_851, 9),
    (9_923, 5),
    (9_964, 6),
    (9_971, 5),
    (10_003, 25),
    (10_165, 25),
    (10_296, 32),
    (10_329, 5),
    (10_347, 5),
    (10_365, 5),
    (10_387, 5),
    (10_405, 19),
    (10_459, 17),
    (10_501, 5),
    (10_519, 5),
    (10_537, 5),
    (10_549, 5),
    (10_561, 5),
    (10_573, 5),
    (10_591, 5),
    (10_608, 2),
    (10_611, 17),
    (10_812, 4),
    (10_817, 5),
    (10_855, 5),
    (10_896, 4),
    (10_901, 5),
    (10_961, 5),
    (11_000, 2),
    (11_003, 9),
];

const AREA002_REFERENCED_CELL_OFFSETS: &[usize] = &[
    114, 162, 184, 206, 244, 292, 313, 334, 350, 371, 387, 434, 455, 471, 492, 513, 534, 596, 617,
    643, 664, 690, 721, 752, 783, 826, 847, 868, 889, 925, 946, 974, 996, 1_030, 1_061, 1_087,
    1_108, 1_140, 1_202, 1_228, 1_249, 1_275, 1_301, 1_327, 1_366, 1_397, 1_433, 1_459, 1_514,
    1_555, 1_596, 1_637, 1_688, 1_719, 1_756, 1_787, 1_824, 1_852, 1_884, 1_922, 1_960, 1_992,
    2_040, 2_100, 2_176, 2_252, 2_352, 2_428, 2_504, 2_575, 2_616, 2_674, 2_750, 2_816, 2_910,
    2_972, 3_024, 3_066, 3_128, 3_206, 3_252, 3_288, 3_329, 3_365, 3_406, 3_468, 3_504, 3_550,
    3_586, 3_632, 3_668, 3_740, 3_796, 3_847, 3_898, 3_979, 4_060, 4_160, 4_236, 4_312, 4_388,
    4_486, 4_562, 4_638, 4_714, 4_790, 4_920, 4_952, 4_974, 4_992, 5_010, 5_062, 5_083, 5_094,
    5_105, 5_116, 5_127, 5_164, 5_175, 5_186, 5_202, 5_218, 5_234, 5_256, 5_267, 5_284, 5_302,
    5_342, 5_353, 5_374, 5_405, 5_432, 5_450, 5_484, 5_500, 5_511, 5_537, 5_566, 5_577, 5_588,
    5_604, 5_634, 5_645, 5_656, 5_672, 5_690, 5_718, 5_760, 5_771, 5_804, 5_844, 5_920, 5_996,
    6_096, 6_172, 6_248, 6_324, 6_395, 6_478, 6_554, 6_658, 6_686, 6_708, 6_730, 6_758, 6_806,
    6_842, 6_868, 6_889, 6_910, 6_931, 6_984, 7_000, 7_016, 7_032, 7_053, 7_069, 7_092, 7_130,
    7_161, 7_217, 7_253, 7_284, 7_330, 7_391, 7_480, 7_518, 7_556, 7_584, 7_616, 7_678, 7_714,
    7_745, 7_776, 7_802, 7_828, 7_886, 7_917, 7_943, 7_984, 8_015, 8_056, 8_096, 8_127, 8_163,
    8_196, 8_254, 8_282, 8_304, 8_326, 8_354, 8_396, 8_427, 8_448, 8_479, 8_500, 8_531, 8_578,
    8_594, 8_620, 8_641, 8_657, 8_678, 8_714, 8_730, 8_752, 8_780, 8_820, 8_841, 8_867, 8_893,
    8_938, 8_959, 8_985, 9_021, 9_086, 9_144, 9_182, 9_204, 9_246, 9_314, 9_350, 9_386, 9_417,
    9_453, 9_484, 9_546, 9_577, 9_613, 9_644, 9_675, 9_706, 9_756, 9_787, 9_818, 9_860, 9_891,
    9_928, 9_976, 10_028, 10_059, 10_080, 10_101, 10_122, 10_143, 10_190, 10_211, 10_227, 10_243,
    10_259, 10_275, 10_334, 10_352, 10_370, 10_392, 10_424, 10_430, 10_441, 10_452, 10_476, 10_482,
    10_488, 10_494, 10_506, 10_524, 10_542, 10_554, 10_566, 10_578, 10_596, 10_628, 10_674, 10_720,
    10_766, 10_822, 10_860, 10_908, 10_966, 11_012, 11_058,
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
                offset: cell + 1 + 5 * part,
                length: 5,
                evidence: cell_evidence,
            });
        }
    }
    ranges.sort_by_key(|range| range.offset);
    ranges
}

fn area_surface_field_ranges(data: &[u8], check_word: u32) -> Option<Vec<NontextRange>> {
    let header = data.get(..22)?;
    if data.len() > 1024 * 1024
        || header[0] == 0
        || header[1] == 0
        || u32::from_le_bytes(header[..4].try_into().ok()?) != check_word
    {
        return None;
    }
    let half = |offset: usize| -> Option<usize> {
        Some(usize::from(u16::from_le_bytes(
            data.get(offset..offset.checked_add(2)?)?.try_into().ok()?,
        )))
    };
    let cells = usize::from(header[0]).checked_mul(usize::from(header[1]))?;
    let grid_offset = half(2)?.checked_mul(4)?;
    let grid_end = grid_offset.checked_add(cells.checked_mul(2)?)?;
    let grid = data.get(grid_offset..grid_end)?;
    let words_offset = grid_offset.checked_add(cells.div_ceil(2).checked_mul(4)?)?;
    let byte_grid = half(20)?.checked_mul(4)?;
    data.get(byte_grid..byte_grid.checked_add(cells)?)?;

    let mut spans = vec![(0, 4), (20, 22), (grid_offset, grid_end)];
    let mut ids: Vec<_> = grid
        .chunks_exact(2)
        .map(|pair| u16::from_le_bytes([pair[0], pair[1]]))
        .collect();
    ids.sort_unstable();
    ids.dedup();
    for id in ids {
        let id = usize::from(id);
        if id == 0 {
            continue;
        }
        let offset = words_offset.checked_add(id.checked_mul(4)?)?;
        let end = offset.checked_add(4)?;
        let word = u32::from_le_bytes(data.get(offset..end)?.try_into().ok()?);
        spans.push((offset, end));
        if word & 0xf00 == 0 {
            continue;
        }
        let paired = word & 0x800 != 0;
        let size = if paired { 8 } else { 4 };
        let field = (if paired { 6 } else { 4 }) + 4 * ((word >> 28) & 3) as usize;
        let index = (word & if paired { 0x7ff } else { 0xff }) as usize;
        let coordinates = half(field)?
            .checked_mul(4)?
            .checked_add(index.checked_mul(size)?)?;
        let end = coordinates.checked_add(size)?;
        data.get(coordinates..end)?;
        spans.push((field, field + 2));
        spans.push((coordinates, end));
    }
    spans.sort_unstable();
    let mut merged: Vec<(usize, usize)> = Vec::new();
    for (start, end) in spans {
        match merged.last_mut() {
            Some(previous) if start <= previous.1 => previous.1 = previous.1.max(end),
            _ => merged.push((start, end)),
        }
    }
    let evidence = "original US raw area surface dimensions, relative offsets, declared u16 cell IDs, nonzero-ID primary packed words and selected coordinate reads; original consumers 0x80154294..0x80154438, 0x80155560..0x801556FC and byte-grid pointer 0x80154094..0x801540F8; exact archive identity, checkword agreement and checked source bounds; unused headers, byte-grid contents, conditional sequential words, gaps and tails excluded; no natural visitation, full allocation, rendering or editing claim";
    Some(
        merged
            .into_iter()
            .map(|(offset, end)| NontextRange {
                offset,
                length: end - offset,
                evidence,
            })
            .collect(),
    )
}

fn compression_metadata_ranges(data: &[u8], check_word: u32) -> Option<Vec<NontextRange>> {
    let header = data.get(..6)?;
    if check_word == 0 || u32::from_le_bytes(header[..4].try_into().unwrap()) != check_word {
        return None;
    }
    let expected = usize::try_from(check_word).ok()?;
    let evidence = "original US type-1 EMI compression checkword, token controls, reference displacements, length extensions and terminator; original dispatch 0x80162420..0x80162444, checkword consumer 0x801623A8..0x80162420 and decoder 0x80164028..0x801644CC; exact archive identity and checked complete token walk; literal data and skipped prefix bytes excluded; no decoded-role, rasterized-text absence or runtime claim";
    let mut ranges = vec![NontextRange {
        offset: 0,
        length: 4,
        evidence,
    }];
    let mut at = 6;
    let mut decoded = 0_usize;
    loop {
        let start = at;
        let control = *data.get(at)?;
        at += 1;
        let terminal = control == 0xff;
        let literal = control & 0x10 != 0;
        let mut count = 0;
        if !terminal {
            if !literal {
                let low = usize::from(*data.get(at)?);
                at += 1;
                let distance = 2048 - ((usize::from(control & 0xe0) << 3) | low);
                if distance > decoded {
                    return None;
                }
            }
            count = if literal {
                32 - usize::from(control & 31)
            } else {
                usize::from(control & 15) + 3
            };
            if count == if literal { 16 } else { 18 } {
                count += usize::from(*data.get(at)?);
                at += 1;
            }
        }
        let previous = ranges.last_mut()?;
        if previous.offset + previous.length == start {
            previous.length += at - start;
        } else {
            ranges.push(NontextRange {
                offset: start,
                length: at - start,
                evidence,
            });
        }
        if terminal {
            return (at == data.len() && decoded == expected).then_some(ranges);
        }
        if literal {
            data.get(at..at.checked_add(count)?)?;
            at += count;
        }
        decoded = decoded.checked_add(count)?;
        if decoded > expected {
            return None;
        }
    }
}

/// Reviewed payload fields and instruction bodies; no display claim.
pub fn payload_nontext_ranges(
    entry: Entry,
    data: &[u8],
    original_us: bool,
) -> Option<Vec<NontextRange>> {
    if !original_us {
        return None;
    }
    if entry.transfer_mode == 1 {
        return compression_metadata_ranges(data, entry.check_word);
    }
    if entry.transfer_mode == 0 && entry.ram_ptr == 0x8010_4000 {
        return area_surface_field_ranges(data, entry.check_word);
    }
    match (entry.index, entry.offset, entry.transfer_mode, entry.ram_ptr, data.len()) {
        (_, _, 0, 0x801d_0c00, 118_224)
            if sha256_hex(data)
                == "f25ef137618bc024a6aec98c2c4d9a571c6f23884762fb4567406e6552b383c1" =>
        {
            Some(vec![
                NontextRange {
                    offset: 0x2c44,
                    length: 376,
                    evidence: "original US battle payload; complete instruction body 0x801D3844..0x801D39BC including return delay slot; complete original control-flow review, sized executable ELF function section and all 26 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x3c50,
                    length: 624,
                    evidence: "original US battle payload; complete instruction body 0x801D4850..0x801D4AC0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 54 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, battler-index-validity, record-validity, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4144,
                    length: 708,
                    evidence: "original US battle payload; complete instruction body 0x801D4D44..0x801D5008 including return delay slot; complete original control-flow review, sized executable ELF function section and all 34 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x48f8,
                    length: 352,
                    evidence: "original US battle payload; complete instruction body 0x801D54F8..0x801D5658 including return delay slot; complete original control-flow review, sized executable ELF function section and all 24 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4a58,
                    length: 340,
                    evidence: "original US battle payload; complete instruction body 0x801D5658..0x801D57AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4bac,
                    length: 352,
                    evidence: "original US battle payload; complete instruction body 0x801D57AC..0x801D590C including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4d0c,
                    length: 340,
                    evidence: "original US battle payload; complete instruction body 0x801D590C..0x801D5A60 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4e60,
                    length: 352,
                    evidence: "original US battle payload; complete instruction body 0x801D5A60..0x801D5BC0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x4fc0,
                    length: 524,
                    evidence: "original US battle payload; complete instruction body 0x801D5BC0..0x801D5DCC including return delay slot; complete original control-flow review, sized executable ELF function section and all 41 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x58c4,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801D64C4..0x801D656C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5b74,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6774..0x801D67B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5bb0,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D67B0..0x801D67EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5bec,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801D67EC..0x801D6818 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x5e14,
                    length: 624,
                    evidence: "original US battle payload; complete instruction body 0x801D6A14..0x801D6C84 including return delay slot; complete original control-flow review, sized executable ELF function section and all 62 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6084,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6C84..0x801D6CC0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x60c0,
                    length: 208,
                    evidence: "original US battle payload; complete instruction body 0x801D6CC0..0x801D6D90 including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6190,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801D6D90..0x801D6DE4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x61e4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6DE4..0x801D6E20 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6220,
                    length: 140,
                    evidence: "original US battle payload; complete instruction body 0x801D6E20..0x801D6EAC including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x62ac,
                    length: 64,
                    evidence: "original US battle payload; complete instruction body 0x801D6EAC..0x801D6EEC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x62ec,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D6EEC..0x801D6F28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6508,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801D7108..0x801D716C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x656c,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801D716C..0x801D71A4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x65a4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801D71A4..0x801D71E0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x65e0,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801D71E0..0x801D7234 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x68d4,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801D74D4..0x801D750C including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x690c,
                    length: 1_332,
                    evidence: "original US battle payload; complete original instruction body 0x801D750C..0x801D7A40 including return delay slot; original JAL at 0x801E91E4 in the already qualified executable caller at 0x801E91CC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x6e40,
                    length: 720,
                    evidence: "original US battle payload; complete original instruction body 0x801D7A40..0x801D7D10 including return delay slot; original JAL at 0x801E9424 in the already qualified executable caller at 0x801E9390 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7110,
                    length: 416,
                    evidence: "original US battle payload; complete instruction body 0x801D7D10..0x801D7EB0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x72b0,
                    length: 480,
                    evidence: "original US battle payload; complete instruction body 0x801D7EB0..0x801D8090 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7490,
                    length: 480,
                    evidence: "original US battle payload; complete instruction body 0x801D8090..0x801D8270 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7670,
                    length: 480,
                    evidence: "original US battle payload; complete instruction body 0x801D8270..0x801D8450 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7850,
                    length: 576,
                    evidence: "original US battle payload; complete instruction body 0x801D8450..0x801D8690 including return delay slot; complete original control-flow review, sized executable ELF function section and all 21 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7a90,
                    length: 1_108,
                    evidence: "original US battle payload; complete original instruction body 0x801D8690..0x801D8AE4 including return delay slot; original JAL at 0x801E9A98 in the already qualified executable caller at 0x801E99AC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x7ee4,
                    length: 788,
                    evidence: "original US battle payload; complete instruction body 0x801D8AE4..0x801D8DF8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 26 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x81f8,
                    length: 1_292,
                    evidence: "original US battle payload; complete original instruction body 0x801D8DF8..0x801D9304 including return delay slot; original JAL at 0x801EA15C in the already qualified executable caller at 0x801E9FDC conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8704,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801D9304..0x801D9388 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8788,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801D9388..0x801D93E4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x87e4,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801D93E4..0x801D9428 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8828,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801D9428..0x801D9484 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8884,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801D9484..0x801D94D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x88d4,
                    length: 432,
                    evidence: "original US battle payload; complete instruction body 0x801D94D4..0x801D9684 including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8a84,
                    length: 384,
                    evidence: "original US battle payload; complete instruction body 0x801D9684..0x801D9804 including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8c04,
                    length: 252,
                    evidence: "original US battle payload; complete instruction body 0x801D9804..0x801D9900 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8d00,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801D9900..0x801D99AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8dac,
                    length: 264,
                    evidence: "original US battle payload; complete instruction body 0x801D99AC..0x801D9AB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x8eb4,
                    length: 460,
                    evidence: "original US battle payload; complete instruction body 0x801D9AB4..0x801D9C80 including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x929c,
                    length: 476,
                    evidence: "original US battle payload; complete original instruction body 0x801D9E9C..0x801DA078 including return delay slot; original JAL at 0x801D8C70 in the already qualified executable caller at 0x801D8AE4 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9478,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801DA078..0x801DA14C including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x954c,
                    length: 476,
                    evidence: "original US battle payload; complete instruction body 0x801DA14C..0x801DA328 including return delay slot; complete original control-flow review, sized executable ELF function section and all 33 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9728,
                    length: 224,
                    evidence: "original US battle payload; complete instruction body 0x801DA328..0x801DA408 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9808,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801DA408..0x801DA4B4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x98b4,
                    length: 244,
                    evidence: "original US battle payload; complete instruction body 0x801DA4B4..0x801DA5A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x99a8,
                    length: 244,
                    evidence: "original US battle payload; complete instruction body 0x801DA5A8..0x801DA69C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x9a9c,
                    length: 312,
                    evidence: "original US battle payload; complete instruction body 0x801DA69C..0x801DA7D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa458,
                    length: 672,
                    evidence: "original US battle payload; complete instruction body 0x801DB058..0x801DB2F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 18 text relocations reconcile with original and captured resident bytes; four BREAK exception paths retained; exact archive and payload identities; neighboring functions and data excluded; no observed execution, arithmetic-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa6f8,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801DB2F8..0x801DB3A0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa7a0,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801DB3A0..0x801DB3E4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa7e4,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801DB3E4..0x801DB434 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa834,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801DB434..0x801DB494 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa894,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801DB494..0x801DB524 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa924,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801DB524..0x801DB5CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xa9cc,
                    length: 296,
                    evidence: "original US battle payload; complete instruction body 0x801DB5CC..0x801DB6F4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xaaf4,
                    length: 336,
                    evidence: "original US battle payload; complete instruction body 0x801DB6F4..0x801DB844 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xac44,
                    length: 416,
                    evidence: "original US battle payload; complete instruction body 0x801DB844..0x801DB9E4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xade4,
                    length: 316,
                    evidence: "original US battle payload; complete instruction body 0x801DB9E4..0x801DBB20 including return delay slot; complete original control-flow review, sized executable ELF function section and all 21 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xaf20,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801DBB20..0x801DBB78 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xaf78,
                    length: 1228,
                    evidence: "original US battle payload; complete instruction body 0x801DBB78..0x801DC044 including return delay slot; complete original control-flow review, sized executable ELF function section and all 91 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xb444,
                    length: 1_784,
                    evidence: "original US battle payload; complete original instruction body 0x801DC044..0x801DC73C including return delay slot; original JAL at 0x801DBBFC in the already qualified complete executable caller conditionally enters this body with a2=0xFFFF; complete original control-flow review and captured resident bytes reconcile; the compiled callee remains a partial mismatch and is not the admission basis; exact archive and payload identities; neighboring functions and data excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xbc94,
                    length: 580,
                    evidence: "original US battle payload; complete instruction body 0x801DC894..0x801DCAD8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc0b0,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801DCCB0..0x801DCD50 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; two BREAK exception paths retained; exact archive and payload identities; neighboring functions and data excluded; no observed execution, arithmetic-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc150,
                    length: 424,
                    evidence: "original US battle payload; complete instruction body 0x801DCD50..0x801DCEF8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc2f8,
                    length: 216,
                    evidence: "original US battle payload; complete instruction body 0x801DCEF8..0x801DCFD0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc3d0,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801DCFD0..0x801DD08C including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc48c,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801DD08C..0x801DD14C including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc54c,
                    length: 288,
                    evidence: "original US battle payload; complete instruction body 0x801DD14C..0x801DD26C including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc66c,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801DD26C..0x801DD29C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc69c,
                    length: 180,
                    evidence: "original US battle payload; complete instruction body 0x801DD29C..0x801DD350 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc750,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801DD350..0x801DD3CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc7cc,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801DD3CC..0x801DD448 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc848,
                    length: 340,
                    evidence: "original US battle payload; complete instruction body 0x801DD448..0x801DD59C including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xc99c,
                    length: 360,
                    evidence: "original US battle payload; complete instruction body 0x801DD59C..0x801DD704 including return delay slot; complete original control-flow review, sized executable ELF function section and all 23 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcb04,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801DD704..0x801DD760 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcb60,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DD760..0x801DD7AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcbac,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801DD7AC..0x801DD7D8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcbd8,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DD7D8..0x801DD800 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcc58,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801DD858..0x801DD8AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xccac,
                    length: 204,
                    evidence: "original US battle payload; complete instruction body 0x801DD8AC..0x801DD978 including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcd78,
                    length: 316,
                    evidence: "original US battle payload; complete instruction body 0x801DD978..0x801DDAB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xceb4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801DDAB4..0x801DDAF0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcef0,
                    length: 140,
                    evidence: "original US battle payload; complete instruction body 0x801DDAF0..0x801DDB7C including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xcf7c,
                    length: 312,
                    evidence: "original US battle payload; complete instruction body 0x801DDB7C..0x801DDCB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd0b4,
                    length: 456,
                    evidence: "original US battle payload; complete original instruction body 0x801DDCB4..0x801DDE7C including return delay slot; original JAL at 0x801D56BC in the already qualified executable caller at 0x801D5658 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd27c,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801DDE7C..0x801DDED8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd2d8,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DDED8..0x801DDF00 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd300,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DDF00..0x801DDF28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd328,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801DDF28..0x801DDF50 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd350,
                    length: 156,
                    evidence: "original US battle payload; complete instruction body 0x801DDF50..0x801DDFEC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd3ec,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801DDFEC..0x801DE0AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd4ac,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DE0AC..0x801DE0F8 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd4f8,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DE0F8..0x801DE144 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd544,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DE144..0x801DE190 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd590,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801DE190..0x801DE1B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd5b0,
                    length: 36,
                    evidence: "original US battle payload; complete instruction body 0x801DE1B0..0x801DE1D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd83c,
                    length: 292,
                    evidence: "original US battle payload; complete instruction body 0x801DE43C..0x801DE560 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xd960,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801DE560..0x801DE60C including return delay slot; complete original control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xda0c,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801DE60C..0x801DE690 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xda90,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801DE690..0x801DE698 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xda98,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801DE698..0x801DE754 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdb54,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801DE754..0x801DE7FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdbfc,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801DE7FC..0x801DE804 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdc04,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801DE804..0x801DE858 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdc58,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DE858..0x801DE8C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdcc0,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801DE8C0..0x801DE92C including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdd2c,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801DE92C..0x801DE94C including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdd4c,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801DE94C..0x801DE9A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdda8,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801DE9A8..0x801DEA18 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xde18,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801DEA18..0x801DEA64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xde64,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801DEA64..0x801DEAE0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xdfc4,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801DEBC4..0x801DEC28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe0e0,
                    length: 116,
                    evidence: "original US battle payload; complete instruction body 0x801DECE0..0x801DED54 including return delay slot; complete original control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe154,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801DED54..0x801DEDE4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe1e4,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DEDE4..0x801DEE4C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe24c,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DEE4C..0x801DEEB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe2b4,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801DEEB4..0x801DEF0C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe3dc,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801DEFDC..0x801DEFE4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe3e4,
                    length: 872,
                    evidence: "original US battle payload; complete instruction body 0x801DEFE4..0x801DF34C including return delay slot; complete original control-flow review, sized executable ELF function section and all 70 text relocations reconcile with original and captured resident bytes; six internal switch targets reconciled, separate address table excluded; exact archive and payload identities; neighboring functions and data excluded; no observed execution, selector-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe74c,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801DF34C..0x801DF3B8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe7b8,
                    length: 64,
                    evidence: "original US battle payload; complete instruction body 0x801DF3B8..0x801DF3F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xe7f8,
                    length: 880,
                    evidence: "original US battle payload; complete instruction body 0x801DF3F8..0x801DF768 including return delay slot; complete original control-flow review, sized executable ELF function section and all 64 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; indirect-call table and selector remain unqualified; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xeb68,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801DF768..0x801DF7EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xebec,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801DF7EC..0x801DF8AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xecac,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801DF8AC..0x801DF914 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xed14,
                    length: 264,
                    evidence: "original US battle payload; complete instruction body 0x801DF914..0x801DFA1C including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xee60,
                    length: 260,
                    evidence: "original US battle payload; complete instruction body 0x801DFA60..0x801DFB64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xef64,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801DFB64..0x801DFBDC including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf020,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801DFC20..0x801DFC70 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf070,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801DFC70..0x801DFCB4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf59c,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E019C..0x801E020C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf60c,
                    length: 608,
                    evidence: "original US battle payload; complete instruction body 0x801E020C..0x801E046C including return delay slot; complete original control-flow review, sized executable ELF function section and all 49 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf86c,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E046C..0x801E04B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf8b0,
                    length: 164,
                    evidence: "original US battle payload; complete instruction body 0x801E04B0..0x801E0554 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf954,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E0554..0x801E05EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xf9ec,
                    length: 256,
                    evidence: "original US battle payload; complete instruction body 0x801E05EC..0x801E06EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xfaec,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E06EC..0x801E0744 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xfb44,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E0744..0x801E0788 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xfb88,
                    length: 988,
                    evidence: "original US battle payload; complete instruction body 0x801E0788..0x801E0B64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 88 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0xff64,
                    length: 680,
                    evidence: "original US battle payload; complete instruction body 0x801E0B64..0x801E0E0C including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1020c,
                    length: 256,
                    evidence: "original US battle payload; complete instruction body 0x801E0E0C..0x801E0F0C including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1030c,
                    length: 324,
                    evidence: "original US battle payload; complete instruction body 0x801E0F0C..0x801E1050 including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10450,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801E1050..0x801E10C8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x104c8,
                    length: 464,
                    evidence: "original US battle payload; complete instruction body 0x801E10C8..0x801E1298 including return delay slot; complete original control-flow review, sized executable ELF function section and all 40 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10698,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E1298..0x801E12DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x106dc,
                    length: 220,
                    evidence: "original US battle payload; complete instruction body 0x801E12DC..0x801E13B8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x107b8,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E13B8..0x801E1450 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10850,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E1450..0x801E1494 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10894,
                    length: 312,
                    evidence: "original US battle payload; complete instruction body 0x801E1494..0x801E15CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x109cc,
                    length: 164,
                    evidence: "original US battle payload; complete instruction body 0x801E15CC..0x801E1670 including return delay slot; complete original control-flow review, sized executable ELF function section and all 17 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10ab4,
                    length: 468,
                    evidence: "original US battle payload; complete instruction body 0x801E16B4..0x801E1888 including return delay slot; complete original control-flow review, sized executable ELF function section and all 42 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10c88,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E1888..0x801E1938 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10d38,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E1938..0x801E1990 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10ddc,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801E19DC..0x801E1A14 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10e14,
                    length: 200,
                    evidence: "original US battle payload; complete instruction body 0x801E1A14..0x801E1ADC including return delay slot; complete original control-flow review, sized executable ELF function section and all 18 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10edc,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E1ADC..0x801E1AE4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10ee4,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801E1AE4..0x801E1B04 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10f04,
                    length: 40,
                    evidence: "original US battle payload; complete instruction body 0x801E1B04..0x801E1B2C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10f2c,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E1B2C..0x801E1B5C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10f5c,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E1B5C..0x801E1B64 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x10fa8,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E1BA8..0x801E1C58 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11058,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801E1C58..0x801E1C78 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11078,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E1C78..0x801E1CD8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1111c,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801E1D1C..0x801E1D88 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11188,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801E1D88..0x801E1DD4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x111d4,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801E1DD4..0x801E1E7C including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x112c0,
                    length: 356,
                    evidence: "original US battle payload; complete instruction body 0x801E1EC0..0x801E2024 including return delay slot; complete original control-flow review, sized executable ELF function section and all 31 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11424,
                    length: 252,
                    evidence: "original US battle payload; complete instruction body 0x801E2024..0x801E2120 including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11520,
                    length: 80,
                    evidence: "original US battle payload; complete instruction body 0x801E2120..0x801E2170 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11570,
                    length: 276,
                    evidence: "original US battle payload; complete instruction body 0x801E2170..0x801E2284 including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11684,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801E2284..0x801E2314 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11714,
                    length: 484,
                    evidence: "original US battle payload; complete instruction body 0x801E2314..0x801E24F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 32 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x118f8,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801E24F8..0x801E257C including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1197c,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E257C..0x801E25E0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x119e0,
                    length: 872,
                    evidence: "original US battle payload; complete instruction body 0x801E25E0..0x801E2948 including return delay slot; complete original control-flow review, sized executable ELF function section and all 63 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11d48,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801E2948..0x801E29B4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11db4,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E29B4..0x801E2A88 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; two BREAK exception paths retained; exact archive and payload identities; neighboring functions and data excluded; no observed execution, arithmetic-validity, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x11e88,
                    length: 540,
                    evidence: "original US battle payload; complete original instruction body 0x801E2A88..0x801E2CA4 including return delay slot; original JAL at 0x801E2860 in the already qualified executable caller at 0x801E25E0 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x120a4,
                    length: 168,
                    evidence: "original US battle payload; complete instruction body 0x801E2CA4..0x801E2D4C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1214c,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E2D4C..0x801E2D90 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12190,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E2D90..0x801E2E30 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x124b8,
                    length: 64,
                    evidence: "original US battle payload; complete instruction body 0x801E30B8..0x801E30F8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x124f8,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E30F8..0x801E3160 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12560,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E3160..0x801E31C8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1260c,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E320C..0x801E32AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 20 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x126f0,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E32F0..0x801E3334 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12734,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E3334..0x801E3398 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12798,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E3398..0x801E3438 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12838,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E3438..0x801E347C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1287c,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E347C..0x801E352C including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1292c,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801E352C..0x801E35D8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x129d8,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E35D8..0x801E3638 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12a38,
                    length: 516,
                    evidence: "original US battle payload; complete instruction body 0x801E3638..0x801E383C including return delay slot; complete original control-flow review, sized executable ELF function section and all 35 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12c3c,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801E383C..0x801E38C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12cc0,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801E38C0..0x801E3980 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12d80,
                    length: 128,
                    evidence: "original US battle payload; complete instruction body 0x801E3980..0x801E3A00 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12e44,
                    length: 192,
                    evidence: "original US battle payload; complete instruction body 0x801E3A44..0x801E3B04 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12f04,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E3B04..0x801E3B68 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12fac,
                    length: 36,
                    evidence: "original US battle payload; complete instruction body 0x801E3BAC..0x801E3BD0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x12fd0,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E3BD0..0x801E3C14 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13504,
                    length: 72,
                    evidence: "original US battle payload; complete instruction body 0x801E4104..0x801E414C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1354c,
                    length: 540,
                    evidence: "original US battle payload; complete instruction body 0x801E414C..0x801E4368 including return delay slot; complete original control-flow review, sized executable ELF function section and all 47 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13768,
                    length: 296,
                    evidence: "original US battle payload; complete instruction body 0x801E4368..0x801E4490 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13890,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E4490..0x801E44D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x138d4,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E44D4..0x801E4544 including return delay slot; complete original control-flow review, sized executable ELF function section and all 11 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13944,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E4544..0x801E45DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x139dc,
                    length: 272,
                    evidence: "original US battle payload; complete instruction body 0x801E45DC..0x801E46EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13aec,
                    length: 116,
                    evidence: "original US battle payload; complete instruction body 0x801E46EC..0x801E4760 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13b60,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E4760..0x801E47A4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13ba4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801E47A4..0x801E47E0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13be0,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801E47E0..0x801E483C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13c3c,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E483C..0x801E48D4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13cd4,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801E48D4..0x801E4928 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13d28,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E4928..0x801E496C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13d6c,
                    length: 228,
                    evidence: "original US battle payload; complete instruction body 0x801E496C..0x801E4A50 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13e50,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E4A50..0x801E4AE8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x13f2c,
                    length: 336,
                    evidence: "original US battle payload; complete instruction body 0x801E4B2C..0x801E4C7C including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1407c,
                    length: 184,
                    evidence: "original US battle payload; complete instruction body 0x801E4C7C..0x801E4D34 including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14134,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E4D34..0x801E4D8C including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x141d8,
                    length: 56,
                    evidence: "original US battle payload; complete instruction body 0x801E4DD8..0x801E4E10 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14210,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801E4E10..0x801E4ECC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x142cc,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E4ECC..0x801E4F34 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14334,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E4F34..0x801E4F64 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x143a8,
                    length: 280,
                    evidence: "original US battle payload; complete instruction body 0x801E4FA8..0x801E50C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 27 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x144c0,
                    length: 284,
                    evidence: "original US battle payload; complete instruction body 0x801E50C0..0x801E51DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x145dc,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801E51DC..0x801E5288 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14688,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E5288..0x801E52E8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x146e8,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E52E8..0x801E52F0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x146f0,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801E52F0..0x801E531C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1471c,
                    length: 464,
                    evidence: "original US battle payload; complete instruction body 0x801E531C..0x801E54EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 24 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14a7c,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E567C..0x801E5704 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14b04,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E5704..0x801E5768 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14b68,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E5768..0x801E57C8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14bc8,
                    length: 92,
                    evidence: "original US battle payload; complete instruction body 0x801E57C8..0x801E5824 including return delay slot; complete original control-flow review, sized executable ELF function section and all 9 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14c24,
                    length: 232,
                    evidence: "original US battle payload; complete instruction body 0x801E5824..0x801E590C including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14d0c,
                    length: 124,
                    evidence: "original US battle payload; complete instruction body 0x801E590C..0x801E5988 including return delay slot; complete original control-flow review, sized executable ELF function section and all nine text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, allocator-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14d88,
                    length: 176,
                    evidence: "original US battle payload; complete instruction body 0x801E5988..0x801E5A38 including the store in its return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; conditional entry through qualified caller 0x800AE09C; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, work-record-validity, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14e38,
                    length: 180,
                    evidence: "original US battle payload; complete instruction body 0x801E5A38..0x801E5AEC including return delay slot; complete original control-flow review, sized executable ELF function section and all 22 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14eec,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E5AEC..0x801E5AF4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14ef4,
                    length: 152,
                    evidence: "original US battle payload; complete instruction body 0x801E5AF4..0x801E5B8C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x14f8c,
                    length: 144,
                    evidence: "original US battle payload; complete instruction body 0x801E5B8C..0x801E5C1C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1501c,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E5C1C..0x801E5CA4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x150a4,
                    length: 392,
                    evidence: "original US battle payload; complete instruction body 0x801E5CA4..0x801E5E2C including return delay slot; complete original control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; five internal switch targets reconciled, separate address table excluded; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15488,
                    length: 84,
                    evidence: "original US battle payload; complete instruction body 0x801E6088..0x801E60DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x154dc,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801E60DC..0x801E6154 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15554,
                    length: 156,
                    evidence: "original US battle payload; complete instruction body 0x801E6154..0x801E61F0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x155f0,
                    length: 204,
                    evidence: "original US battle payload; complete instruction body 0x801E61F0..0x801E62BC including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x156bc,
                    length: 260,
                    evidence: "original US battle payload; complete instruction body 0x801E62BC..0x801E63C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x157c0,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E63C0..0x801E6420 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15820,
                    length: 148,
                    evidence: "original US battle payload; complete instruction body 0x801E6420..0x801E64B4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15b24,
                    length: 120,
                    evidence: "original US battle payload; complete instruction body 0x801E6724..0x801E679C including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15b9c,
                    length: 336,
                    evidence: "original US battle payload; complete instruction body 0x801E679C..0x801E68EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15cec,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801E68EC..0x801E6930 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15d30,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E6930..0x801E6990 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15d90,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E6990..0x801E69C0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15dc0,
                    length: 148,
                    evidence: "original US battle payload; complete instruction body 0x801E69C0..0x801E6A54 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15e54,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801E6A54..0x801E6AA0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15ea0,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E6AA0..0x801E6B28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 7 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15f28,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E6B28..0x801E6BC8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x15fc8,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E6BC8..0x801E6C20 including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16020,
                    length: 100,
                    evidence: "original US battle payload; complete instruction body 0x801E6C20..0x801E6C84 including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16084,
                    length: 164,
                    evidence: "original US battle payload; complete instruction body 0x801E6C84..0x801E6D28 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16128,
                    length: 300,
                    evidence: "original US battle payload; complete instruction body 0x801E6D28..0x801E6E54 including return delay slot; complete original control-flow review, sized executable ELF function section and all 19 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16254,
                    length: 332,
                    evidence: "original US battle payload; complete instruction body 0x801E6E54..0x801E6FA0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 31 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x163a0,
                    length: 16,
                    evidence: "original US battle payload; complete instruction body 0x801E6FA0..0x801E6FB0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x163b0,
                    length: 572,
                    evidence: "original US battle payload; complete instruction body 0x801E6FB0..0x801E71EC including return delay slot; complete original control-flow review, sized executable ELF function section and all 49 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x165ec,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801E71EC..0x801E72A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x166a8,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801E72A8..0x801E72F4 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x166f4,
                    length: 452,
                    evidence: "original US battle payload; complete instruction body 0x801E72F4..0x801E74B8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 43 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x168b8,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E74B8..0x801E7528 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16928,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E7528..0x801E7558 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16958,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E7558..0x801E75B0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x169b0,
                    length: 132,
                    evidence: "original US battle payload; complete instruction body 0x801E75B0..0x801E7634 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16a34,
                    length: 324,
                    evidence: "original US battle payload; complete instruction body 0x801E7634..0x801E7778 including return delay slot; complete original control-flow review, sized executable ELF function section and all 29 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16b78,
                    length: 160,
                    evidence: "original US battle payload; complete instruction body 0x801E7778..0x801E7818 including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16c18,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E7818..0x801E7888 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16c88,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801E7888..0x801E78F0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16cf0,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E78F0..0x801E7948 including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16d48,
                    length: 492,
                    evidence: "original US battle payload; complete instruction body 0x801E7948..0x801E7B34 including return delay slot; complete original control-flow review, sized executable ELF function section and all 43 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x16f34,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801E7B34..0x801E7BA0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x17958,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E8558..0x801E862C including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x17a2c,
                    length: 88,
                    evidence: "original US battle payload; complete instruction body 0x801E862C..0x801E8684 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18104,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E8D04..0x801E8DD8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 16 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x181d8,
                    length: 464,
                    evidence: "original US battle payload; complete original instruction body 0x801E8DD8..0x801E8FA8 including return delay slot; original JAL at 0x801E6F54 in the already qualified executable caller at 0x801E6E54 conditionally enters this body; complete original word and control-flow review and captured resident bytes reconcile; no matching callee ELF is claimed; exact archive and payload identities; neighboring functions, data and referenced callees excluded; no observed entry or execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x183a8,
                    length: 204,
                    evidence: "original US battle payload; complete instruction body 0x801E8FA8..0x801E9074 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18474,
                    length: 136,
                    evidence: "original US battle payload; complete instruction body 0x801E9074..0x801E90FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x184fc,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E90FC..0x801E915C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1855c,
                    length: 112,
                    evidence: "original US battle payload; complete instruction body 0x801E915C..0x801E91CC including return delay slot; complete original control-flow review, sized executable ELF function section and all 6 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x185cc,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801E91CC..0x801E91FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x185fc,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E91FC..0x801E925C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1865c,
                    length: 32,
                    evidence: "original US battle payload; complete instruction body 0x801E925C..0x801E927C including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18790,
                    length: 172,
                    evidence: "original US battle payload; complete instruction body 0x801E9390..0x801E943C including return delay slot; complete original control-flow review, sized executable ELF function section and all 12 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1883c,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E943C..0x801E949C including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x1889c,
                    length: 52,
                    evidence: "original US battle payload; complete instruction body 0x801E949C..0x801E94D0 including return delay slot; complete original control-flow review, sized executable ELF function section and all 5 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x188d0,
                    length: 96,
                    evidence: "original US battle payload; complete instruction body 0x801E94D0..0x801E9530 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18930,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801E9530..0x801E9538 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18938,
                    length: 212,
                    evidence: "original US battle payload; complete instruction body 0x801E9538..0x801E960C including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18a0c,
                    length: 412,
                    evidence: "original US battle payload; complete instruction body 0x801E960C..0x801E97A8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 27 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18d10,
                    length: 156,
                    evidence: "original US battle payload; complete instruction body 0x801E9910..0x801E99AC including return delay slot; complete original control-flow review, sized executable ELF function section and all 13 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18dac,
                    length: 364,
                    evidence: "original US battle payload; complete instruction body 0x801E99AC..0x801E9B18 including return delay slot; complete original control-flow review, sized executable ELF function section and all 28 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x18f18,
                    length: 400,
                    evidence: "original US battle payload; complete instruction body 0x801E9B18..0x801E9CA8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 25 text relocations reconcile with original and captured resident bytes; indirect callback target and selector remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19318,
                    length: 196,
                    evidence: "original US battle payload; complete instruction body 0x801E9F18..0x801E9FDC including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x193dc,
                    length: 408,
                    evidence: "original US battle payload; complete instruction body 0x801E9FDC..0x801EA174 including return delay slot; complete original control-flow review, sized executable ELF function section and all 32 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19574,
                    length: 48,
                    evidence: "original US battle payload; complete instruction body 0x801EA174..0x801EA1A4 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x195a4,
                    length: 60,
                    evidence: "original US battle payload; complete instruction body 0x801EA1A4..0x801EA1E0 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x195e0,
                    length: 116,
                    evidence: "original US battle payload; complete instruction body 0x801EA1E0..0x801EA254 including return delay slot; complete original control-flow review, sized executable ELF function section and all 2 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19930,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801EA530..0x801EA59C including return delay slot; complete original control-flow review, sized executable ELF function section and all 1 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19a50,
                    length: 216,
                    evidence: "original US battle payload; complete instruction body 0x801EA650..0x801EA728 including return delay slot; complete original control-flow review, sized executable ELF function section and all 14 text relocations reconcile with original and captured resident bytes; indirect-call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19b28,
                    length: 68,
                    evidence: "original US battle payload; complete instruction body 0x801EA728..0x801EA76C including return delay slot; complete original control-flow review, sized executable ELF function section and all 3 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19b6c,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801EA76C..0x801EA774 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19b74,
                    length: 104,
                    evidence: "original US battle payload; complete instruction body 0x801EA774..0x801EA7DC including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19bdc,
                    length: 108,
                    evidence: "original US battle payload; complete instruction body 0x801EA7DC..0x801EA848 including return delay slot; complete original control-flow review, sized executable ELF function section and all 4 text relocations reconcile with original and captured resident bytes; indirect call targets and selectors remain unqualified; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19c48,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801EA848..0x801EA894 including return delay slot; complete original control-flow review, sized executable ELF function section and all 8 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19cf4,
                    length: 264,
                    evidence: "original US battle payload; complete instruction body 0x801EA8F4..0x801EA9FC including return delay slot; complete original control-flow review, sized executable ELF function section and all 21 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19dfc,
                    length: 188,
                    evidence: "original US battle payload; complete instruction body 0x801EA9FC..0x801EAAB8 including return delay slot; complete original control-flow review, sized executable ELF function section and all 15 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19eb8,
                    length: 128,
                    evidence: "original US battle payload; complete instruction body 0x801EAAB8..0x801EAB38 including return delay slot; complete original control-flow review, sized executable ELF function section and all 10 text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19f38,
                    length: 44,
                    evidence: "original US battle payload; complete instruction body 0x801EAB38..0x801EAB64 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19f64,
                    length: 8,
                    evidence: "original US battle payload; complete instruction body 0x801EAB64..0x801EAB6C including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
                NontextRange {
                    offset: 0x19f6c,
                    length: 76,
                    evidence: "original US battle payload; complete instruction body 0x801EAB6C..0x801EABB8 including return delay slot; complete original control-flow review, sized executable ELF function section and complete object text without text relocations reconcile with original and captured resident bytes; exact archive and payload identities; neighboring functions and data excluded; no observed execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
                },
            ])
        }
        (_, _, 0, 0x8009_6800, 133_316)
            if sha256_hex(data)
                == "77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140" =>
        {
            Some([
                (0x15f54, 124),
                (0x15fd0, 116),
                (0x16044, 44),
                (0x16070, 196),
                (0x16134, 48),
            ]
            .into_iter()
            .map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; five complete instruction bodies spanning 0x800AC754..0x800AC964 including return delay slots; original bytes match sized executable ELF function sections and captured resident bytes; exact archive and payload identities; neighboring functions and tables excluded; no argument-validity, resource-safety, unique-supplier or duplicate-source ownership claim",
            })
            .chain([NontextRange {
                offset: 0x16164,
                length: 1564,
                evidence: "original US shared battle payload; complete instruction body 0x800AC964..0x800ACF80 including return delay slot; native entry PC 0x800AC964 and RA 0x801D1A80 bind the complete original control-flow review; original bytes match canonical payload and captured resident bytes; exact archive and payload identities; no ELF-match, full-function execution, argument-validity, resource-safety, callee-semantics or duplicate-source ownership claim",
            }])
            .chain([(0x16780, 116), (0x167f4, 80)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; two complete instruction bodies spanning 0x800ACF80..0x800AD044 including return delay slots; original bytes match complete sized executable ELF function sections and captured resident bytes, with text relocations reconciled; exact archive and payload identities; neighboring functions and tables excluded; no argument-validity, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([(0x16844, 48), (0x16874, 52), (0x168a8, 452), (0x16a6c, 128)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; four complete instruction bodies spanning 0x800AD044..0x800AD2EC including return delay slots; original bytes match complete sized executable ELF function sections and captured resident bytes, with text relocations reconciled; exact archive and payload identities; neighboring functions and tables excluded; no argument-validity, selector-bound, indirect-call-safety, resource-safety, unique-supplier or duplicate-source ownership claim",
            }))
            .chain([(0x16aec, 328), (0x16c34, 160)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; two complete instruction bodies spanning 0x800AD2EC..0x800AD4D4 including return delay slots; complete original control-flow review and conditional entries through original table 0x80096994 consumed by qualified dispatcher 0x800AD26C; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, observed-execution, selector-bound, indirect-call-safety, argument-validity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([NontextRange {
                offset: 0x16e7c,
                length: 32,
                evidence: "original US shared battle payload; complete instruction body 0x800AD67C..0x800AD69C including return delay slot; original bytes match a complete sized executable ELF function section and captured resident bytes, with its call relocation reconciled; conditional entry through original table 0x80096994 consumed by qualified dispatcher 0x800AD26C; exact archive and payload identities; intervening and neighboring bodies excluded; no observed-execution, selector-bound, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }, NontextRange {
                offset: 0x194,
                length: 12,
                evidence: "original US shared battle payload; three numeric function-address words at 0x80096994..0x800969A0; original dispatcher 0x800AD26C loads all three words, copies them to its stack and conditionally calls the indexed address; original words match captured resident bytes; exact archive and payload identities; surrounding header bytes excluded; no selector-bound, current-work-validity, pointer-lifetime or observed-invocation claim",
            }])
            .chain([(0x16cd4, 244), (0x16dc8, 180)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; two complete instruction bodies spanning 0x800AD4D4..0x800AD67C including return delay slots; original bytes match complete sized executable ELF function sections and captured resident bytes, with all text relocations reconciled; qualified dispatcher 0x800AD434 constructs their addresses for conditional work-byte-2 selection; exact archive and payload identities; no observed-execution, valid-work-record, selector-bound, list-integrity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([(0x16e9c, 128), (0x17130, 124), (0x171ac, 32), (0x171cc, 128), (0x174a4, 32), (0x174c4, 128), (0x1779c, 32), (0x177bc, 88), (0x17814, 88), (0x1786c, 48)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; ten complete instruction bodies within 0x800AD69C..0x800AE09C including return delay slots; complete original control-flow reviews and sized executable ELF function sections match original and captured resident bytes, with all text relocations reconciled; exact archive and payload identities; no observed-execution, valid-work-record, selector-bound, indirect-call-safety, argument-validity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([(0x16f1c, 236), (0x17008, 208), (0x170d8, 88), (0x1724c, 296), (0x17374, 304), (0x17544, 296), (0x1766c, 304)].into_iter().map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US shared battle payload; seven complete instruction bodies within 0x800AD71C..0x800ADF9C including return delay slots; complete original control-flow reviews and conditional entries through reviewed callers that load handler addresses from tables or construct them on the stack; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, observed-execution, valid-work-record, selector-bound, indirect-call-safety, argument-validity, resource-safety or duplicate-source ownership claim",
            }))
            .chain([0x1a0, 0x1ac, 0x1b8].into_iter().map(|offset| NontextRange {
                offset,
                length: 12,
                evidence: "original US shared battle payload; three numeric function-address words in each original table at 0x800969A0, 0x800969AC and 0x800969B8; complete original dispatchers 0x800AD69C, 0x800AD9CC and 0x800ADCC4 read all three words, copy them to their stacks and conditionally call the indexed address; all table words and complete target bodies match original and captured resident bytes; exact archive and payload identities; surrounding header bytes excluded; no selector-bound, current-work-validity, pointer-lifetime, observed-invocation, resource-safety or duplicate-source ownership claim",
            }))
            .chain([NontextRange {
                offset: 0x1789c,
                length: 108,
                evidence: "original US shared battle payload; complete instruction body 0x800AE09C..0x800AE108 including return delay slot; complete original control-flow review and conditional selector-1 entry through qualified dispatcher 0x800AE014; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, observed-execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }])
            .chain([NontextRange {
                offset: 0x17908,
                length: 240,
                evidence: "original US shared battle payload; complete instruction body 0x800AE108..0x800AE1F8 including return delay slots; complete original control-flow review and captured native entry PC 0x800AE108 with RA 0x801D39F0 from original caller 0x801D39E8; original bytes match captured resident bytes; call continuations assume returns; exact archive and payload identities; no ELF-match, full-function execution, valid-work-record, selector-bound, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }])
            .chain([NontextRange {
                offset: 0x179f8,
                length: 104,
                evidence: "original US shared battle payload; complete instruction body 0x800AE1F8..0x800AE260 including return delay slot; complete original control-flow review and captured native entry PC 0x800AE1F8 with RA 0x801D381C from original caller 0x801D3814; original bytes match captured resident bytes; call continuation assumes return; exact archive and payload identities; no ELF-match, full-function execution, argument-validity, callee-semantics, resource-safety or duplicate-source ownership claim",
            }])
            .collect())
        }
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
                "original US AREA024 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
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
                "original US AREA000 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
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
                "original US AREA001 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
            ))
        }
        (4, 198_656, 0, 0x800d_3800, 11_104)
            if sha256_hex(data)
                == "4171a51723373dab664aa85e07361672d26942763d5a5f0ce608c85980679c06" =>
        {
            Some(resident_animation_field_ranges(
                data,
                AREA002_ANIMATION_METADATA_RANGES,
                AREA002_REFERENCED_CELL_OFFSETS,
                "original US AREA002 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
                "original US AREA002 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
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
