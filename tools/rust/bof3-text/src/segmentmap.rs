//! Text-versus-data segment mapping report (read-only).
//!
//! This exists to answer one question for the lifting toolside: *which byte ranges are text,
//! and on what evidence?* It decides nothing by itself and edits nothing. Three states are kept
//! distinct rather than collapsed:
//!
//! - [`Role::Text`] — a **verified** text bank: it parses as a pointer-table section *and*
//!   reproduces byte-identically, and its class is one with evidence.
//! - text-like runs — the scanner's latches **inside data**. These are candidates listed with
//!   their extents, never ranges to act on.
//! - everything else — data, including subfiles nothing identifies.
//!
//! Classification outside the verified classes rests on byte-verifiable magic in the payload
//! itself (`pBAV`, `pQES`) rather than on an address table, so the report can be checked against
//! the archive. Where an identity comes from outside knowledge it is not asserted here.

use std::collections::BTreeMap;
use std::fs;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::extract;
use crate::lexagraph;
use crate::models::{Error, Result, Source, TextClass};
use crate::pack;

/// The report's schema identifier.
pub const MAP_SCHEMA: &str = "bof3.text-segment-map/v1";
/// Report schema version.
pub const MAP_VERSION: u32 = 1;

/// What a subfile range is, on present evidence.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub enum Role {
    /// A verified text bank: parses as a section and reproduces byte-identically.
    Text,
    /// Anything else. Text-like runs inside it are candidates only.
    Data,
}

/// One text-like run inside a subfile.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Run {
    /// Offset within the subfile payload.
    pub offset: usize,
    /// Run length in bytes.
    pub length: usize,
}

/// One subfile, mapped.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Subfile {
    pub entry: usize,
    /// TOC load argument, as `0x…`.
    pub address: String,
    /// Absolute offset of the payload in the archive file.
    pub offset: usize,
    pub size: u32,
    pub role: Role,
    /// Structural class, when the load argument selects a known one.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub class: Option<TextClass>,
    /// Whether the payload parsed as a text-bank section.
    pub parsed: bool,
    /// Whether an unedited round trip reproduced it, when it parsed.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub round_trip: Option<bool>,
    /// Byte-verifiable magic in the payload, when one is recognised.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub magic: Option<String>,
    /// Scanner latches inside this subfile: **candidates**, never text ranges.
    #[serde(skip_serializing_if = "Vec::is_empty")]
    pub text_like_runs: Vec<Run>,
    /// Latches lying wholly inside a parsed bank's string body. Present only when the subfile
    /// parsed, and it does **not** mean the latch is text: it is confined to the text region of a
    /// verified bank.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub latches_confined_to_text_body: Option<usize>,
    /// Latches that straddle a pointer table, bank header or prefix inside a parsed subfile.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub latches_straddling_text_structure: Option<usize>,
    /// True when nothing identifies this subfile: no class, no parse, no magic.
    pub unidentified: bool,
}

/// One archive's subfiles.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Archive {
    pub archive: String,
    pub subfiles: Vec<Subfile>,
}

/// The whole report.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Report {
    pub schema: String,
    pub version: u32,
    pub root: String,
    /// Standing reminder that this is evidence, not an instruction.
    pub note: String,
    pub totals: BTreeMap<String, usize>,
    /// Identified per-window removals, not only the aggregate counts.
    pub windows_detail: Vec<crate::lexagraph::WindowDetail>,
    pub archives: Vec<Archive>,
}

/// The byte-verifiable four-byte signature at the head of a payload, when it has one.
///
/// Only bytes are reported here. What a signature *means* (that `pBAV` is a VAB sound bank) comes
/// from the EU knowledgebase and is deliberately **not** encoded in the artifact: it stays prose,
/// labelled as a lead, so the report cannot be mistaken for asserting a US fact it did not check.
fn magic(payload: &[u8]) -> Option<String> {
    let head = payload.get(..4)?;
    match head {
        b"pBAV" | b"pQES" => Some(String::from_utf8_lossy(head).into_owned()),
        _ => None,
    }
}

/// Map every archive under `root`.
pub fn build(
    root: &Path,
    windows: &crate::windows::WindowIndex,
    payloads: &crate::filters::KnownPayload,
    vocabulary: &crate::filters::Vocabulary,
    readability: crate::filters::Readability,
) -> Result<Report> {
    let mut archives = Vec::new();
    let mut windows_detail = Vec::new();
    let mut totals: BTreeMap<String, usize> = BTreeMap::new();
    let mut count = |key: &str, by: usize| {
        *totals.entry(key.to_string()).or_default() += by;
    };
    // Composed once, from the filters' own notion of what an opt-out means.
    let filters = crate::filters::compose(windows, payloads, vocabulary, readability);
    for path in crate::textindex::archives(root)? {
        let data = fs::read(&path)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        let name = crate::textindex::relative_archive(root, &path);
        let mut subfiles = Vec::new();
        let mut preceding: Vec<&[u8]> = Vec::new();
        for (entry, payload) in lexagraph::entries(&data)? {
            let class = TextClass::from_load_argument(entry.ram_ptr);
            // The admission decision comes first: an identified payload is never parsed at all.
            let input = crate::models::PreFilterInput {
                archive: &name,
                entry: entry.index,
                payload,
                class,
                preceding: &preceding,
            };
            let mut parsed = false;
            let mut round_trip = None;
            let mut parsed_banks = None;
            if !filters.identifies_payload(&input) {
                if let Ok(section) = extract::parse_section(payload) {
                    parsed_banks = Some(section.banks.clone());
                    parsed = true;
                    let source = Source::new(
                        name.clone(),
                        Some(entry.index),
                        entry.ram_ptr,
                        class.unwrap_or(TextClass::Heuristic),
                    );
                    let document = extract::document(&section, &source);
                    round_trip = Some(
                        matches!(pack::rebuild(&section, &document), Ok(rebuilt) if rebuilt == payload),
                    );
                }
            }
            // A range is verified text only when it both parses and reproduces.
            let role = if parsed && round_trip == Some(true) && class.is_some() {
                count("verified_text_subfiles", 1);
                Role::Text
            } else {
                count("data_subfiles", 1);
                Role::Data
            };
            let found_magic = magic(payload);
            if let Some(what) = &found_magic {
                count(&format!("magic:{what}"), 1);
            }
            let (latches, tally) = filters.scan_bytes(&input, lexagraph::MIN_RUN);
            preceding.push(payload);
            windows_detail.extend(crate::lexagraph::details(&name, entry.index, &tally));
            count("windows_applied", tally.windows);
            count("window_bytes_skipped", tally.bytes_skipped);
            count("latches_removed_by_windows", tally.latches_removed);
            count("latches_split_by_windows", tally.latches_split);
            let text_like_runs: Vec<Run> = latches
                .into_iter()
                .map(|latch| Run {
                    offset: latch.offset,
                    length: latch.length,
                })
                .collect();
            // Runs inside a verified text bank are that bank's own text; runs inside data are
            // candidates. Counting them together would overstate the candidate set, so they are
            // counted apart.
            let mut confined = None;
            let mut straddling = None;
            if let Some(banks) = &parsed_banks {
                let mut inside = 0usize;
                for run in &text_like_runs {
                    let end = run.offset + run.length;
                    let in_body = banks.iter().any(|bank| {
                        let body_start = bank.start + bank.pointer_bytes;
                        let body_end = bank.start + bank.size;
                        run.offset >= body_start && end <= body_end
                    });
                    if in_body {
                        inside += 1;
                    }
                }
                confined = Some(inside);
                straddling = Some(text_like_runs.len() - inside);
                count("latches_confined_to_text_body", inside);
                count("latches_straddling_text_structure", straddling.unwrap_or(0));
            }
            if !text_like_runs.is_empty() {
                if role == Role::Text {
                    count("text_like_runs_in_text_subfiles", text_like_runs.len());
                } else {
                    count("subfiles_with_text_like_runs", 1);
                    count("text_like_runs_in_data_subfiles", text_like_runs.len());
                }
            }
            // "Unidentified" must mean nothing identifies it: a payload with a recognised magic
            // is identified even though it is not text.
            let unidentified = role == Role::Data && found_magic.is_none();
            if unidentified {
                count("unidentified_data_subfiles", 1);
            }
            subfiles.push(Subfile {
                entry: entry.index,
                address: format!("{:#010x}", entry.ram_ptr),
                offset: entry.offset,
                size: entry.size,
                role,
                class,
                parsed,
                round_trip,
                magic: found_magic,
                text_like_runs,
                latches_confined_to_text_body: confined,
                latches_straddling_text_structure: straddling,
                unidentified,
            });
        }
        count("archives", 1);
        count("subfiles", subfiles.len());
        archives.push(Archive {
            archive: name,
            subfiles,
        });
    }
    archives.sort_by(|left, right| left.archive.cmp(&right.archive));
    Ok(Report {
        schema: MAP_SCHEMA.to_string(),
        version: MAP_VERSION,
        root: root.display().to_string(),
        windows_detail,
        note: "Evidence, not an instruction: this report proposes text-or-data roles and makes no configuration change. Text-like runs inside data are candidates, not text.".to_string(),
        totals,
        archives,
    })
}

/// Write the report as JSON.
pub fn write(report: &Report, path: &Path) -> Result<()> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| Error::Invalid(format!("{}: {error}", parent.display())))?;
    }
    let body = serde_json::to_string_pretty(report)
        .map_err(|error| Error::Invalid(format!("cannot serialize map report: {error}")))?;
    fs::write(path, format!("{body}\n"))
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
    Ok(())
}
