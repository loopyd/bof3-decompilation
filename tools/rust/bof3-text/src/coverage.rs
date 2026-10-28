//! Deterministic accounting of an extracted disc tree, including unresolved data.

use std::collections::BTreeMap;
use std::fs;
use std::path::{Path, PathBuf};

use serde::Serialize;

use crate::digest::sha256_hex;
use crate::filters::KnownPayload;
use crate::models::{Error, Result, TextClass};

const US_EXECUTABLE_SHA256: &str =
    "0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a";

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
        if path.extension().is_some_and(|extension| extension == "EMI") {
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
                            let inline = crate::inline::supports(*entry, payload)
                                .then(|| crate::inline::extract(path, &bytes, *entry, payload).ok())
                                .flatten();
                            let monster = crate::monsters::supports(*entry, payload)
                                .then(|| {
                                    crate::monsters::extract(path, &bytes, *entry, payload).ok()
                                })
                                .flatten();
                            if let Some(mut document) = inline {
                                text_count += 1;
                                document.strings.sort_by_key(|text| text.file_offset);
                                let mut at = entry.offset;
                                for text in document.strings {
                                    if text.file_offset > at {
                                        record.ranges.push(range(
                                            at,
                                            text.file_offset - at,
                                            "unresolved",
                                            "bytes outside the supported inline allocations".into(),
                                        ));
                                    }
                                    record.ranges.push(RangeRecord {
                                        offset: text.file_offset, length: text.capacity,
                                        entry: Some(entry.index), status: "known-text",
                                        evidence: format!("{} allocation {:#x}; consumers {}; renderers {}; codec {}; runtime acceptance pending", document.layout, text.address, document.consumers.iter().map(|address| format!("{address:#x}")).collect::<Vec<_>>().join(", "), if text.renderers.is_empty() { "unverified".into() } else { text.renderers.iter().map(|address| format!("{address:#x}")).collect::<Vec<_>>().join(", ") }, text.codec.as_str()),
                                    });
                                    at = text.file_offset + text.capacity;
                                }
                                let end = entry.offset + payload.len();
                                if at < end {
                                    record.ranges.push(range(
                                        at,
                                        end - at,
                                        "unresolved",
                                        "remaining inline payload consumers unverified".into(),
                                    ));
                                }
                            } else if let Some(names) = monster
                                .as_ref()
                                .map(|document| &document.names)
                                .or_else(|| {
                                    names
                                        .as_ref()
                                        .filter(|(index, _)| entry.index == *index)
                                        .map(|(_, names)| names)
                                })
                            {
                                text_count += 1;
                                let mut at = entry.offset;
                                for row in names {
                                    if row.file_offset > at {
                                        record.ranges.push(range(
                                            at,
                                            row.file_offset - at,
                                            "unresolved",
                                            "bytes outside the supported name record fields".into(),
                                        ));
                                    }
                                    let consumer = if row.family == "monsters" {
                                        "battle template copy 0x800ac218; lookup 0x800ac704; eight-byte name copy 0x801dea18"
                                    } else if row.family == "characters" {
                                        "START/STATUS initializer 0x801d15c0; SISYOU five-byte copy 0x801d39cc and draw call 0x801d1848"
                                    } else if row.family == "abilities" {
                                        "SISYOU entry 0 twelve-byte copy 0x801d3d64"
                                    } else {
                                        "resolver 0x80165d48; COMMU00 entry 0 twelve-byte copy 0x801f1a30"
                                    };
                                    record.ranges.push(RangeRecord {
                                        offset: row.file_offset,
                                        length: row.capacity,
                                        entry: Some(entry.index),
                                        status: "known-text",
                                        evidence: format!(
                                            "{} record {}; {consumer}; runtime acceptance pending",
                                            row.family, row.index
                                        ),
                                    });
                                    at = row.file_offset + row.capacity;
                                }
                                let end = entry.offset + payload.len();
                                if at < end {
                                    record.ranges.push(range(
                                        at,
                                        end - at,
                                        "unresolved",
                                        "remaining record entry consumers unverified".into(),
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
                    let mut cursor = 0;
                    for text in document.strings {
                        if text.file_offset > cursor {
                            record.ranges.push(range(
                                cursor,
                                text.file_offset - cursor,
                                "unresolved",
                                "executable ranges outside the supported ASCII consumers".into(),
                            ));
                        }
                        record.ranges.push(range(text.file_offset, text.capacity, "known-text", format!("US executable reference sites {:?}; codec {}; consumer {:#x}, formatter 0x8017e3f4 for hexadecimal formats, renderer 0x80150098; runtime acceptance pending", text.references, text.codec.as_str(), text.consumer)));
                        cursor = text.file_offset + text.capacity;
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
