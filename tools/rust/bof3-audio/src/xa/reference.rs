//! Whole-file references independently compared with the original US disc extents.
//! Hashes identify the known truncated extraction without guessing from filenames.

use serde::Serialize;

#[derive(Clone, Debug, Serialize)]
pub struct Completeness {
    pub status: &'static str,
    pub reference_source: &'static str,
    pub actual_sectors: usize,
    pub expected_sectors: usize,
    pub missing_audio_sectors: usize,
}

impl Completeness {
    pub fn is_truncated(&self) -> bool {
        self.actual_sectors < self.expected_sectors
    }
}

pub fn identify(hash: &str) -> Option<Completeness> {
    for (path, expected, prior, missing_audio, full_hash, prior_hash) in [
        (
            "BIN/BMAG_XA/MAGIC00.STR",
            15408,
            13509,
            1347,
            "9916db03238af4469843db0e71891ab6680e8a3a9e755bcb2d96e2dfbf92d3fa",
            "42361df114bf2c2128a4951fbba3c7850e717dac11807a235034acb6c79168d3",
        ),
        (
            "BIN/SCE_XA/S_XA00.STR",
            40416,
            35434,
            1238,
            "4dd8d034d4524aa907be793da1450f9f85cb67cd837ba412f1e328b719e0a37d",
            "fe01331f117f5130477c1d8e38e216a8a2e01a930046fe055bfc575e0f400bb0",
        ),
        (
            "BIN/SCE_XA/VOICE.STR",
            3536,
            3101,
            0,
            "f4f9d8ce9a2849501e8cd44dfcfee85aad4bc92cd9ff5113acde43c9362c65cc",
            "021c39dc0774973048deccc909e534f364dba806daf0cfc8ba0f36cc2c239d93",
        ),
        (
            "LOGO/CAPCOM30.STR",
            1155,
            1013,
            18,
            "c496080be2e134dba766e89a52bacbae7616239e1e26fe9d0c1242a92d12e915",
            "0f9145e980e401ded21f4c315375bcb989f49b8b83582f46f4a2946dd33ff06d",
        ),
    ] {
        if hash == full_hash {
            return Some(Completeness {
                status: "known_complete_disc_extent",
                reference_source: path,
                actual_sectors: expected,
                expected_sectors: expected,
                missing_audio_sectors: 0,
            });
        }
        if hash == prior_hash {
            return Some(Completeness {
                status: "known_truncated_disc_extent",
                reference_source: path,
                actual_sectors: prior,
                expected_sectors: expected,
                missing_audio_sectors: missing_audio,
            });
        }
    }
    None
}
