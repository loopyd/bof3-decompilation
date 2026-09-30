//! Indexing, querying and command accounting over dialogue objects.

use std::collections::BTreeMap;

use serde::Serialize;

use crate::command;
use crate::extract;
use crate::models::{Result, Section};

/// One indexed dialogue subfile.
#[derive(Clone, Debug, Serialize)]
pub struct IndexedEntry {
    pub entry: usize,
    pub offset: usize,
    pub ram_ptr: String,
    pub size: u32,
    /// Text class.
    pub class: crate::models::TextClass,
    pub banks: usize,
    pub rows: usize,
    pub used: usize,
    /// Rows whose extent carries an option list after its `{end}`.
    pub options: usize,
    pub status: String,
}

/// Index every dialogue subfile of an archive.
pub fn index(data: &[u8]) -> Result<Vec<IndexedEntry>> {
    let mut indexed = Vec::new();
    for (entry, class) in extract::text_entries(data)? {
        let payload = &data[entry.offset..entry.offset + entry.size as usize];
        let mut record = IndexedEntry {
            entry: entry.index,
            offset: entry.offset,
            ram_ptr: format!("{:#010x}", entry.ram_ptr),
            size: entry.size,
            class,
            banks: 0,
            rows: 0,
            used: 0,
            options: 0,
            status: "ok".to_string(),
        };
        match extract::parse_section(payload) {
            Ok(section) => {
                record.banks = section.banks.len();
                record.rows = section.rows();
                record.used = section.used();
                record.options = extract::option_rows(&section);
            }
            Err(error) => record.status = error.to_string(),
        }
        indexed.push(record);
    }
    Ok(indexed)
}

/// One row selected by a query.
#[derive(Clone, Debug, Serialize)]
pub struct RowMatch {
    pub bank: usize,
    pub number: usize,
    /// Bank-relative start of the row's extent.
    pub offset: usize,
    /// Extent length in bytes.
    pub length: usize,
    /// Archive-absolute start of the containing string — the row extent, which is where a hit is
    /// anchored unless `--match-offset` asks for the match start instead.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub string_offset: Option<usize>,
    /// Length of the containing string in bytes.
    pub string_length: usize,
    /// Whether the row carries an option list after its `{end}`.
    pub options: bool,
    pub text: String,
    /// Bank-relative byte offset of the match itself, when the needle has literal bytes.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub match_offset: Option<usize>,
    /// Byte length of the match.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub match_length: Option<usize>,
    /// The matched text as read, with commands rendered as breaks.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub matched: Option<String>,
    /// Which reading produced the hit: `spaced` (a command read as a break) or `joined`
    /// (a command dropped, which can synthesise a join — report it, never hide it).
    #[serde(skip_serializing_if = "Option::is_none")]
    pub reading: Option<&'static str>,
    /// The anchor the caller asked to be reported: the containing `string`, or the `match`.
    pub anchor: String,
    /// The reported offset and length, following the anchor selection rather than being fixed.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub reported_offset: Option<usize>,
    pub reported_length: usize,
}

/// Rows of a section, optionally filtered by a command-transparent text match.
///
/// Matching runs over [`crate::search`] readings, never over the decoded token string, so
/// command names and operand bytes cannot match and a phrase may span a command.
///
/// `payload_base` is the subfile's offset in the archive file, so a hit's `match_offset` is a true
/// offset in the archive rather than a bank-relative one. The pre-existing `offset` field keeps its
/// documented bank-relative meaning.
pub fn query(
    section: &Section,
    needle: Option<&str>,
    ignore_case: bool,
    payload_base: usize,
    match_offset: bool,
) -> Vec<RowMatch> {
    // The recorded row extents of this section, payload-relative and sorted: the boundaries a match
    // is walked back over to find the beginning of the string it sits in.
    let mut extents: Vec<crate::lexagraph::Extent> = Vec::new();
    for bank in &section.banks {
        for row in &bank.rows {
            if let Some((start, end)) = row.span {
                extents.push((payload_base + bank.start + start, end - start));
            }
        }
    }
    extents.sort_unstable();
    let mut matches = Vec::new();
    for bank in &section.banks {
        for row in &bank.rows {
            let Some((start, end)) = row.span else {
                continue;
            };
            let text = command::deserialize(&row.encoded);
            let hit = match needle {
                None => None,
                Some(needle) => match crate::search::find(&row.encoded, needle, ignore_case) {
                    Some(hit) => Some(hit),
                    None => continue,
                },
            };
            let options = row.encoded[..row.encoded.len().saturating_sub(1)].contains(&0x00);
            // A row span is relative to its **bank**, so the bank's start must be added: without
            // it every bank-1 offset of a two-bank subfile was short by the bank's start.
            let row_start = bank.start + start;
            let (match_offset_value, match_length, matched, reading) = match &hit {
                Some(hit) => (
                    hit.byte_range
                        .map(|(offset, _)| payload_base + row_start + offset),
                    hit.byte_range.map(|(offset, end)| end - offset),
                    Some(hit.text.clone()),
                    Some(hit.reading),
                ),
                None => (None, None, None, None),
            };
            // The beginning is *walked back to* from the match over the recorded row extents; the
            // row's own span is used only when there is no match to walk back from.
            let anchored = match_offset_value
                .and_then(|offset| crate::lexagraph::walk_back_to_containing(&extents, offset))
                .unwrap_or((payload_base + row_start, end - start));
            matches.push(RowMatch {
                bank: bank.index,
                number: row.number,
                offset: start,
                length: end - start,
                string_offset: Some(anchored.0),
                string_length: anchored.1,
                options,
                text,
                match_offset: match_offset_value,
                match_length,
                matched,
                reading,
                anchor: if match_offset {
                    "match".to_string()
                } else {
                    "string".to_string()
                },
                reported_offset: if match_offset {
                    match_offset_value
                } else {
                    Some(anchored.0)
                },
                reported_length: if match_offset {
                    match_length.unwrap_or(anchored.1)
                } else {
                    anchored.1
                },
            });
        }
    }
    matches
}

/// Command usage histogram, ordered by descending count.
pub fn command_usage(section: &Section) -> Vec<(&'static str, usize)> {
    let mut counts: BTreeMap<&'static str, usize> = BTreeMap::new();
    for bank in &section.banks {
        for row in &bank.rows {
            for class in command::commands_in(&command::deserialize(&row.encoded)) {
                *counts.entry(class.name).or_default() += 1;
            }
        }
    }
    let mut ordered: Vec<(&'static str, usize)> = counts.into_iter().collect();
    ordered.sort_by(|left, right| right.1.cmp(&left.1).then(left.0.cmp(right.0)));
    ordered
}

/// Render matches as a JSON array.
pub fn query_json(matches: &[RowMatch]) -> Result<serde_json::Value> {
    serde_json::to_value(matches).map_err(|error| {
        crate::models::Error::Invalid(format!("cannot serialize matches: {error}"))
    })
}
