//! Small XML 1.0 writer for extraction manifests; names are static schema names.

use crate::Result;
use serde::Serialize;
use std::fmt::Write;

pub(crate) fn attribute(value: &str) -> Result<String> {
    let mut escaped = String::new();
    for c in value.chars() {
        match c {
            '&' => escaped.push_str("&amp;"),
            '<' => escaped.push_str("&lt;"),
            '>' => escaped.push_str("&gt;"),
            '"' => escaped.push_str("&quot;"),
            '\'' => escaped.push_str("&apos;"),
            '\t' => escaped.push_str("&#9;"),
            '\n' => escaped.push_str("&#10;"),
            '\r' => escaped.push_str("&#13;"),
            '\u{20}'..='\u{d7ff}' | '\u{e000}'..='\u{fffd}' | '\u{10000}'..='\u{10ffff}' => {
                escaped.push(c)
            }
            _ => {
                return Err(
                    format!("XML 1.0 cannot represent character U+{:04X}", u32::from(c)).into(),
                )
            }
        }
    }
    Ok(escaped)
}

/// Emit scalar fields as attributes. Nested fields must be explicitly handled by
/// the caller: an unexpected array/object is an error rather than lost metadata.
pub(crate) fn attributes(value: &impl Serialize, nested: &[&str]) -> Result<String> {
    let value = serde_json::to_value(value)?;
    let fields = value.as_object().ok_or("XML metadata must be an object")?;
    let mut output = String::new();
    for (key, value) in fields {
        if nested.contains(&key.as_str()) {
            continue;
        }
        let scalar = match value {
            serde_json::Value::String(value) => value.clone(),
            serde_json::Value::Number(_) | serde_json::Value::Bool(_) => value.to_string(),
            _ => return Err(format!("XML metadata field {key} needs explicit handling").into()),
        };
        write!(output, " {key}=\"{}\"", attribute(&scalar)?)?;
    }
    Ok(output)
}

pub(crate) fn hex(bytes: &[u8]) -> String {
    const DIGITS: &[u8; 16] = b"0123456789abcdef";
    let mut text = String::with_capacity(bytes.len() * 2);
    for &byte in bytes {
        text.push(char::from(DIGITS[usize::from(byte >> 4)]));
        text.push(char::from(DIGITS[usize::from(byte & 15)]));
    }
    text
}
