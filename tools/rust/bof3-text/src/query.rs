//! Indexing, querying and command accounting over dialogue objects.

use std::collections::BTreeMap;

use serde::Serialize;

use crate::command;
use crate::extract;
use crate::models::{Entry, Result, Section, TextClass};

/// Query allocations from a verified literal codec, retaining its source metadata.
pub fn literal(
    document: impl Serialize,
    needle: Option<&str>,
    ignore_case: bool,
    match_offset: bool,
    commands: bool,
) -> Result<serde_json::Value> {
    use serde_json::json;
    let mut report = serde_json::to_value(document)
        .map_err(|error| crate::models::Error::Invalid(error.to_string()))?;
    let field = if report.get("names").is_some() {
        "names"
    } else {
        "strings"
    };
    let rows =
        report[field].take().as_array().cloned().ok_or_else(|| {
            crate::models::Error::Invalid("literal query has no allocations".into())
        })?;
    report
        .as_object_mut()
        .ok_or_else(|| {
            crate::models::Error::Invalid("literal query document is not an object".into())
        })?
        .remove(field);
    report["source_schema"] = report["schema"].take();
    report["schema"] = json!("bof3.literal-query/v1");
    report["rows"] = json!(rows.len());
    let mut matches = Vec::new();
    let mut glyph_count = 0;
    for mut row in rows {
        let text = row["text"]
            .as_str()
            .ok_or_else(|| crate::models::Error::Invalid("literal query has no text".into()))?;
        let offset = row["file_offset"]
            .as_u64()
            .ok_or_else(|| crate::models::Error::Invalid("literal query has no offset".into()))?;
        let capacity = row["capacity"]
            .as_u64()
            .ok_or_else(|| crate::models::Error::Invalid("literal query has no capacity".into()))?;
        let glyphs = row["codec"] == json!(crate::inline::Codec::GameGlyphs);
        let maximum = capacity.checked_sub(1).ok_or_else(|| {
            crate::models::Error::Invalid("literal allocation has no terminator capacity".into())
        })?;
        let maximum = usize::try_from(maximum)
            .map_err(|error| crate::models::Error::Invalid(error.to_string()))?;
        if commands {
            if glyphs {
                glyph_count += crate::inline::Codec::GameGlyphs
                    .commands(text, maximum)?
                    .len();
            }
            continue;
        }
        let hit = match needle {
            Some(needle) => {
                let hit = if glyphs {
                    crate::search::find_glyphs(
                        &crate::inline::Codec::GameGlyphs.encode(text, maximum)?,
                        needle,
                        ignore_case,
                    )
                } else {
                    crate::search::find_literal(text, needle, ignore_case)
                };
                match hit {
                    Some(hit) => Some(hit),
                    None => continue,
                }
            }
            None => None,
        };
        let range = hit.as_ref().and_then(|hit| hit.byte_range);
        let matched_offset = range.map(|(start, _)| offset + start as u64);
        let matched_length = range.map(|(start, end)| end - start);
        row["string_offset"] = json!(offset);
        row["string_length"] = json!(capacity);
        row["match_offset"] = json!(matched_offset);
        row["match_length"] = json!(matched_length);
        row["anchor"] = json!(if match_offset { "match" } else { "string" });
        row["reported_offset"] = json!(if match_offset {
            matched_offset.unwrap_or(offset)
        } else {
            offset
        });
        row["reported_length"] = json!(if match_offset {
            matched_length.map_or(capacity, |length| length as u64)
        } else {
            capacity
        });
        if let Some(hit) = hit {
            row["matched"] = json!(hit.text);
            row["reading"] = json!(hit.reading);
        }
        matches.push(row);
    }
    if commands {
        report["usage"] = if glyph_count == 0 {
            json!([])
        } else {
            json!([{"command": "glyph", "count": glyph_count}])
        };
        return Ok(report);
    }
    report["matched"] = json!(matches.len());
    report["matches"] = json!(matches);
    Ok(report)
}

/// One supported text layout in a source file.
#[derive(Clone, Debug, Serialize)]
pub struct IndexedEntry {
    pub entry: Option<usize>,
    pub offset: usize,
    pub ram_ptr: Option<String>,
    pub size: usize,
    pub class: String,
    pub banks: Option<usize>,
    pub rows: usize,
    pub used: Option<usize>,
    /// Bank rows whose extent carries an option list after its `{end}`.
    pub options: Option<usize>,
    pub status: String,
}

/// Summarize banks while retaining parse failures in their status fields.
pub fn index_banks<'a>(
    entries: impl IntoIterator<Item = (Entry, TextClass, &'a [u8])>,
) -> Vec<IndexedEntry> {
    entries
        .into_iter()
        .map(|(entry, class, payload)| {
            let mut record = IndexedEntry {
                entry: Some(entry.index),
                offset: entry.offset,
                ram_ptr: Some(format!("{:#010x}", entry.ram_ptr)),
                size: entry.size as usize,
                class: class.name().into(),
                banks: Some(0),
                rows: 0,
                used: Some(0),
                options: Some(0),
                status: "ok".into(),
            };
            match extract::parse_section(payload) {
                Ok(section) => {
                    record.banks = Some(section.banks.len());
                    record.rows = section.rows();
                    record.used = Some(section.used());
                    record.options = Some(extract::option_rows(&section));
                }
                Err(error) => record.status = error.to_string(),
            }
            record
        })
        .collect()
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
