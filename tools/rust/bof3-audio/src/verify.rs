//! Evidence categories are independent: a valid container does not prove playback.

use crate::archive::MediaImage;
use crate::{digest::sha256_hex, machine::profile::Profile};
use serde::Serialize;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Status {
    Pass,
    Fail,
    MissingEvidence,
}

impl std::fmt::Display for Status {
    fn fmt(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        formatter.write_str(match self {
            Self::Pass => "pass",
            Self::Fail => "fail",
            Self::MissingEvidence => "missing_evidence",
        })
    }
}

#[derive(Debug, Serialize)]
pub struct Check {
    pub status: Status,
    pub reason: &'static str,
}

#[derive(Debug, Serialize)]
pub struct Checks {
    pub structure: Check,
    pub identity: Check,
    pub completeness: Check,
    pub whole_file: Check,
    pub translation: Check,
    pub encoding: Check,
    pub rendering: Check,
}

impl Checks {
    pub fn categories(&self) -> [(&'static str, &Check); 7] {
        [
            ("structure", &self.structure),
            ("identity", &self.identity),
            ("completeness", &self.completeness),
            ("whole_file", &self.whole_file),
            ("translation", &self.translation),
            ("encoding", &self.encoding),
            ("rendering", &self.rendering),
        ]
    }

    pub fn has_failures(&self) -> bool {
        self.categories()
            .iter()
            .any(|(_, check)| check.status == Status::Fail)
    }
}

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
    pub checks: Checks,
    pub byte_equality: Option<ByteComparison>,
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
        let byte_equality = original.map(|bytes| compare_bytes(bytes, image.bytes()));
        let completeness = match &media_completeness {
            Some(reference) if reference.is_truncated() => Check {
                status: Status::Fail,
                reason: "Whole-file identity matches a known truncated disc extent.",
            },
            Some(_) => Check {
                status: Status::Pass,
                reason: "Whole-file identity matches a complete original disc extent.",
            },
            None => Check {
                status: Status::MissingEvidence,
                reason: "No independently established whole-file completeness reference matches this media.",
            },
        };
        let whole_file = match &byte_equality {
            Some(comparison) if comparison.equal => Check {
                status: Status::Pass,
                reason: "Every byte matches the supplied comparison file, including length.",
            },
            Some(_) => Check {
                status: Status::Fail,
                reason: "Content or length differs from the supplied comparison file.",
            },
            None => Check {
                status: Status::MissingEvidence,
                reason: "No comparison file was supplied with --against.",
            },
        };
        Self {
            schema: "bof3.audio-verification/v2",
            source,
            container,
            bytes: image.bytes().len(),
            sha256: hash,
            entries_or_sectors: count,
            checks: Checks {
                structure: Check {
                    status: Status::Pass,
                    reason: "The container parser accepted its layout; audio payload semantics are not checked.",
                },
                identity: Check {
                    status: Status::MissingEvidence,
                    reason: "Qualified asset bindings are not verified; an executable profile alone does not establish them.",
                },
                completeness,
                whole_file,
                translation: Check {
                    status: Status::MissingEvidence,
                    reason: "No source-to-MIDI or SoundFont semantic comparison was performed.",
                },
                encoding: Check {
                    status: Status::MissingEvidence,
                    reason: "No independent decoded-sample or codec quality comparison was performed.",
                },
                rendering: Check {
                    status: Status::MissingEvidence,
                    reason: "No independent runtime trace or pre-lossy PCM comparison was performed.",
                },
            },
            byte_equality,
            runtime_profile: None,
            media_completeness,
        }
    }
}
