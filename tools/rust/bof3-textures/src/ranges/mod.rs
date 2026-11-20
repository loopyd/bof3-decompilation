//! Source-bound graphics byte roles; opaque bytes remain unclassified.

mod api;
mod compressed;
mod copies;
mod resident;
mod sprite;

use crate::digest::sha256_hex;

pub use api::{classify, identity};

#[derive(Clone, Copy, serde::Serialize)]
struct Entry {
    index: usize,
    offset: usize,
    size: u32,
    ram_ptr: u32,
    transfer_mode: u16,
    check_word: u32,
}

#[derive(serde::Serialize)]
struct NontextRange {
    offset: usize,
    length: usize,
    evidence: &'static str,
}

use self::compressed::{
    additional_model_decoded_nontext_fields, compression_metadata_ranges,
    model_decoded_nontext_fields, model_program_decoded_nontext_fields,
};
use self::copies::{
    stored_image_copy_ranges, stored_raw_sprite_record_copy_ranges,
    stored_resident_record_copy_ranges, stored_sprite_record_copy_ranges,
};
use self::resident::{
    BPLU349_GRAPHICS_RANGES, area_surface_field_ranges, resident_animation_payload_ranges,
};
use self::sprite::{
    BPL_FLAG_SPRITE_EVIDENCE, BPLU_SELECTED_SPRITE_EVIDENCE, bpl_flag_sprite_fields,
    bpl_selected_sprite_fields, raw_sprite_program_ranges,
};

const BATTLE_RESIDENT_FIELD_EVIDENCE: &str = "Original US battle resident animation fields at 0x800F0800. Exact whole-archive, raw-entry descriptor and payload identities bind the co-resident 0x801D0C00 consumer. Original dispatcher 0x801E6C84 temporarily publishes this base. Initialized state 1 selects group 1 and programs 3/4/5 through mask 0x58; state 3 advances their finite forward walks and queues the same work pointer. SLUS 0x8014BB20 restores that queued pointer, and 0x8014C4D4 reads the retained work+0x54 table and work+0x5A offset after the shared base changes. Only fetched root/program/cell numeric fields qualify. Ordinary noninterfering returns, valid nonaliasing memory, stable bank/work/pointers, queue capacity, enabled drain and visible-coordinate guards remain conditions. Program header byte zero, unselected programs/cells, gaps, tails and control bits 0x70 retain their prior status. No actual invocation, loading, residency, upload, display, rasterized-text absence or safe-edit claim.";
fn reviewed_texture_ranges(entry: Entry, data: &[u8]) -> Option<Vec<NontextRange>> {
    if entry.transfer_mode == 0
        && entry.index == 3
        && entry.offset == 26_624
        && entry.ram_ptr == 0x8003_b80c
        && entry.check_word == 0x10
        && entry.size == 313_320
        && data.len() == 313_320
        && sha256_hex(data) == "6669621720080f29b7a431d150ade295210ecef49d1dec3085b4e828c962f538"
    {
        return Some(
            [
                (0, 8),
                (16, 32),
                (48, 38_240),
                (38_288, 61_448),
                (99_736, 35_508),
                (135_244, 36_872),
                (172_120, 38_240),
                (210_360, 61_448),
                (271_808, 9_568),
                (281_376, 9_568),
                (290_944, 544),
            ]
            .into_iter()
            .map(|(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US DRG04_00 entry 3 image/palette section and directory words, eight image headers with consumed packed/raw words, and eight 32-halfword palettes; original MAGIC151 entry 3 state table 0x801EEC24 selects 0x801EF388/0x801EF484, form code 11 selects slot 545 through original BATTLE entry 15 cells 0x800B49F4/0x800B49F8 and u16 0x800B4B5C; original SLUS LBA table and complete disc user bytes bind that slot to this archive; after successful unskipped loading, MAGIC151 call 0x801EF87C supplies the first hint-row identifier and A1=0 to GAME 0x801C1254, which selects ordinal 0 and publishes root 0x8003B80C at 0x801C1308 if the original root-array word 12 survives; palette calls 0x801C13DC/0x801EF8A4 reach SLUS 0x80166C48, and finite valid image selectors reach 0x8014DC08 through 0x801EF8F8/0x8014D8D4/0x8014D6B8/0x8014D6DC before queued consumption at 0x8014E6D0 and 0x8014E7F8 or 0x8017B690; exact original archive, entry geometry, mode, load, checkword, size and payload identities; stable original hint 0..18, retained array header, valid separate palette indices 0..7 and image-selector cases, clear image-skip bit, changed request selector, queue capacity, bounded destinations, nonaliasing and ordinary returning helpers remain conditions; program/selection storage and allocation gaps excluded; no natural selector production, global caller bounds, actual request, residency, hardware transfer, live CLUT, display, rasterized-text absence or safe-edit claim",
            })
            .collect(),
        );
    }
    let form_sprite_ranges: &[(usize, usize)] = if entry.index == 3
        && entry.transfer_mode == 0
        && entry.ram_ptr == 0x8003_b80c
        && entry.check_word == 0x10
        && usize::try_from(entry.size).ok() == Some(data.len())
    {
        match (entry.offset, data.len(), sha256_hex(data).as_str()) {
            // DRG00_00, slot 525.
            (
                40_960,
                44_404,
                "1effd0ac6e77259713f70e51130634e848ed75bc66ddb3a5961d9476697d92d1",
            ) => &[(0, 8), (16, 21_052)],
            // DRG01_00, slot 530.
            (
                40_960,
                38_844,
                "2d8dada5f5112fbc1363e54a2b12ed8d31cd12df53f7456d42015d65e1c47c33",
            ) => &[
                (0, 8),
                (16, 10_984),
                (11_004, 6_035),
                (17_040, 8),
                (17_052, 6_199),
                (23_252, 8),
                (23_264, 5_244),
                (28_512, 4_443),
                (32_956, 3_296),
            ],
            // DRG03_00, slot 540.
            (
                40_960,
                76_072,
                "0d918e6fdd6852c3d184213423c02b2aa46ea0cee36d156153e22b55a546d574",
            ) => &[
                (0, 8),
                (16, 8_232),
                (8_252, 9_576),
                (17_832, 5_761),
                (23_596, 8),
                (23_608, 5_396),
                (29_008, 5_090),
                (34_100, 8),
                (34_112, 5_607),
                (39_720, 8_200),
                (47_924, 8_200),
                (56_128, 544),
            ],
            // DRG05_00, slot 546.
            (
                38_912,
                70_700,
                "4297311b37aa27df957dbacaca840945875fafab5c4fab1238a34252938e6ac7",
            ) => &[
                (0, 8),
                (16, 23_260),
                (23_280, 8),
                (23_292, 8_457),
                (31_752, 8),
                (31_764, 8_063),
                (39_828, 8),
                (39_840, 8_189),
                (48_032, 8),
                (48_044, 8_027),
                (56_072, 6_024),
            ],
            // DRG06_00, slot 551.
            (
                34_816,
                233_124,
                "ccc2b49c546625cb6fbdf3d51996d3eba0ec25bb2b3c71fdf8e5b753a9d872a6",
            ) => &[
                (0, 8),
                (16, 58_764),
                (58_784, 122_904),
                (181_692, 15_087),
                (196_780, 8),
                (196_792, 12_378),
                (209_172, 544),
            ],
            // DRG07_00, slot 552.
            (
                38_912,
                42_264,
                "c8743de97beea25057b3db3bdf89819fc7e43ce89483fe30d8ea54c297a2338f",
            ) => &[
                (0, 8),
                (16, 4_136),
                (4_156, 5_480),
                (9_640, 5_646),
                (15_288, 8),
                (15_300, 5_253),
                (20_556, 8),
                (20_568, 5_175),
                (25_744, 8),
                (25_756, 4_815),
                (30_572, 6_024),
            ],
            // PAPYD00, slot 562.
            (
                32_768,
                11_796,
                "feea0916b287895af705b3598c55581a2fdaa3657ce7e0b3efff68a47f1c0338",
            ) => &[(0, 8), (16, 6_872), (6_892, 631), (7_524, 1_920)],
            // PAPYU00, slot 567.
            (
                32_768,
                17_400,
                "f9d984db05b512d943860a31776800a6678c772bf322f265f4339f0f3704b864",
            ) => &[(0, 8), (16, 15_636)],
            _ => &[],
        }
    } else {
        &[]
    };
    if !form_sprite_ranges.is_empty() {
        return Some(
            form_sprite_ranges
                .iter()
                .map(|&(offset, length)| NontextRange {
                    offset,
                    length,
                    evidence: "original US standalone form-bank entry-3 image and palette section/directory words, image headers with consumed packed/raw words or mode-2 tokens, and eight 32-halfword palettes; original MAGIC151 state handlers 0x801EF388/0x801EF484, original BATTLE entry-15 form-pointer pairs and their first u16 slots bind the eight documented form/archive cases through the SLUS LBA table and complete matching physical disc user bytes; after successful unskipped loading, call 0x801EF87C supplies the first stable hint-row identifier and A1=0 to GAME 0x801C1254, which selects ordinal 0 and publishes root 0x8003B80C if the original root-array word 12 survives; palette calls reach SLUS 0x80166C48; separately valid documented image selectors reach 0x8014DC08 through 0x801EF8F8/0x8014D8D4/0x8014D6B8/0x8014D6DC before queued consumption at 0x8014E6D0 and its packed/raw/mode-2 helpers; exact original archive and entry/offset/mode/load/checkword/size/payload identities; stable hint 0..18, retained header, separate palette indices 0..7, finite image cases, changed request selector, clear image-skip bit, valid animation records, queue capacity, bounded destinations including final-token overshoot, nonaliasing and ordinary returning helpers remain conditions; opaque mode-2 prefixes, allocation tails, program/selection storage and other archives excluded; no natural selector production, general caller bounds, actual request, invocation, residency, hardware transfer, live CLUT, display, rasterized-text absence or safe-edit claim",
                })
                .collect(),
        );
    }
    let further_form_sprite_ranges: &[(usize, usize)] = if entry.index == 3
        && entry.transfer_mode == 0
        && entry.ram_ptr == 0x8003_b80c
        && entry.check_word == 0x10
        && usize::try_from(entry.size).ok() == Some(data.len())
    {
        match (entry.offset, data.len(), sha256_hex(data).as_str()) {
            // CRYUD00, slot 521.
            (
                40_960,
                110_592,
                "0cb3bd788cbdf439572efa1e6f33c544d0ce1ee9cef6c0aa3ca55d605d9c7db9",
            ) => &[
                (0, 8),
                (16, 68),
                (88, 824),
                (916, 1_907),
                (2_824, 9_584),
                (12_412, 1_464),
                (13_880, 1_485),
                (15_368, 8),
                (15_380, 878),
                (16_260, 8),
                (16_272, 6_308),
                (22_584, 8),
                (22_596, 1_593),
                (24_192, 8),
                (24_204, 1_866),
                (26_072, 5_480),
                (31_556, 2_496),
                (34_056, 4_369),
                (38_428, 8),
                (38_440, 321),
                (38_764, 68),
            ],
            // CRYUU00, slot 523.
            (
                40_960,
                110_592,
                "440f35b120293916a9d19a7c60a6589dfe47800b9e8dbab95de24d25a4d4f58e",
            ) => &[
                (0, 8),
                (16, 32),
                (52, 5_504),
                (5_560, 1_382),
                (6_944, 8),
                (6_956, 2_241),
                (9_200, 8),
                (9_212, 761),
                (9_976, 8),
                (9_988, 824),
                (10_816, 1_599),
                (12_416, 2_748),
                (15_168, 1_567),
                (20_552, 6_844),
                (27_400, 4_104),
                (31_508, 8),
                (31_520, 1_684),
                (33_208, 4_245),
                (37_456, 8),
                (37_468, 321),
                (37_792, 68),
            ],
            // DRG02_00, slot 535.
            (
                20_480,
                72_420,
                "46af6dbcedb4eab31ae5d2db9195c3dbc968840c316d08d04b1ca95a1762efb3",
            ) => &[
                (0, 8),
                (16, 12_328),
                (12_348, 13_672),
                (26_024, 6_850),
                (32_876, 8),
                (32_888, 7_098),
                (39_988, 8),
                (40_000, 7_408),
                (47_412, 14_440),
                (61_856, 544),
            ],
            // DRG08_00, slot 557.
            (
                20_480,
                84_732,
                "ecb3e431738d1e065eb0e90a47c392368be4fa84a08fb90ec928659c40d19b36",
            ) => &[
                (0, 8),
                (16, 12_328),
                (12_348, 8),
                (12_360, 5_700),
                (18_064, 6_533),
                (24_600, 8_200),
                (32_804, 12_296),
                (45_104, 8),
                (45_116, 5_893),
                (51_012, 8),
                (51_024, 14_380),
            ],
            _ => &[],
        }
    } else {
        &[]
    };
    if !further_form_sprite_ranges.is_empty() {
        return Some(
            further_form_sprite_ranges
                .iter()
                .map(|&(offset, length)| NontextRange {
                    offset,
                    length,
                    evidence: "Original US DRG02_00/DRG08_00/CRYUD00/CRYUU00 entry 3 image and palette section words, selected image directory words, consumed image headers/packed words/mode-2 tokens, and palette directory words with 32-halfword records. Original MAGIC151 first-u16 request and post-load handlers 0x801EF388/0x801EF484 use BATTLE entry 15 form 21..25 pointer cells 0x800B4A44..0x800B4A68 to select slots 535/557/521/523, bound through the SLUS LBA table and complete matching physical disc user bytes. Successful unskipped loading, a stable hint in 0..18 and retained root-array word 12 let GAME initializer 0x801C1254 publish ordinal 0 at 0x8003B80C. Palette calls reach SLUS 0x80166C48; finite documented image cases reach 0x8014DC08 and queued consumption at 0x8014E6D0 with its original packed/raw/mode-2 readers. Qualification requires exact original archive and entry/offset/mode/load/checkword/size/payload identities. Request work byte 8 selects CRYUD00 below 2 or CRYUU00 otherwise. Separate palette indices 0..7 for DRG and 0 for CRYU, a changed request selector, clear image-skip bit, valid animation records, queue capacity, bounded destinations including final-token overshoot, nonaliasing, stable original code/tables/source and ordinary helper returns remain conditions. CRYUU00 image slot 8 and its directory word, opaque mode-2 prefixes, allocation tails, programs and selection storage are excluded. No natural selector production, general caller bounds, actual invocation/request, residency, transfer, live CLUT, display, rasterized-text absence or safe-edit claim.",
                })
                .collect(),
        );
    }
    let replacement_sprite_ranges: &[(usize, usize)] = if entry.index == 3
        && entry.transfer_mode == 0
        && entry.check_word == 0x10
        && usize::try_from(entry.size).ok() == Some(data.len())
    {
        match (
            entry.offset,
            entry.ram_ptr,
            data.len(),
            sha256_hex(data).as_str(),
        ) {
            // CRYUD01.
            (
                43_008,
                0x8003_b80c,
                110_592,
                "8e109d9442d43c46aab8f73e6dbaddbe3e9dc4daddee891a960ea9dd94bbdbcd",
            ) => &[
                (0, 4),
                (16, 68),
                (88, 824),
                (916, 1_907),
                (2_824, 9_584),
                (12_412, 1_464),
                (13_880, 1_485),
                (15_368, 8),
                (15_380, 909),
                (16_292, 8),
                (16_304, 3_459),
                (19_764, 4_104),
                (23_872, 8),
                (23_884, 1_593),
                (25_480, 8),
                (25_492, 1_866),
                (27_360, 5_480),
                (32_844, 2_496),
                (35_344, 4_369),
                (39_716, 8),
                (39_728, 321),
            ],
            // CRYUU01.
            (
                43_008,
                0x8003_b80c,
                110_592,
                "bcf1849fd598d529fd441f4d09561ccc7c143c03babbf3142d24d051ce8b81fe",
            ) => &[
                (0, 4),
                (16, 32),
                (52, 5_504),
                (5_560, 1_382),
                (6_944, 8),
                (6_956, 4_310),
                (11_268, 8),
                (11_280, 931),
                (12_212, 8),
                (12_224, 824),
                (13_052, 1_599),
                (14_652, 2_748),
                (17_404, 1_567),
                (22_788, 6_844),
                (29_636, 4_104),
                (33_744, 8),
                (33_756, 1_684),
                (35_444, 4_245),
                (39_692, 8),
                (39_704, 321),
            ],
            // DRG00_01, DRG00_02, DRG00_03, DRG00_04.
            (
                40_960,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                44_404,
                "1effd0ac6e77259713f70e51130634e848ed75bc66ddb3a5961d9476697d92d1",
            ) => &[(0, 4), (16, 20_508)],
            // DRG01_01, DRG01_02, DRG01_03, DRG01_04.
            (
                40_960,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                38_844,
                "2d8dada5f5112fbc1363e54a2b12ed8d31cd12df53f7456d42015d65e1c47c33",
            ) => &[
                (0, 4),
                (16, 10_984),
                (11_004, 6_035),
                (17_040, 8),
                (17_052, 6_199),
                (23_252, 8),
                (23_264, 5_244),
                (28_512, 4_443),
                (32_956, 2_752),
            ],
            // DRG02_01, DRG02_02, DRG02_03, DRG02_04.
            (
                20_480,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                72_420,
                "46af6dbcedb4eab31ae5d2db9195c3dbc968840c316d08d04b1ca95a1762efb3",
            ) => &[
                (0, 4),
                (16, 12_328),
                (12_348, 13_672),
                (26_024, 6_850),
                (32_876, 8),
                (32_888, 7_098),
                (39_988, 8),
                (40_000, 7_408),
                (47_412, 14_440),
            ],
            // DRG03_01, DRG03_02, DRG03_03, DRG03_04.
            (
                40_960,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                76_072,
                "0d918e6fdd6852c3d184213423c02b2aa46ea0cee36d156153e22b55a546d574",
            ) => &[
                (0, 4),
                (16, 8_232),
                (8_252, 9_576),
                (17_832, 5_761),
                (23_596, 8),
                (23_608, 5_396),
                (29_008, 5_090),
                (34_100, 8),
                (34_112, 5_607),
                (39_720, 8_200),
                (47_924, 8_200),
            ],
            // DRG05_01, DRG05_02, DRG05_03, DRG05_04.
            (
                38_912,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                70_700,
                "4297311b37aa27df957dbacaca840945875fafab5c4fab1238a34252938e6ac7",
            ) => &[
                (0, 4),
                (16, 23_260),
                (23_280, 8),
                (23_292, 8_457),
                (31_752, 8),
                (31_764, 8_063),
                (39_828, 8),
                (39_840, 8_189),
                (48_032, 8),
                (48_044, 8_027),
                (56_072, 5_480),
            ],
            // DRG07_01, DRG07_02, DRG07_03, DRG07_04.
            (
                38_912,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                42_264,
                "c8743de97beea25057b3db3bdf89819fc7e43ce89483fe30d8ea54c297a2338f",
            ) => &[
                (0, 4),
                (16, 4_136),
                (4_156, 5_480),
                (9_640, 5_646),
                (15_288, 8),
                (15_300, 5_253),
                (20_556, 8),
                (20_568, 5_175),
                (25_744, 8),
                (25_756, 4_815),
                (30_572, 5_480),
            ],
            // DRG08_01, DRG08_02, DRG08_03, DRG08_04.
            (
                20_480,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                84_732,
                "ecb3e431738d1e065eb0e90a47c392368be4fa84a08fb90ec928659c40d19b36",
            ) => &[
                (0, 4),
                (16, 12_328),
                (12_348, 8),
                (12_360, 5_700),
                (18_064, 6_533),
                (24_600, 8_200),
                (32_804, 12_296),
                (45_104, 8),
                (45_116, 5_893),
                (51_012, 8),
                (51_024, 13_836),
            ],
            // PAPYD01, PAPYD02, PAPYD03, PAPYD04.
            (
                32_768,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                11_796,
                "feea0916b287895af705b3598c55581a2fdaa3657ce7e0b3efff68a47f1c0338",
            ) => &[(0, 4), (16, 6_872), (6_892, 631), (7_524, 1_376)],
            // PAPYU01, PAPYU02, PAPYU03, PAPYU04.
            (
                32_768,
                0x8007_a80c | 0x8005_f80c | 0x8007_180c | 0x8005_680c,
                17_400,
                "f9d984db05b512d943860a31776800a6678c772bf322f265f4339f0f3704b864",
            ) => &[(0, 4), (16, 15_092)],
            _ => &[],
        }
    } else {
        &[]
    };
    if !replacement_sprite_ranges.is_empty() {
        return Some(
            replacement_sprite_ranges
                .iter()
                .map(|&(offset, length)| NontextRange {
                    offset,
                    length,
                    evidence: "Original US MAGIC131 raw replacements: 38 documented entry-3 archives, image section word, selected image directory words and consumed image headers/packed words/mode-2 tokens. Original GAME phase table 0x801C7B54 selects family-1/2 BPL request 0x80197BC0 and publisher 0x80197C9C -> 0x801C1400 -> 0x801C1254, which installs the documented identifier-selected root at actor+0x4C before the battle overlay request. Original MAGIC131 handlers 0x801EF1C0/0x801EF3D0 request the same BPL family/hint and BATTLE entry-15 form-table u16 indexed by hint; SLUS LBA rows and complete physical disc user bytes bind each archive and replacement destination. This is conditional source-use: actor root, actor+0x24=1, original hint/source/code/tables and noninterfering ordinary helpers must survive battle setup and replacement. A finite actor-0 case supplies the documented identifier at 0x80144F5D, active count 1 and an unset special-BPL flag. Post-load call 0x801EF508 -> 0x8014D6B8 with (actor[8]+4)&255 uses documented selectors below 128, avoids reinitialization and reaches 0x8014DC08; queued images reach 0x8014E6D0 and its original readers. Exact original archive and entry/offset/mode/load/checkword/size/payload identities, successful unskipped loads, loader-ready gates, valid animation records, queue capacity and bounded nonaliasing outputs including final-token overshoot are required. Palette storage, CRYUU01 image slot 8 and its directory word, opaque mode-2 prefixes, tails, programs and selection storage are excluded. Natural root retention, selector production, invocation, runtime residency, transfer, live CLUT, display, rasterized-text absence and safe editing remain unverified.",
                })
                .collect(),
        );
    }
    let ryu_replacement_evidence = "Original US RYUD/U00..03 and RYUD/U10..13 entry 3 conditional MAGIC131 image storage. Original BATTLE entry-15 form 25 or 4 and stable finite hints bind slots 580..595 through the original SLUS LBA table and complete matching disc user bytes. GAME phase-1 BPL family request and phase-3 call to 0x801C1400(1) precede initializer 0x801C1254 publishing a decoded original BPL root at actor +0x4C and byte +0x24=1. The ordinary MAGIC131 BPL flag must remain zero; original request 0x801EF3D0 and ready handler 0x801EF488 reach SLUS 0x8014D6B8 with (actor[8]+4)&255, then 0x8014DC08 and queued reader 0x8014E6D0. Only the image section word, reachable directory words and consumed image headers/packed words/mode-2 tokens qualify. Exact original archive/entry/offset/raw-mode/load/checkword/size/payload identities, retained root/byte24, stable original source/code/tables, ordinary noninterfering helper returns, valid animation records, queue capacity and bounded nonaliasing outputs including final-token overshoot are conditions. Form-4 image slot 13 and its directory word, all palettes, opaque mode-2 prefixes, tails, programs and selections are excluded. No natural state/selector/root retention, invocation, residency, transfer, display, rasterized-text absence or safe-edit claim.";
    let rei_replacement_evidence = "Original US REID/U00..01 entry 3 conditional image and palette storage. GAME original BPL publication selects ordinal 0 at 0x8003B80C with hint 0/id 0, or ordinal 1 at 0x8005F80C with hint 10/id 7, and writes actor byte +0x24=1. Original BATTLE03 producer 0x801D6818 with cursor 0, available actor and actor byte +0x79=4 stores actor 0 in queued mode-0 selector-11 slot +0x74. Original dispatcher 0x801E5824 publishes that pointer at 0x801EB4E0; 0x801E5AF4 and table 0x801D0CD0 reach 0x801E74B8. States 2/3 call request 0x801E75B0 and post-load reader 0x801E7634. The same retained actor halfword +0x2C selects suffix 00/01, and actor byte 8 below 2 selects REID or otherwise REIU. Reader argument (actor[8]+4)&255 and request predicates jointly restrict admitted images. Calls 0x801E7698/0x801E76E4 reach SLUS image selection and palette index 0. Slots 572..575 match the complete original physical archive bytes. Exact original archive/entry/offset/raw-mode/load/checkword/size/payload identities, retained actor root/selector, bounded dispatch states, successful unskipped loads, clear image-skip bit, stable original source/code/tables, ordinary noninterfering helper returns, valid animation records and bounded nonaliasing work/queue/output including final-token overshoot are conditions. Only section words 0/4, reachable image directory words and consumed image storage, palette directory word 0 and its 32 halfwords qualify. REID image slots other than 1, remaining palettes, opaque mode-2 prefixes, tails, programs and selection storage are excluded. No natural state/selector/root retention, invocation, residency, transfer, display, rasterized-text absence or safe-edit claim.";
    let rt_replacement_evidence = "Original US RTD/U00..01 entry 3 conditional image and palette storage. GAME original BPL publication selects ordinal 0 at 0x8003B80C with hint 0/id 0, or ordinal 1 at 0x8005F80C with hint 10/id 7, and writes actor byte +0x24=1. Original BATTLE15 0x800AE108 with action 64 maps byte 77 at 0x800B6550, queues mode 2 and publishes incoming A1 at slot +0x74; conditional A1=GAME actor 0 and selection byte 0x80146374=0 bind request and reader to that actor. Code request 0x800AE1F8 uses row 0x800B6860 slot 336, whose complete physical archive is MAGIC064.EMI. With global flag 0x801462E8 bit 0x800, queued dispatcher 0x801E5824 and 0x800ADFBC call row handler 0x801EEC7C. Bounded outer state 1 and inner states 8/9 reach original MAGIC064 request 0x801EF2A4 and post-load reader 0x801EF3C8. Stable actor halfword +0x2C selects suffix 00/01 and byte 8 below 2 selects RTD or otherwise RTU. After saved work byte 9 equals 0x80 and loader-ready, reader passes actor byte 8 unchanged through 0x8014D8D4 and copies palette index 0. Request predicates restrict RTD to image slot 3; RTU has finite below-128 cases for all six images. Slots 576..579 match complete original physical archives. Exact original archive/entry/offset/raw-mode/load/checkword/size/payload identities, retained root/selector/byte24, changed image selector, clear image-skip bit, successful unskipped loads, stable original source/code/tables, ordinary noninterfering helper returns, valid animation records and bounded nonaliasing work/queue/output including final-token overshoot are conditions. Only section words 0/4, reachable image directory words and consumed image storage, palette directory word 0 and its 32 halfwords qualify. Other RTD images, remaining palettes, opaque mode-2 prefixes, tails, programs and selection storage are excluded. No natural incoming arguments/state/selector/root retention, invocation, residency, transfer, display, rasterized-text absence or safe-edit claim.";
    let (remaining_sprite_ranges, remaining_sprite_evidence): (&[(usize, usize)], &str) =
        if entry.index == 3
            && entry.transfer_mode == 0
            && entry.check_word == 0x10
            && usize::try_from(entry.size).ok() == Some(data.len())
        {
            match (
                entry.offset,
                entry.ram_ptr,
                data.len(),
                sha256_hex(data).as_str(),
            ) {
                // RYUD00, RYUD01, RYUD02, RYUD03.
                (
                    40_960,
                    0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
                    110_592,
                    "c5b66a9d3aa3aa53f0a01ffdd83227ff46656c424318444f26e5a9080920bcf5",
                ) => (
                    &[
                        (0, 4),
                        (16, 5_552),
                        (5_572, 2_055),
                        (7_628, 8),
                        (7_640, 4_033),
                        (11_676, 8),
                        (11_688, 5_699),
                        (17_388, 8),
                        (17_400, 6_847),
                        (24_248, 8),
                        (24_260, 2_565),
                        (26_828, 8),
                        (26_840, 1_378),
                        (28_220, 8),
                        (28_232, 899),
                        (29_132, 8),
                        (29_144, 1_625),
                        (30_772, 8),
                        (30_784, 823),
                        (31_608, 6_844),
                        (38_456, 2_438),
                        (40_896, 8),
                        (40_908, 5_708),
                        (46_620, 5_493),
                        (52_116, 8),
                        (52_128, 499),
                    ],
                    ryu_replacement_evidence,
                ),
                // RYUD10, RYUD11, RYUD12, RYUD13.
                (
                    40_960,
                    0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
                    61_260,
                    "79ab35585a155a4d67210af5b64140f4dfb971e4cd8f97517ac75c51b758b048",
                ) => (
                    &[
                        (0, 4),
                        (16, 52),
                        (72, 5_496),
                        (5_572, 2_055),
                        (7_628, 8),
                        (7_640, 4_033),
                        (11_676, 8),
                        (11_688, 5_699),
                        (17_388, 8),
                        (17_400, 6_847),
                        (24_248, 8),
                        (24_260, 2_565),
                        (26_828, 8),
                        (26_840, 1_378),
                        (28_220, 8),
                        (28_232, 899),
                        (29_132, 8),
                        (29_144, 1_625),
                        (30_772, 8),
                        (30_784, 823),
                        (31_608, 6_844),
                        (38_456, 2_438),
                        (46_608, 8),
                        (46_620, 5_493),
                        (52_116, 8),
                        (52_128, 499),
                    ],
                    ryu_replacement_evidence,
                ),
                // RYUU00, RYUU01, RYUU02, RYUU03.
                (
                    40_960,
                    0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
                    110_592,
                    "fdfe8bd3a62f759adba71e370483e2b6d3c70a0419ed152c3518ad10fd68b8c1",
                ) => (
                    &[
                        (0, 4),
                        (16, 5_552),
                        (5_572, 2_063),
                        (7_636, 8),
                        (7_648, 3_788),
                        (11_440, 6_603),
                        (18_044, 8),
                        (18_056, 6_510),
                        (24_568, 8),
                        (24_580, 2_400),
                        (26_984, 1_371),
                        (28_356, 8),
                        (28_368, 830),
                        (29_200, 8),
                        (29_212, 1_388),
                        (30_604, 6_272),
                        (36_880, 2_020),
                        (38_904, 4_918),
                        (43_824, 8),
                        (43_836, 4_542),
                        (48_380, 8),
                        (48_392, 499),
                    ],
                    ryu_replacement_evidence,
                ),
                // RYUU10, RYUU11, RYUU12, RYUU13.
                (
                    40_960,
                    0x8005_680c | 0x8005_f80c | 0x8007_180c | 0x8007_a80c,
                    57_480,
                    "10dafbb38ae2aa7d798a2b6d02f35b88d89aa833f61015415ad8185a1c0f2f4d",
                ) => (
                    &[
                        (0, 4),
                        (16, 52),
                        (72, 5_496),
                        (5_572, 2_063),
                        (7_636, 8),
                        (7_648, 3_788),
                        (11_440, 6_603),
                        (18_044, 8),
                        (18_056, 6_510),
                        (24_568, 8),
                        (24_580, 2_400),
                        (26_984, 1_371),
                        (28_356, 8),
                        (28_368, 830),
                        (29_200, 8),
                        (29_212, 1_388),
                        (30_604, 6_272),
                        (36_880, 2_012),
                        (43_824, 8),
                        (43_836, 4_542),
                        (48_380, 8),
                        (48_392, 499),
                    ],
                    ryu_replacement_evidence,
                ),
                // REID00, REID01.
                (
                    38_912,
                    0x8003_b80c | 0x8005_f80c,
                    110_592,
                    "30da9d48edceff573fa4d61982f76f79faa0a0f0b10095d5aee55428df03b271",
                ) => (
                    &[(0, 8), (20, 4), (13_732, 4_104), (69_700, 4), (69_708, 64)],
                    rei_replacement_evidence,
                ),
                // REIU00, REIU01.
                (
                    38_912,
                    0x8003_b80c | 0x8005_f80c,
                    110_592,
                    "674e66c3fb9779917786c30c579456ebeace8419c19a9a11907037a0b96a7e40",
                ) => (
                    &[
                        (0, 8),
                        (16, 4_156),
                        (4_176, 13_672),
                        (17_852, 5_205),
                        (23_060, 8),
                        (23_072, 5_574),
                        (28_648, 10_940),
                        (39_592, 971),
                        (40_564, 8),
                        (40_576, 7_598),
                        (48_176, 8),
                        (48_188, 4_986),
                        (53_176, 8),
                        (53_188, 3_549),
                        (56_740, 8),
                        (56_752, 5_350),
                        (62_104, 8),
                        (62_116, 6_111),
                        (68_228, 8),
                        (68_240, 4_513),
                        (72_756, 8),
                        (72_768, 504),
                        (73_276, 64),
                    ],
                    rei_replacement_evidence,
                ),
                // RTD00, RTD01.
                (
                    26_624,
                    0x8003_b80c | 0x8005_f80c,
                    110_592,
                    "436640506a37ffea2605d20e1b1f63983d8cd019399279ed3f62e5fb17be9e18",
                ) => (
                    &[
                        (0, 8),
                        (28, 4),
                        (13_660, 8),
                        (13_672, 7_143),
                        (37_984, 4),
                        (37_992, 64),
                    ],
                    rt_replacement_evidence,
                ),
                // RTU00, RTU01.
                (
                    26_624,
                    0x8003_b80c | 0x8005_f80c,
                    110_592,
                    "834705a61dc821f942d47bc68d3a9a08c5c924f1266527aec14173ac9aa532e1",
                ) => (
                    &[
                        (0, 8),
                        (16, 10_976),
                        (10_996, 2_718),
                        (13_716, 8),
                        (13_728, 6_667),
                        (20_396, 8),
                        (20_408, 8_609),
                        (29_020, 8),
                        (29_032, 8_536),
                        (37_572, 64),
                    ],
                    rt_replacement_evidence,
                ),
                _ => (&[], ""),
            }
        } else {
            (&[], "")
        };
    if !remaining_sprite_ranges.is_empty() {
        return Some(
            remaining_sprite_ranges
                .iter()
                .map(|&(offset, length)| NontextRange {
                    offset,
                    length,
                    evidence: remaining_sprite_evidence,
                })
                .collect(),
        );
    }
    if entry.transfer_mode == 0
        && entry.ram_ptr == 0x801e_ec00
        && entry.check_word == 0x141
        && entry.size == 5_268
        && matches!((entry.index, entry.offset), (8, 231_424) | (9, 479_232))
        && data.len() == 5_268
        && sha256_hex(data) == "6e7b93d4fd75586413b5ab3a6785d7b2492d8d1b566a3a6d79941bb656131a4d"
    {
        let images = "original US SHOP entry 8 or START entry 9 save-icon palette and three packed 4-bit frames; complete original producer 0x801EF27C..0x801EF96C copies 32 bytes at 0x801EFE68 to SC header +0x60 and, in mode 0x13, 128 bytes each at 0x801EFE94/0x801EFF54/0x801F0014 to +0x80/+0x100/+0x180; original SHOP call 0x801D6A08 supplies mode 0x13 in its delay slot; exact original archive, entry geometry, raw load, checkword, size and payload identities; stable source/code, bounded valid title, ordinary returning helper, intact frame and nonaliased work are conditions; producer gaps and surrounding bytes excluded; no actual invocation, active supplier, BIOS animation, display or safe-edit claim";
        return Some(
            [(0x1268, 32), (0x1294, 128), (0x1354, 128), (0x1414, 128)]
                .into_iter()
                .map(|(offset, length)| NontextRange {
                    offset,
                    length,
                    evidence: images,
                })
                .collect(),
        );
    }
    if let Some(length) = vram_tile_prefix_bytes(entry, data)
        && length > 0
        && length < data.len()
    {
        return Some(vec![NontextRange {
            offset: 0,
            length,
            evidence: "original US partial mode-3 VRAM submission prefix; exact whole-archive identity, TOC size/mode/checkword and complete nonwrapping tile rows; original SLUS 0x80162698..0x80162790 constructs ring-state-3 coordinates and 0x80163378..0x80163418 conditionally submits each in-bounds 0x800-byte chunk as 32-by-32 words through LoadImage; only the complete prefix inside 1024-by-512-word VRAM qualifies; clipped tail excluded; no completed loading, hardware transfer, CLUT, display or safe-edit claim",
        }]);
    }
    if entry.transfer_mode == 0
        && entry.ram_ptr >= 0x8003_3800
        && entry.ram_ptr.is_multiple_of(32)
        && !data.is_empty()
        && data.len().is_multiple_of(32)
        && entry.size as usize == data.len()
        && entry
            .ram_ptr
            .checked_add(entry.size)
            .is_some_and(|end| end <= 0x8003_7000)
        && data[..4] == entry.check_word.to_le_bytes()
    {
        return Some(vec![NontextRange {
            offset: 0,
            length: data.len(),
            evidence: "original US type-0 palette words; exact whole-archive identity, TOC size/destination/checkword and 32-byte-aligned span within 0x80033800..0x80037000; original SLUS 0x80162B08 and 0x80162C14 load the declared span, 0x8014E284 stages its rows, and 0x8014E22C conditionally uploads 256-by-32 image words through LoadImage; no completed loading, active CLUT, sampling, display or safe-edit claim",
        }]);
    }
    if entry.transfer_mode == 1 {
        return compression_metadata_ranges(
            entry,
            data,
            model_decoded_nontext_fields(entry, data),
            additional_model_decoded_nontext_fields(entry, data),
            model_program_decoded_nontext_fields(entry, data),
        );
    }
    if entry.transfer_mode == 0 && entry.ram_ptr == 0x8010_4000 {
        return area_surface_field_ranges(data, entry.check_word);
    }
    if entry.transfer_mode == 0 && entry.ram_ptr == 0x800d_3800 {
        return resident_animation_payload_ranges(entry, data);
    }
    match (
        entry.index,
        entry.offset,
        entry.transfer_mode,
        entry.ram_ptr,
        data.len(),
    ) {
        (_, _, 0, 0x800f_0800, 3_692)
            if entry.check_word == 12
                && matches!(
                    (entry.index, entry.offset),
                    (2, 153_600)
                        | (5, 417_792)
                        | (5, 419_840)
                        | (5, 425_984)
                        | (5, 430_080)
                        | (5, 442_368)
                )
                && sha256_hex(data)
                    == "2cd03f3e11d86c2de1fdcc3b3c996eece1c02c45957f71a94362ac95e8959934" =>
        {
            Some(
                [(4, 4), (1_798, 6), (2_211, 188), (2_401, 171), (2_573, 46)]
                    .into_iter()
                    .map(|(offset, length)| NontextRange {
                        offset,
                        length,
                        evidence: BATTLE_RESIDENT_FIELD_EVIDENCE,
                    })
                    .collect(),
            )
        }
        (0, 2048, 0, 0x8003_b800, 331_788)
            if sha256_hex(data)
                == "3633597f03d7f2129ff7d7fc1ad0fe7c2e6a0affb2562e789c4a0b97b5d14b3e" =>
        {
            let mut ranges: Vec<_> = BPLU349_GRAPHICS_RANGES
            .iter()
            .map(|&(offset, length)| NontextRange {
                offset,
                length,
                evidence: "original US BPLU349 image headers, packed words, mode-2 tokens and 32-halfword palette records; consumers 0x8014E6D0, 0x8014E7F8, 0x8014E8FC and 0x80166C48; opaque prefixes excluded; exact source identity; no rasterized-text absence or runtime claim",
            })
            .chain(bpl_selected_sprite_fields(data, 2)?.into_iter().map(|(start, end)| NontextRange {
                offset: start,
                length: end - start,
                evidence: BPLU_SELECTED_SPRITE_EVIDENCE,
            }))
            .collect();
            let mut additional = vec![false; data.len()];
            for (start, end) in bpl_flag_sprite_fields(data, 2).unwrap_or_default() {
                additional.get_mut(start..end)?.fill(true);
            }
            for range in &ranges {
                additional
                    .get_mut(range.offset..range.offset.checked_add(range.length)?)?
                    .fill(false);
            }
            let mut offset = 0;
            while offset < additional.len() {
                if !additional[offset] {
                    offset += 1;
                    continue;
                }
                let start = offset;
                while offset < additional.len() && additional[offset] {
                    offset += 1;
                }
                ranges.push(NontextRange {
                    offset: start,
                    length: offset - start,
                    evidence: BPL_FLAG_SPRITE_EVIDENCE,
                });
            }
            Some(ranges)
        }
        (_, _, 0, 0x800e_3800, length) if length >= 4 && length == 4 + usize::from(data[0]) * 8 => {
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
fn vram_tile_prefix_bytes(entry: Entry, data: &[u8]) -> Option<usize> {
    if entry.transfer_mode != 3
        || data.is_empty()
        || !data.len().is_multiple_of(0x800)
        || data.len() != entry.size as usize
        || data[..4] != entry.check_word.to_le_bytes()
    {
        return None;
    }
    let column = ((entry.ram_ptr >> 24) & 0x3f) as usize;
    let row = ((entry.ram_ptr >> 16) & 0x1f) as usize;
    let width = ((entry.ram_ptr >> 8) & 0x3f) as usize;
    let sectors = data.len() / 0x800;
    if width == 0 || !sectors.is_multiple_of(width) || column + width > 32 {
        return None;
    }
    let rows = sectors / width;
    if row + rows > 64 {
        return None;
    }
    Some(16usize.saturating_sub(row).min(rows) * width * 0x800)
}

/// Reviewed payload fields and instruction bodies; no display claim.
fn uncovered_nontext_ranges(
    size: usize,
    ranges: Vec<NontextRange>,
    previous: &[NontextRange],
) -> Option<Vec<NontextRange>> {
    let mut covered = vec![false; size];
    for range in previous {
        covered
            .get_mut(range.offset..range.offset.checked_add(range.length)?)?
            .fill(true);
    }
    let mut additions = Vec::new();
    for range in ranges {
        let end = range.offset.checked_add(range.length)?;
        let known = covered.get(range.offset..end)?;
        let mut at = 0;
        while at < known.len() {
            if known[at] {
                at += 1;
                continue;
            }
            let start = at;
            while at < known.len() && !known[at] {
                at += 1;
            }
            additions.push(NontextRange {
                offset: range.offset + start,
                length: at - start,
                evidence: range.evidence,
            });
        }
    }
    Some(additions)
}

fn payload_ranges(entry: Entry, data: &[u8]) -> Vec<NontextRange> {
    let mut ranges = reviewed_texture_ranges(entry, data).unwrap_or_default();
    if let Some(copies) = stored_image_copy_ranges(entry, data) {
        ranges.extend(copies);
    }
    if let Some(copies) = stored_sprite_record_copy_ranges(entry, data) {
        ranges.extend(copies);
    }
    if let Some(fields) = raw_sprite_program_ranges(entry, data, &ranges) {
        ranges.extend(fields);
    }
    if let Some(copies) = stored_resident_record_copy_ranges(entry, data, &ranges) {
        ranges.extend(copies);
    }
    if let Some(copies) = stored_raw_sprite_record_copy_ranges(entry, data, &ranges) {
        ranges.extend(copies);
    }
    ranges
}
