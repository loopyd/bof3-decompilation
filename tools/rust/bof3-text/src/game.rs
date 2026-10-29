//! Reviewed mutable text fields in the original US GAME archive.

use crate::digest::sha256_hex;

pub(crate) const LOAD: u32 = 0x80195800;
pub(crate) const PAYLOAD_SIZE: usize = 229720;
pub(crate) const PAYLOAD_SHA256: &str =
    "54acc54f26dfa2d81b4d4124e884f6403ff68eedb29ae89ba6cd720da2209d6c";
const ARCHIVE_SHA256: &str = "1971159609c78c33ec99eb500f4e6dbe0cff57af2ae74f37023aeaf45fa7efe9";
pub(crate) const NAME_TABLES: &[(&str, usize, usize, usize)] = &[
    ("items", 0x33964, 92, 0x12),
    ("key-items", 0x33fdc, 16, 0x10),
    ("weapons", 0x340dc, 83, 0x18),
    ("armor", 0x348a4, 68, 0x16),
    ("accessories", 0x34e7c, 52, 0x14),
    ("abilities", 0x3570c, 228, 0x14),
];
const CATEGORY_FIELDS: &[(usize, usize)] = &[
    (0x32434, 5),
    (0x3243c, 7),
    (0x32444, 6),
    (0x3244c, 7),
    (0x32454, 6),
];

pub(crate) fn mask_text(payload: &mut [u8]) {
    for &(_, start, count, stride) in NAME_TABLES {
        for index in 0..count {
            let at = start - 0x800 + index * stride;
            payload[at..at + 12].fill(0);
        }
    }
    for &(at, capacity) in CATEGORY_FIELDS {
        payload[at..at + capacity].fill(0);
    }
}

pub(crate) fn supports_archive(data: &[u8]) -> bool {
    let Ok(entries) = crate::lexagraph::entries(data) else {
        return false;
    };
    if !entries.first().is_some_and(|(entry, payload)| {
        entry.offset == 0x800 && entry.ram_ptr == LOAD && payload.len() == PAYLOAD_SIZE
    }) {
        return false;
    }
    let mut masked = data.to_vec();
    mask_text(&mut masked[0x800..0x800 + PAYLOAD_SIZE]);
    sha256_hex(&masked) == ARCHIVE_SHA256
}
