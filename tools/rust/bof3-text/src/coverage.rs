//! Deterministic accounting of an extracted disc tree, including unresolved data.

use std::collections::BTreeMap;
use std::fs;
use std::path::{Path, PathBuf};

use serde::Serialize;

use crate::digest::sha256_hex;
use crate::filters::KnownPayload;
use crate::models::{Error, Result, TextClass};
use crate::original::US_EXECUTABLE_SHA256;

const US_CDDA_WAV_SHA256: &str = "8e4a881152ec055823afd1c6275220c960cba2a57391cd920a37e6fb2e99a11b";
const US_CDDA_WAV_SIZE: usize = 37_044_044;

#[derive(Serialize)]
pub struct Report {
    schema: &'static str,
    build: &'static str,
    regional_support: BTreeMap<&'static str, &'static str>,
    policy_sha256: String,
    disc_membership: DiscMembership,
    pub complete: bool,
    unresolved_ranges: usize,
    verified_text_entries: usize,
    blockers: Vec<&'static str>,
    files: Vec<FileRecord>,
}

#[derive(Serialize)]
struct FileRecord {
    path: String,
    size: usize,
    sha256: String,
    status: String,
    ranges: Vec<RangeRecord>,
    #[serde(skip_serializing_if = "Option::is_none")]
    derived_from: Option<String>,
}

#[derive(Serialize)]
struct DiscMembership {
    qualification_sha256: String,
    release: String,
    project_sha256: String,
    original_files_match: bool,
    expected_files: usize,
    matched_files: usize,
    expected_emi_entries: usize,
    missing_files: Vec<String>,
    changed_files: Vec<String>,
    additional_files: usize,
}

fn disc_membership(records: &BTreeMap<String, FileRecord>) -> Result<DiscMembership> {
    let original = crate::original::disc()?;
    let mut missing_files = Vec::new();
    let mut changed_files = Vec::new();
    let mut matched_files = 0;
    for file in &original.files {
        match records.get(&file.path) {
            None => missing_files.push(file.path.clone()),
            Some(record) if record.size == file.size && record.sha256 == file.sha256 => {
                matched_files += 1;
            }
            Some(_) => changed_files.push(file.path.clone()),
        }
    }
    let present_files = original.files.len() - missing_files.len();
    Ok(DiscMembership {
        qualification_sha256: sha256_hex(crate::original::DISC.as_bytes()),
        release: original.release.clone(),
        project_sha256: original.project_sha256.clone(),
        original_files_match: missing_files.is_empty() && changed_files.is_empty(),
        expected_files: original.files.len(),
        matched_files,
        expected_emi_entries: original
            .files
            .iter()
            .filter_map(|file| file.emi_entries)
            .sum(),
        missing_files,
        changed_files,
        additional_files: records.len() - present_files,
    })
}

#[derive(Serialize)]
struct RangeRecord {
    offset: usize,
    length: usize,
    #[serde(skip_serializing_if = "Option::is_none")]
    entry: Option<usize>,
    status: &'static str,
    evidence: String,
}

fn read(path: &Path) -> Result<Vec<u8>> {
    fs::read(path).map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))
}

/// Enumerate regular inputs without following symlinks or silently skipping unreadable data.
pub fn files_under(root: &Path) -> Result<Vec<PathBuf>> {
    if !fs::symlink_metadata(root)?.is_dir() {
        return Err(Error::Invalid("corpus root must be a directory".into()));
    }
    let mut files = Vec::new();
    let mut pending = vec![root.to_path_buf()];
    while let Some(directory) = pending.pop() {
        for entry in fs::read_dir(&directory)? {
            let entry = entry?;
            let kind = entry.file_type()?;
            if kind.is_dir() {
                pending.push(entry.path());
            } else if kind.is_file() {
                files.push(entry.path());
            } else {
                return Err(Error::Invalid(format!(
                    "{}: symlinks and special files are not corpus inputs",
                    entry.path().display()
                )));
            }
        }
    }
    files.sort();
    Ok(files)
}

pub(crate) fn relative(root: &Path, path: &Path) -> Result<String> {
    let components: Result<Vec<_>> = path
        .strip_prefix(root)
        .unwrap()
        .components()
        .map(|component| {
            component.as_os_str().to_str().ok_or_else(|| {
                Error::Invalid(format!("{}: corpus paths must be UTF-8", path.display()))
            })
        })
        .collect();
    Ok(components?.join("/"))
}

fn range(offset: usize, length: usize, status: &'static str, evidence: String) -> RangeRecord {
    RangeRecord {
        offset,
        length,
        entry: None,
        status,
        evidence,
    }
}

fn padding(data: &[u8], start: usize, end: usize) -> Option<RangeRecord> {
    (end > start).then(|| {
        let zero = data[start..end].iter().all(|byte| *byte == 0);
        range(
            start,
            end - start,
            if zero { "known-nontext" } else { "unresolved" },
            if zero {
                "zero-filled container padding"
            } else {
                "nonzero container gap; consumer unverified"
            }
            .into(),
        )
    })
}

/// Report every file and payload. Structural recognition never proves unknown bytes are not text.
pub fn report(root: &Path, policy: &Path) -> Result<Report> {
    let inventory = KnownPayload::load(policy)?;
    let policy_sha256 = sha256_hex(&read(policy)?);
    let inputs = files_under(root)?;
    let executable = root.join("SLUS_004.22");
    let xa_sources = if inputs.contains(&executable) {
        crate::original::xa_sources(&read(&executable)?)?
    } else {
        Vec::new()
    };
    let mut records = BTreeMap::new();
    let mut derived: BTreeMap<String, (String, String)> = BTreeMap::new();
    let mut build = "unrecognized";
    let mut text_count = 0;
    for path in &inputs {
        let name = relative(root, path)?;
        let bytes = read(path)?;
        let digest = sha256_hex(&bytes);
        if name == "SLUS_004.22" && digest == US_EXECUTABLE_SHA256 {
            build = "US SLUS-00422";
        }
        let mut record = FileRecord {
            path: name.clone(),
            size: bytes.len(),
            sha256: digest,
            status: "unresolved".into(),
            ranges: Vec::new(),
            derived_from: None,
        };
        let names = crate::records::supports(&bytes)
            .then(|| crate::records::extract(path, &bytes).ok())
            .flatten()
            .map(|document| (0, document.names));
        if bytes.len() == US_CDDA_WAV_SIZE && record.sha256 == US_CDDA_WAV_SHA256 {
            record.status = "verified-audio-export".into();
            record.ranges.push(range(
                0,
                bytes.len(),
                "known-nontext",
                "original US CDDA track 2 PCM WAV export; exact whole-file identity; source writer tools/rust/bof3-disk/src/cue.rs".into(),
            ));
        } else if let Some(stream) = xa_sources.iter().find(|stream| {
            stream.file.path == name
                && stream.file.size == bytes.len()
                && stream.file.sha256 == record.sha256
        }) {
            record.status = "partially-verified-xa".into();
            let mut cursor = 0;
            for span in stream.nontext_ranges(&bytes)? {
                let start = span.offset;
                if start > cursor {
                    record.ranges.push(range(
                        cursor,
                        start - cursor,
                        "unresolved",
                        "XA bytes outside reviewed metadata, audio, empty payloads and coded video prefixes".into(),
                    ));
                }
                record.ranges.push(range(
                    start,
                    span.length,
                    "known-nontext",
                    span.evidence.into(),
                ));
                cursor = start + span.length;
            }
            if cursor < bytes.len() {
                record.ranges.push(range(
                    cursor,
                    bytes.len() - cursor,
                    "unresolved",
                    "remaining XA bytes outside reviewed ranges".into(),
                ));
            }
        } else if path.extension().is_some_and(|extension| extension == "EMI") {
            match crate::lexagraph::entries(&bytes) {
                Err(error) => {
                    record
                        .ranges
                        .push(range(0, bytes.len(), "unresolved", error.to_string()))
                }
                Ok(entries) => {
                    let declared = u32::from_le_bytes(bytes[..4].try_into().unwrap()) as usize;
                    if declared != entries.len() {
                        record.ranges.push(range(
                            0,
                            bytes.len(),
                            "unresolved",
                            "truncated EMI payload table".into(),
                        ));
                    } else {
                        let header_end = 16 + entries.len() * 16;
                        record.ranges.push(range(
                            0,
                            header_end,
                            "known-nontext",
                            "EMI header and TOC".into(),
                        ));
                        let mut cursor = header_end;
                        let original_us = crate::original::archive(&bytes)?;
                        let mut preceding = Vec::new();
                        let mut expected_metadata = Vec::new();
                        for (entry, payload) in &entries {
                            if let Some(gap) = padding(&bytes, cursor, entry.offset) {
                                record.ranges.push(gap);
                            }
                            let (status, evidence) = if let Some(class) =
                                TextClass::from_load_argument(entry.ram_ptr)
                            {
                                match crate::extract::parse_section(payload) {
                                    Ok(section) => {
                                        text_count += 1;
                                        (
                                            "known-text",
                                            format!(
                                                "{} pointer banks; {} rows; load argument {}",
                                                section.banks.len(),
                                                section.rows(),
                                                class.name()
                                            ),
                                        )
                                    }
                                    Err(error) => (
                                        "unresolved",
                                        format!("declared text bank failed parsing: {error}"),
                                    ),
                                }
                            } else if let Some(family) = inventory.identify(payload).or_else(|| {
                                inventory.identify_context(&crate::models::PreFilterInput {
                                    archive: &name,
                                    entry: entry.index,
                                    payload,
                                    class: None,
                                    toc: Some(*entry),
                                    original_us,
                                    preceding: &preceding,
                                })
                            }) {
                                (
                                    "known-nontext",
                                    format!(
                                        "{}: {}; records: {}",
                                        family.family,
                                        family.check.kind,
                                        family.records.join(", ")
                                    ),
                                )
                            } else {
                                (
                                    "unresolved",
                                    format!(
                                        "consumer unverified; load argument 0x{:08x}",
                                        entry.ram_ptr
                                    ),
                                )
                            };
                            let inline = crate::inline::supports(&bytes, *entry, payload)
                                .then(|| crate::inline::extract(path, &bytes, *entry, payload).ok())
                                .flatten();
                            let monster = crate::monsters::supports(*entry, payload)
                                .then(|| {
                                    crate::monsters::extract(path, &bytes, *entry, payload).ok()
                                })
                                .flatten();
                            let entry_names = monster
                                .as_ref()
                                .map(|document| &document.names)
                                .or_else(|| {
                                    names
                                        .as_ref()
                                        .filter(|(index, _)| entry.index == *index)
                                        .map(|(_, names)| names)
                                });
                            let inline_layout = inline.is_some();
                            let mixed = inline_layout && entry_names.is_some();
                            let mut allocations = Vec::new();
                            if let Some(document) = inline {
                                for text in document.strings {
                                    for reference in &text.references {
                                        let relative = reference
                                            .checked_sub(entry.ram_ptr)
                                            .ok_or_else(|| {
                                                Error::Invalid(
                                                    "inline pointer precedes payload".into(),
                                                )
                                            })?
                                            as usize;
                                        let reference_end =
                                            relative.checked_add(4).ok_or_else(|| {
                                                Error::Invalid(
                                                    "inline pointer offset overflow".into(),
                                                )
                                            })?;
                                        let word = payload
                                            .get(relative..reference_end)
                                            .ok_or_else(|| {
                                                Error::Invalid(
                                                    "inline pointer outside payload".into(),
                                                )
                                            })?;
                                        if u32::from_le_bytes(word.try_into().unwrap())
                                            != text.address
                                        {
                                            return Err(Error::Invalid(
                                                "inline pointer target mismatch".into(),
                                            ));
                                        }
                                        let offset = entry
                                            .offset
                                            .checked_add(relative)
                                            .ok_or_else(|| {
                                                Error::Invalid(
                                                    "inline pointer file offset overflow".into(),
                                                )
                                            })?;
                                        allocations.push(RangeRecord {
                                            offset, length: 4,
                                            entry: Some(entry.index), status: "known-nontext",
                                            evidence: format!("{} source-bound pointer word {reference:#x} targets inline allocation {:#x}", document.layout, text.address),
                                        });
                                    }
                                    allocations.push(RangeRecord {
                                        offset: text.file_offset, length: text.capacity,
                                        entry: Some(entry.index), status: "known-text",
                                        evidence: format!("{} allocation {:#x}; consumers {}; renderers {}; codec {}; runtime acceptance pending", document.layout, text.address, document.consumers.iter().map(|address| format!("{address:#x}")).collect::<Vec<_>>().join(", "), if text.renderers.is_empty() { "unverified".into() } else { text.renderers.iter().map(|address| format!("{address:#x}")).collect::<Vec<_>>().join(", ") }, text.codec.as_str()),
                                    });
                                }
                            }
                            if let Some(names) = entry_names {
                                for row in names {
                                    let consumer = if row.family == "monsters" {
                                        "battle template copy 0x800ac218; lookup 0x800ac704; eight-byte name copy 0x801dea18"
                                    } else if row.family == "characters" {
                                        "START/STATUS initializer 0x801d15c0; SISYOU five-byte copy 0x801d39cc and draw call 0x801d1848"
                                    } else if row.family == "abilities" {
                                        "SISYOU entry 0 twelve-byte copy 0x801d3d64"
                                    } else {
                                        "resolver 0x80165d48; COMMU00 entry 0 twelve-byte copy 0x801f1a30"
                                    };
                                    allocations.push(RangeRecord {
                                        offset: row.file_offset,
                                        length: row.capacity,
                                        entry: Some(entry.index),
                                        status: "known-text",
                                        evidence: format!(
                                            "{} record {}; {consumer}; runtime acceptance pending",
                                            row.family, row.index
                                        ),
                                    });
                                }
                            }
                            if !allocations.is_empty() {
                                text_count += 1;
                                allocations.sort_by_key(|text| text.offset);
                                let gap = if mixed {
                                    "bytes outside the supported text allocations"
                                } else if inline_layout {
                                    "bytes outside the supported inline allocations"
                                } else {
                                    "bytes outside the supported name record fields"
                                };
                                let tail = if mixed {
                                    "remaining text entry consumers unverified"
                                } else if inline_layout {
                                    "remaining inline payload consumers unverified"
                                } else {
                                    "remaining record entry consumers unverified"
                                };
                                let mut at = entry.offset;
                                let end =
                                    entry.offset.checked_add(payload.len()).ok_or_else(|| {
                                        Error::Invalid("inline payload end overflow".into())
                                    })?;
                                for allocation in allocations {
                                    let allocation_end = allocation
                                        .offset
                                        .checked_add(allocation.length)
                                        .ok_or_else(|| {
                                            Error::Invalid("allocation end overflow".into())
                                        })?;
                                    if allocation.offset < at || allocation_end > end {
                                        return Err(Error::Invalid(
                                            "overlapping or out-of-entry supported allocations"
                                                .into(),
                                        ));
                                    }
                                    if allocation.offset > at {
                                        record.ranges.push(range(
                                            at,
                                            allocation.offset - at,
                                            "unresolved",
                                            gap.into(),
                                        ));
                                    }
                                    at = allocation_end;
                                    record.ranges.push(allocation);
                                }
                                if at < end {
                                    record.ranges.push(range(
                                        at,
                                        end - at,
                                        "unresolved",
                                        tail.into(),
                                    ));
                                }
                            } else if let Some(spans) = crate::original::graphics_storage_ranges(
                                *entry,
                                payload,
                                original_us,
                            ) {
                                let mut at = 0;
                                for span in spans {
                                    if span.offset > at {
                                        record.ranges.push(range(
                                            entry.offset + at,
                                            span.offset - at,
                                            "unresolved",
                                            "sprite-bank bytes outside reviewed graphics storage"
                                                .into(),
                                        ));
                                    }
                                    record.ranges.push(RangeRecord {
                                        offset: entry.offset + span.offset,
                                        length: span.length,
                                        entry: Some(entry.index),
                                        status: "known-nontext",
                                        evidence: span.evidence.into(),
                                    });
                                    at = span.offset + span.length;
                                }
                                if at < payload.len() {
                                    record.ranges.push(range(
                                        entry.offset + at,
                                        payload.len() - at,
                                        "unresolved",
                                        "remaining sprite-bank bytes outside reviewed graphics storage".into(),
                                    ));
                                }
                            } else {
                                record.ranges.push(RangeRecord {
                                    offset: entry.offset,
                                    length: payload.len(),
                                    entry: Some(entry.index),
                                    status,
                                    evidence,
                                });
                            }
                            let sidecar =
                                format!("{}/{}.bin", name.trim_end_matches(".EMI"), entry.index);
                            derived.insert(
                                sidecar,
                                (sha256_hex(payload), format!("{name}#{}", entry.index)),
                            );
                            expected_metadata.push((entry.index, entry.ram_ptr, entry.size));
                            cursor = entry.offset + payload.len();
                            preceding.push((*entry, *payload));
                        }
                        if let Some(gap) = padding(&bytes, cursor, bytes.len()) {
                            record.ranges.push(gap);
                        }
                        let metadata_name = format!("{}/emi.json", name.trim_end_matches(".EMI"));
                        let metadata_path = root.join(&metadata_name);
                        if metadata_path.is_file() {
                            let metadata_bytes = read(&metadata_path)?;
                            let metadata: serde_json::Value =
                                serde_json::from_slice(&metadata_bytes)
                                    .unwrap_or(serde_json::Value::Null);
                            let valid = metadata["entries"].as_array().is_some_and(|rows| {
                                rows.len() == expected_metadata.len()
                                    && rows.iter().zip(&expected_metadata).all(
                                        |(row, (index, address, size))| {
                                            row["index"].as_u64() == Some(*index as u64)
                                                && row["ram_ptr"].as_u64() == Some(*address as u64)
                                                && row["size"].as_u64() == Some(*size as u64)
                                                && row["name"].as_str()
                                                    == Some(format!("{index}.bin").as_str())
                                        },
                                    )
                            });
                            if valid {
                                derived.insert(
                                    metadata_name,
                                    (sha256_hex(&metadata_bytes), name.clone()),
                                );
                            }
                        }
                        record.status = "container".into();
                    }
                }
            }
        } else if crate::executable::is_executable(&bytes) && crate::executable::supports(&bytes) {
            match crate::executable::extract(path, &bytes) {
                Ok(document) => {
                    let mut spans: Vec<_> = document.strings.into_iter().map(|text| {
                        range(text.file_offset, text.capacity, "known-text", format!("US executable reference sites {:?}; codec {}; consumers {}; output {:?}; runtime acceptance pending", text.references, text.codec.as_str(), text.consumers.iter().map(|address| format!("{address:#x}")).collect::<Vec<_>>().join(", "), text.output))
                    }).collect();
                    spans.extend(
                        crate::original::executable_nontext_ranges(&bytes)
                            .into_iter()
                            .map(|field| {
                                range(
                                    field.offset,
                                    field.length,
                                    "known-nontext",
                                    field.evidence.into(),
                                )
                            }),
                    );
                    spans.sort_by_key(|span| span.offset);
                    let mut cursor = 0;
                    for span in spans {
                        let end = span.offset + span.length;
                        if span.offset < cursor || end > bytes.len() {
                            return Err(Error::Invalid(
                                "executable coverage ranges overlap or exceed the source".into(),
                            ));
                        }
                        if span.offset > cursor {
                            record.ranges.push(range(
                                cursor,
                                span.offset - cursor,
                                "unresolved",
                                "executable ranges outside the supported ASCII consumers".into(),
                            ));
                        }
                        cursor = end;
                        record.ranges.push(span);
                    }
                    if cursor < bytes.len() {
                        record.ranges.push(range(
                            cursor,
                            bytes.len() - cursor,
                            "unresolved",
                            "remaining executable consumers unverified".into(),
                        ));
                    }
                    record.status = "partially-verified-executable".into();
                }
                Err(error) => {
                    record
                        .ranges
                        .push(range(0, bytes.len(), "unresolved", error.to_string()))
                }
            }
        } else {
            record.ranges.push(range(
                0,
                bytes.len(),
                "unresolved",
                "file consumer and text-bearing ranges unverified".into(),
            ));
        }
        records.insert(name, record);
    }
    for (name, (expected, source)) in derived {
        if let Some(record) = records.get_mut(&name) {
            if record.sha256 == expected {
                record.status = "verified-derivative".into();
                record.ranges.clear();
                record.derived_from = Some(source);
            }
        }
    }
    let disc_membership = disc_membership(&records)?;
    let files: Vec<_> = records.into_values().collect();
    let unresolved_ranges = files
        .iter()
        .flat_map(|file| &file.ranges)
        .filter(|range| range.status == "unresolved")
        .count();
    let mut blockers = Vec::new();
    if !disc_membership.original_files_match {
        blockers.push("original US disc files are missing or changed");
    }
    blockers.push("US executable and remaining consumers require source and runtime evidence");
    Ok(Report {
        schema: "bof3.text-coverage/v1",
        build,
        regional_support: BTreeMap::from([
            ("US", "partial; see unresolved ranges"),
            ("EU", "unverified"),
            ("JP", "unverified"),
        ]),
        policy_sha256,
        disc_membership,
        complete: false,
        unresolved_ranges,
        verified_text_entries: text_count,
        blockers,
        files,
    })
}
