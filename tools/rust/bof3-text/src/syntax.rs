//! Syntax checking and the JSON codec for editable text objects.
//!
//! The document is JSON only. Escaping is JSON's own (`\\`, `\"`, `\n`, `\uXXXX`),
//! so any literal is representable; the game's own control codes keep their
//! `{token}` spelling inside string values, a `{`/`}`/`\` that is not a token is
//! rejected, and a byte with no modelled token fails closed as `{byte(0xNN)}`.

use serde::{Deserialize, Serialize};

use crate::command;
use crate::models::{DOCUMENT_VERSION, Error, Result, invalid};

/// JSON schema identifier for the editable text-object document.
pub const DOCUMENT_SCHEMA: &str = "bof3.dialogue-text/v2";

/// A byte extent inside a subfile payload; how a raw-text instance is bounded.
#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize, Deserialize)]
pub struct Extent {
    /// Offset of the instance within its subfile payload.
    pub offset: usize,
    /// Length of the instance in bytes.
    pub length: usize,
}

/// One editable row.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct DocumentRow {
    /// One-based slot number within its bank.
    pub number: usize,
    /// Editable text; `None` marks an empty row. A choice row's text also carries
    /// the option strings that follow its `{end}`.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub text: Option<String>,
}

/// A parsed, syntax-checked editable document.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Document {
    #[serde(default = "default_version")]
    pub version: u32,
    /// Where this object came from.
    pub source: crate::models::Source,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub fingerprint: Option<String>,
    /// One entry per bank; each holds that bank's rows in slot order.
    pub banks: Vec<Vec<DocumentRow>>,
    /// Byte extent of a raw-text instance. Absent for the banked classes, whose
    /// slots are located structurally. When present, packing may only write the
    /// same number of bytes, because no allocation is proven for raw text.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub latch: Option<Extent>,
}

/// FNV-1a over a byte span, used as a raw-text instance fingerprint.
pub fn span_fingerprint(bytes: &[u8]) -> String {
    let mut hash: u64 = 0xcbf2_9ce4_8422_2325;
    for byte in bytes {
        hash ^= u64::from(*byte);
        hash = hash.wrapping_mul(0x0000_0100_0000_01b3);
    }
    format!("{hash:016x}")
}

fn default_version() -> u32 {
    DOCUMENT_VERSION
}

impl Document {
    /// Total rows across every bank.
    pub fn rows(&self) -> usize {
        self.banks.iter().map(|bank| bank.len()).sum()
    }

    /// Non-empty rows across every bank.
    pub fn used(&self) -> usize {
        self.banks
            .iter()
            .flatten()
            .filter(|row| row.text.is_some())
            .count()
    }

    /// Per-bank row counts, for comparison with a parsed section.
    pub fn bank_sizes(&self) -> Vec<usize> {
        self.banks.iter().map(|bank| bank.len()).collect()
    }

    /// One bounded summary line per non-empty row.
    pub fn summaries(&self) -> Vec<String> {
        let mut lines = Vec::new();
        for (bank, rows) in self.banks.iter().enumerate() {
            for row in rows {
                let Some(text) = &row.text else { continue };
                let commands: Vec<&str> = command::commands_in(text)
                    .iter()
                    .map(|class| class.name)
                    .collect();
                lines.push(format!(
                    "b{bank}.{}\t{} chars\tcommands: {}",
                    row.number,
                    text.chars().count(),
                    if commands.is_empty() {
                        "-".to_string()
                    } else {
                        commands.join(",")
                    }
                ));
            }
        }
        lines
    }
}

/// Check one row's text and return its encoded bytes.
pub fn check_row(text: &str) -> Result<Vec<u8>> {
    command::serialize(text)
}

/// Parse the JSON codec. The advertised schema and version are enforced.
pub fn parse_json(text: &str) -> Result<Document> {
    let value: serde_json::Value = serde_json::from_str(text)
        .map_err(|error| Error::Invalid(format!("invalid JSON: {error}")))?;
    match value.get("schema").and_then(|value| value.as_str()) {
        Some(schema) if schema == DOCUMENT_SCHEMA => {}
        Some(schema) => {
            return invalid(format!(
                "unsupported schema {schema}; expected {DOCUMENT_SCHEMA}"
            ));
        }
        None => return invalid(format!("missing schema; expected {DOCUMENT_SCHEMA}")),
    }
    let version = value
        .get("version")
        .and_then(|value| value.as_u64())
        .unwrap_or(0);
    if version != u64::from(DOCUMENT_VERSION) {
        return invalid(format!(
            "unsupported version {version}; expected {DOCUMENT_VERSION}"
        ));
    }
    let document: Document = serde_json::from_value(value)
        .map_err(|error| Error::Invalid(format!("invalid document: {error}")))?;
    if let Some(value) = &document.fingerprint {
        check_fingerprint(value)?;
    }
    validate(&document)?;
    Ok(document)
}

/// Serialize a document as JSON.
pub fn render_json(document: &Document) -> Result<String> {
    let mut value = serde_json::to_value(document)
        .map_err(|error| Error::Invalid(format!("cannot serialize document: {error}")))?;
    if let Some(object) = value.as_object_mut() {
        object.insert(
            "schema".to_string(),
            serde_json::Value::String(DOCUMENT_SCHEMA.to_string()),
        );
    }
    serde_json::to_string_pretty(&value)
        .map_err(|error| Error::Invalid(format!("cannot serialize document: {error}")))
}

/// Structural validation.
fn validate(document: &Document) -> Result<()> {
    if document.banks.is_empty() {
        return invalid("document has no banks");
    }
    for (bank, rows) in document.banks.iter().enumerate() {
        if rows.is_empty() {
            return invalid(format!("bank {bank} has no rows"));
        }
        let mut seen = vec![false; rows.len()];
        for (position, row) in rows.iter().enumerate() {
            let number = row.number;
            if number == 0 || number > rows.len() {
                return invalid(format!(
                    "bank {bank}: row {number} is outside 1..={}",
                    rows.len()
                ));
            }
            if seen[number - 1] {
                return invalid(format!("bank {bank}: duplicate row {number}"));
            }
            seen[number - 1] = true;
            if seen[position] && number != position + 1 {
                return invalid(format!("bank {bank}: rows are out of order at {number}"));
            }
            if let Some(text) = &row.text {
                command::serialize(text).map_err(|error| {
                    Error::Invalid(format!("bank {bank} row {number}: {error}"))
                })?;
            }
        }
        if let Some(missing) = seen.iter().position(|value| !*value) {
            return invalid(format!("bank {bank}: missing row {}", missing + 1));
        }
    }
    Ok(())
}

fn check_fingerprint(value: &str) -> Result<()> {
    if value.len() != 16 || !value.chars().all(|character| character.is_ascii_hexdigit()) {
        return invalid(format!("header block_fingerprint is malformed: {value}"));
    }
    Ok(())
}
