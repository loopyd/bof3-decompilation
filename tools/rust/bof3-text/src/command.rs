//! Serializer and deserializer for the editable text grammar.
//!
//! Decoding is total: every input byte becomes either a literal character, a
//! `{command(arg, arg)}` token, or a `{byte(0xNN)}` token. Encoding is the exact
//! inverse, so an unedited round trip reproduces the original bytes.

use std::fmt::Write as _;

use crate::models::{
    Command, CommandDef, Result, command_for, command_named, invalid, literal_byte, literal_char,
};

/// Deserialize a row extent into editable text.
pub fn deserialize(bytes: &[u8]) -> String {
    let mut output = String::new();
    let mut index = 0;
    while index < bytes.len() {
        let byte = bytes[index];
        if let Some(character) = literal_char(byte) {
            output.push(character);
            index += 1;
            continue;
        }
        let Some(class) = command_for(byte) else {
            let _ = write!(output, "{{byte(0x{byte:02x})}}");
            index += 1;
            continue;
        };
        let first = bytes.get(index + 1).copied();
        let width = class.width(first);
        // A command byte whose operands are cut off by the extent is not a
        // command; emit it verbatim so decoding stays length-preserving.
        if width > bytes.len() - index - 1 {
            let _ = write!(output, "{{byte(0x{byte:02x})}}");
            index += 1;
            continue;
        }
        let command = Command::new(byte, &bytes[index + 1..index + 1 + width]);
        output.push('{');
        output.push_str(class.name);
        if !command.operands().is_empty() {
            output.push('(');
            for (position, operand) in command.operands().iter().enumerate() {
                if position > 0 {
                    output.push_str(", ");
                }
                let _ = write!(output, "0x{operand:02x}");
            }
            output.push(')');
        }
        output.push('}');
        index += 1 + width;
    }
    output
}

/// Serialize editable text back into a row extent's bytes.
pub fn serialize(text: &str) -> Result<Vec<u8>> {
    let mut output = Vec::new();
    let characters: Vec<char> = text.chars().collect();
    let mut index = 0;
    while index < characters.len() {
        let character = characters[index];
        match character {
            '{' => {
                let close = characters[index..]
                    .iter()
                    .position(|value| *value == '}')
                    .ok_or_else(|| crate::models::Error::Invalid("unterminated command".into()))?
                    + index;
                let body: String = characters[index + 1..close].iter().collect();
                output.extend_from_slice(&serialize_command(&body)?);
                index = close + 1;
            }
            '}' => return invalid("unescaped '}': an escape or {byte(0x7d)} is required"),
            '\\' => {
                // Escapes are recognised and refused, not silently dropped: the verified
                // character table has no byte for these literals, so an escape could only
                // invent one. The edit fails closed and names the alternative.
                return match characters.get(index + 1).copied() {
                    Some('n') | Some('r') => {
                        invalid("use {nl} for a display line break; a raw line break is not a byte")
                    }
                    Some(c @ ('{' | '}' | '\\')) => invalid(format!(
                        "the verified character table has no byte for a literal {c:?}; use {{byte(0xNN)}}"
                    )),
                    Some(other) => invalid(format!("unknown escape \\{other}")),
                    None => invalid("trailing backslash"),
                };
            }
            '\n' | '\r' | '\t' => {
                return invalid(
                    "raw whitespace is not permitted inside text; use {nl} for a line break",
                );
            }
            _ => {
                let byte = literal_byte(character).ok_or_else(|| {
                    crate::models::Error::Invalid(format!(
                        "character {character:?} has no known encoding"
                    ))
                })?;
                output.push(byte);
                index += 1;
            }
        }
    }
    Ok(output)
}

fn serialize_command(body: &str) -> Result<Vec<u8>> {
    let (name, arguments) = match body.split_once('(') {
        Some((name, rest)) => {
            let rest = rest.strip_suffix(')').ok_or_else(|| {
                crate::models::Error::Invalid(format!("command {name} is missing ')'"))
            })?;
            let values = if rest.trim().is_empty() {
                Vec::new()
            } else {
                rest.split(',')
                    .map(|value| parse_operand(value.trim()))
                    .collect::<Result<Vec<u8>>>()?
            };
            (name, values)
        }
        None => (body, Vec::new()),
    };
    if name == "byte" {
        if arguments.len() != 1 {
            return invalid("{byte} takes exactly one operand");
        }
        // The universal single-byte escape. It is required for commands whose
        // operands are cut off by the extent, so it is accepted for any value.
        return Ok(vec![arguments[0]]);
    }
    let class = command_named(name)
        .ok_or_else(|| crate::models::Error::Invalid(format!("unknown command {{{name}}}")))?;
    let expected = class.width(arguments.first().copied());
    if arguments.len() != expected {
        return invalid(format!(
            "{{{name}}} takes {expected} operand(s), found {}",
            arguments.len()
        ));
    }
    let command = Command::new(class.code, &arguments);
    Ok(command.bytes())
}

fn parse_operand(value: &str) -> Result<u8> {
    let parsed = match value
        .strip_prefix("0x")
        .or_else(|| value.strip_prefix("0X"))
    {
        Some(hex) => u8::from_str_radix(hex, 16),
        None => value.parse::<u8>(),
    };
    parsed.map_err(|_| crate::models::Error::Invalid(format!("invalid operand {value:?}")))
}

/// The command classes a text document actually uses, in first-seen order.
pub fn commands_in(text: &str) -> Vec<&'static CommandDef> {
    let mut found: Vec<&'static CommandDef> = Vec::new();
    let characters: Vec<char> = text.chars().collect();
    let mut index = 0;
    while index < characters.len() {
        if characters[index] != '{' {
            index += 1;
            continue;
        }
        let Some(offset) = characters[index..].iter().position(|value| *value == '}') else {
            break;
        };
        let body: String = characters[index + 1..index + offset].iter().collect();
        let name = body.split('(').next().unwrap_or(&body);
        if let Some(class) = command_named(name) {
            if !found.iter().any(|seen| seen.code == class.code) {
                found.push(class);
            }
        }
        index += offset + 1;
    }
    found
}
