//! Byte-based analysis of verified bank rows and their shared allocations.

use std::fs;
use std::path::Path;

use serde_json::{Value, json};

use crate::models::{Result, TextClass, command_for};

pub fn analyze(path: &Path) -> Result<Value> {
    let data = fs::read(path)?;
    if crate::characters::supports(&data) {
        return serde_json::to_value(crate::characters::extract(path, &data)?)
            .map_err(|error| crate::models::Error::Invalid(error.to_string()));
    }
    if crate::records::supports(&data) {
        return serde_json::to_value(crate::records::extract(path, &data)?)
            .map_err(|error| crate::models::Error::Invalid(error.to_string()));
    }
    if crate::executable::is_executable(&data) {
        return serde_json::to_value(crate::executable::extract(path, &data)?)
            .map_err(|error| crate::models::Error::Invalid(error.to_string()));
    }
    let mut entries = Vec::new();
    let mut inline_entries = Vec::new();
    for (entry, payload) in crate::lexagraph::entries(&data)? {
        if crate::inline::supports(entry, payload) {
            inline_entries.push(crate::inline::extract(path, &data, entry, payload)?);
            continue;
        }
        let Some(class) = TextClass::from_load_argument(entry.ram_ptr) else {
            continue;
        };
        let section = crate::extract::parse_section(payload)?;
        let mut rows = Vec::new();
        for bank in &section.banks {
            for row in &bank.rows {
                let mut commands = Vec::new();
                let mut ends = Vec::new();
                let mut at = 0;
                while at < row.encoded.len() {
                    let byte = row.encoded[at];
                    if let Some(command) = command_for(byte) {
                        let width = command.width(row.encoded.get(at + 1).copied());
                        if at + 1 + width <= row.encoded.len() {
                            commands.push(json!({"offset": at, "code": byte, "name": command.name, "operands": &row.encoded[at+1..at+1+width]}));
                            if byte == 0 {
                                ends.push(at);
                            }
                            at += width + 1;
                            continue;
                        }
                    }
                    at += 1;
                }
                let shared: Vec<_> = bank
                    .rows
                    .iter()
                    .filter(|other| row.span.is_some() && other.span == row.span)
                    .map(|other| other.number)
                    .collect();
                let segments: Vec<_> = ends.windows(2).filter_map(|pair| {
                    let start = pair[0] + 1;
                    let end = pair[1];
                    (end > start).then(|| json!({"offset": start, "length": end-start, "text": crate::command::deserialize(&row.encoded[start..end])}))
                }).collect();
                let text = crate::command::deserialize(&row.encoded);
                let encoded_size = crate::syntax::check_row(&text)?.len();
                rows.push(json!({
                    "bank": bank.index, "row": row.number,
                    "archive_offset": row.span.map(|(start, _)| entry.offset + bank.start + start),
                    "payload_offset": row.span.map(|(start, _)| bank.start + start),
                    "capacity": row.span.map_or(0, |(start, end)| end-start),
                    "encoded_size": encoded_size, "text": text,
                    "commands": commands, "terminator_offsets": ends,
                    "trailing_string_segments": segments,
                    "trailing_segments_note": "structural strings after a terminator; consumer evidence determines option semantics",
                    "shared_rows": shared,
                }));
            }
        }
        entries.push(json!({"entry": entry.index, "class": class, "offset": entry.offset, "size": entry.size, "rows": rows}));
    }
    Ok(
        json!({"schema": "bof3.text-analysis/v1", "archive": path.to_string_lossy(), "archive_sha256": crate::digest::sha256_hex(&data), "scope": "verified EMI banks and supported inline allocations", "entries": entries, "inline_entries": inline_entries}),
    )
}
