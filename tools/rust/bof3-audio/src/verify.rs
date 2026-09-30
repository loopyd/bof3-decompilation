//! Evidence categories are independent: a valid container does not prove playback.

use crate::archive::MediaImage;
use crate::{digest::sha256_hex, machine::profile::Profile};
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct ByteComparison {
    pub equal: bool,
    pub expected_bytes: usize,
    pub actual_bytes: usize,
    pub first_difference: Option<usize>,
}

pub fn compare_bytes(expected: &[u8], actual: &[u8]) -> ByteComparison {
    let first_difference = expected
        .iter()
        .zip(actual)
        .position(|(a, b)| a != b)
        .or_else(|| (expected.len() != actual.len()).then_some(expected.len().min(actual.len())));
    ByteComparison {
        equal: first_difference.is_none(),
        expected_bytes: expected.len(),
        actual_bytes: actual.len(),
        first_difference,
    }
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub source: String,
    pub container: &'static str,
    pub bytes: usize,
    pub sha256: String,
    pub entries_or_sectors: usize,
    pub structural_validity: &'static str,
    pub identity_consistency: &'static str,
    pub byte_equality: Option<ByteComparison>,
    pub translation: &'static str,
    pub sample_encoding: &'static str,
    pub rendering: &'static str,
    pub runtime_profile: Option<Profile>,
    pub media_completeness: Option<crate::xa::reference::Completeness>,
}

impl Report {
    pub fn inspect(source: String, image: &MediaImage, original: Option<&[u8]>) -> Self {
        let (container, count) = match image {
            MediaImage::Emi(archive) => ("emi", archive.entries().len()),
            MediaImage::Xa(stream) => ("xa", stream.sectors().len()),
        };
        let hash = sha256_hex(image.bytes());
        let media_completeness = if container == "xa" {
            crate::xa::reference::identify(&hash)
        } else {
            None
        };
        Self {
            schema: "bof3.audio-verification/v1",
            source,
            container,
            bytes: image.bytes().len(),
            sha256: hash,
            entries_or_sectors: count,
            structural_validity: "container_pass; audio_payloads_not_checked",
            identity_consistency: "not_checked",
            byte_equality: original.map(|bytes| compare_bytes(bytes, image.bytes())),
            translation: "not_checked",
            sample_encoding: "not_checked",
            rendering: "not_checked",
            runtime_profile: None,
            media_completeness,
        }
    }
}
