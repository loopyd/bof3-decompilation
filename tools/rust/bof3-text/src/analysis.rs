//! Byte-based analysis of verified text and its allocations.

use std::fs;
use std::path::Path;

use serde_json::{Value, json};

use crate::models::{Error, Result, TextClass, command_for};

enum Encoding {
    Ascii,
    GameLiteral,
    Inline,
}

fn literal_report(
    document: impl serde::Serialize,
    field: &str,
    data: &[u8],
    encoding: Encoding,
    terminator_bytes: usize,
) -> Result<Value> {
    let mut report =
        serde_json::to_value(document).map_err(|error| Error::Invalid(error.to_string()))?;
    let source_schema = report["schema"].take();
    report["schema"] = json!("bof3.text-analysis/v1");
    report["source_schema"] = source_schema;
    report["encoding"] = json!(match encoding {
        Encoding::Ascii => "ascii",
        Encoding::GameLiteral => "game-literal",
        Encoding::Inline => "per-string",
    });
    let rows = report[field]
        .as_array_mut()
        .ok_or_else(|| Error::Invalid("literal analysis has no allocations".into()))?;
    for row in rows {
        let number = |name: &str| -> Result<usize> {
            row[name]
                .as_u64()
                .and_then(|value| usize::try_from(value).ok())
                .ok_or_else(|| Error::Invalid(format!("invalid analysis {name}")))
        };
        let offset = number("file_offset")?;
        let capacity = number("capacity")?;
        let inline_codec: Option<crate::inline::Codec> = if matches!(encoding, Encoding::Inline) {
            Some(
                serde_json::from_value(row["codec"].clone())
                    .map_err(|error| Error::Invalid(error.to_string()))?,
            )
        } else {
            None
        };
        let length_bytes = inline_codec.map_or(0, crate::inline::Codec::length_bytes);
        let terminator_bytes =
            inline_codec.map_or(terminator_bytes, crate::inline::Codec::terminator_bytes);
        let maximum = capacity
            .checked_sub(terminator_bytes + length_bytes)
            .ok_or_else(|| Error::Invalid("analysis allocation cannot hold its metadata".into()))?;
        let text = row["text"]
            .as_str()
            .ok_or_else(|| Error::Invalid("literal analysis has no text".into()))?;
        let mut max_formatted_size = None;
        let encoded_size = match encoding {
            Encoding::Ascii => {
                let codec: crate::executable::Codec = serde_json::from_value(row["codec"].clone())
                    .map_err(|error| Error::Invalid(error.to_string()))?;
                max_formatted_size = codec.max_formatted_size(text)?;
                codec.encode(text, maximum)?.len()
            }
            Encoding::GameLiteral => crate::records::encode_name(text, maximum)?.len(),
            Encoding::Inline => {
                let codec = inline_codec.unwrap();
                max_formatted_size = codec.max_formatted_size(text)?;
                codec.encode(text, maximum)?.len()
            }
        };
        let remaining = maximum
            .checked_sub(encoded_size)
            .ok_or_else(|| Error::Invalid("analysis text exceeds its allocation".into()))?;
        let commands = if let Some(codec) = inline_codec {
            serde_json::to_value(codec.commands(text, maximum)?)
                .map_err(|error| Error::Invalid(error.to_string()))?
        } else {
            json!([])
        };
        let bytes = offset
            .checked_add(capacity)
            .and_then(|end| data.get(offset..end))
            .ok_or_else(|| Error::Invalid("analysis allocation escapes source bytes".into()))?;
        row["encoded_size"] = json!(encoded_size);
        row["max_encoded_size"] = json!(maximum);
        row["required_terminator_bytes"] = json!(terminator_bytes);
        row["required_length_bytes"] = json!(length_bytes);
        row["remaining_capacity"] = json!(remaining);
        row["original_bytes"] = json!(
            bytes
                .iter()
                .map(|byte| format!("{byte:02x}"))
                .collect::<String>()
        );
        row["commands"] = commands;
        if let Some(size) = max_formatted_size {
            row["max_formatted_size"] = json!(size);
        }
    }
    Ok(report)
}

pub fn analyze(path: &Path) -> Result<Value> {
    let data = fs::read(path)?;
    if crate::records::supports(&data) {
        return literal_report(
            crate::records::extract(path, &data)?,
            "names",
            &data,
            Encoding::GameLiteral,
            0,
        );
    }
    if crate::executable::is_executable(&data) {
        return literal_report(
            crate::executable::extract(path, &data)?,
            "strings",
            &data,
            Encoding::Ascii,
            1,
        );
    }
    let mut entries = Vec::new();
    let mut inline_entries = Vec::new();
    let mut monster_entries = Vec::new();
    for (entry, payload) in crate::lexagraph::entries(&data)? {
        if crate::monsters::supports(entry, payload) {
            monster_entries.push(literal_report(
                crate::monsters::extract(path, &data, entry, payload)?,
                "names",
                &data,
                Encoding::GameLiteral,
                0,
            )?);
            continue;
        }
        if crate::inline::supports(entry, payload) {
            inline_entries.push(literal_report(
                crate::inline::extract(path, &data, entry, payload)?,
                "strings",
                &data,
                Encoding::Inline,
                1,
            )?);
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
        json!({"schema": "bof3.text-analysis/v1", "archive": path.to_string_lossy(), "archive_sha256": crate::digest::sha256_hex(&data), "scope": "verified EMI banks, inline allocations and monster name fields", "entries": entries, "inline_entries": inline_entries, "monster_entries": monster_entries}),
    )
}
