//! Bounded XML manifest reading and preservation/path validation for packing.

use crate::{digest::sha256_hex, Result};
use roxmltree::{Document, ParsingOptions};
use serde::Serialize;
use std::{
    collections::BTreeMap,
    fs,
    io::Read,
    path::{Path, PathBuf},
};

#[derive(Clone, Copy)]
pub struct Limits {
    pub bytes: usize,
    pub nodes: u32,
    pub depth: usize,
}

impl Default for Limits {
    fn default() -> Self {
        Self {
            // Complete S_XA00.STR is 94,411,776 bytes before hexadecimal XML.
            bytes: 256 * 1024 * 1024,
            nodes: 200_000,
            depth: 64,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Element {
    pub name: String,
    pub attributes: BTreeMap<String, String>,
    pub children: Vec<Element>,
    pub text: String,
}

impl Element {
    pub fn attribute(&self, name: &str) -> Result<&str> {
        self.attributes
            .get(name)
            .map(String::as_str)
            .ok_or_else(|| format!("manifest <{}>: missing attribute {name}", self.name).into())
    }

    pub fn number<T: std::str::FromStr>(&self, name: &str) -> Result<T> {
        self.attribute(name)?.parse().map_err(|_| {
            format!("manifest <{}>: invalid numeric attribute {name}", self.name).into()
        })
    }

    pub fn child(&self, name: &str) -> Result<&Element> {
        let mut nodes = self.children.iter().filter(|n| n.name == name);
        let node = nodes
            .next()
            .ok_or_else(|| format!("manifest <{}>: missing <{name}>", self.name))?;
        if nodes.next().is_some() {
            return Err(format!("manifest <{}>: duplicate <{name}>", self.name).into());
        }
        Ok(node)
    }

    pub fn shape(&self, attributes: &[&str], children: &[&str], text: bool) -> Result<()> {
        for key in self.attributes.keys() {
            if !attributes.contains(&key.as_str()) {
                return Err(
                    format!("manifest <{}>: unsupported attribute {key}", self.name).into(),
                );
            }
        }
        for child in &self.children {
            if !children.contains(&child.name.as_str()) {
                return Err(format!(
                    "manifest <{}>: unsupported child <{}>",
                    self.name, child.name
                )
                .into());
            }
        }
        if !text && !self.text.trim_matches([' ', '\t', '\n', '\r']).is_empty() {
            return Err(format!("manifest <{}>: unexpected text", self.name).into());
        }
        Ok(())
    }

    /// Compare parsed scalar metadata to authoritative source fields. Numeric
    /// spelling may change; altered or missing values cannot be silently ignored.
    pub fn scalars(
        &self,
        expected: &impl Serialize,
        nested: &[&str],
        extras: &[&str],
    ) -> Result<()> {
        let value = serde_json::to_value(expected)?;
        let fields = value
            .as_object()
            .ok_or("manifest scalar reference must be an object")?;
        for key in self.attributes.keys() {
            if !fields.contains_key(key) && !extras.contains(&key.as_str()) {
                return Err(
                    format!("manifest <{}>: unsupported attribute {key}", self.name).into(),
                );
            }
            if nested.contains(&key.as_str()) {
                return Err(
                    format!("manifest <{}>: {key} must be child elements", self.name).into(),
                );
            }
        }
        for (key, expected) in fields {
            if nested.contains(&key.as_str()) {
                continue;
            }
            let actual = self.attribute(key)?;
            let equal = match expected {
                serde_json::Value::String(s) => actual == s,
                serde_json::Value::Bool(b) => actual.parse::<bool>().ok() == Some(*b),
                serde_json::Value::Number(n) if n.is_u64() => {
                    actual.parse::<u64>().ok() == n.as_u64()
                }
                serde_json::Value::Number(n) if n.is_i64() => {
                    actual.parse::<i64>().ok() == n.as_i64()
                }
                _ => {
                    return Err(format!(
                        "manifest <{}>: field {key} needs explicit handling",
                        self.name
                    )
                    .into())
                }
            };
            if !equal {
                return Err(format!("manifest <{}>: edited or conflicting {key}: expected {expected}, found {actual:?}; this transformation is unsupported", self.name).into());
            }
        }
        Ok(())
    }
}

pub fn parse(text: &str, limits: Limits) -> Result<Element> {
    if text.len() > limits.bytes {
        return Err("manifest exceeds XML byte limit".into());
    }
    strict_references(text)?;
    let doc = Document::parse_with_options(
        text,
        ParsingOptions {
            allow_dtd: false,
            nodes_limit: limits.nodes,
            entity_resolver: None,
        },
    )
    .map_err(|e| format!("manifest XML: {e}"))?;
    if doc.descendants().any(|n| n.is_pi()) {
        return Err("manifest processing instructions are unsupported".into());
    }
    element(doc.root_element(), 1, limits.depth)
}

// roxmltree 0.21.1 replaces out-of-Unicode-range numeric references with U+FFFD
// and ignores some declaration details. Those substitutions are not acceptable
// for manifest identities. This lexical guard adds strictness, not XML parsing.
fn strict_references(text: &str) -> Result<()> {
    let start = text.trim_start_matches('\u{feff}');
    if start.starts_with("<?xml ")
        || start.starts_with("<?xml\t")
        || start.starts_with("<?xml\r")
        || start.starts_with("<?xml\n")
    {
        let end = start
            .find("?>")
            .ok_or("manifest has an unterminated XML declaration")?;
        let wrapper = format!("<declaration{}/>", &start[5..end]);
        let declaration = Document::parse(&wrapper)?;
        let root = declaration.root_element();
        if root.attribute("version") != Some("1.0")
            || root
                .attribute("encoding")
                .is_some_and(|e| !e.eq_ignore_ascii_case("UTF-8"))
            || root
                .attribute("standalone")
                .is_some_and(|s| !matches!(s, "yes" | "no"))
            || root
                .attributes()
                .any(|a| !matches!(a.name(), "version" | "encoding" | "standalone"))
        {
            return Err("manifest requires XML 1.0 with UTF-8 encoding".into());
        }
    }
    let bytes = text.as_bytes();
    let mut at = 0;
    while at < bytes.len() {
        let rest = &bytes[at..];
        let skipped = if rest.starts_with(b"<!--") {
            Some((4, b"-->".as_slice()))
        } else if rest.starts_with(b"<![CDATA[") {
            Some((9, b"]]>".as_slice()))
        } else if rest.starts_with(b"<?") {
            Some((2, b"?>".as_slice()))
        } else {
            None
        };
        if let Some((prefix, end)) = skipped {
            if let Some(len) = rest[prefix..].windows(end.len()).position(|w| w == end) {
                at += prefix + len + end.len();
                continue;
            }
            break; // The parser diagnoses unterminated constructs.
        }
        if rest.starts_with(b"<!DOCTYPE") {
            return Err("manifest DTD declarations are unsupported".into());
        }
        if rest.starts_with(b"&#") {
            let end = rest
                .iter()
                .position(|&b| b == b';')
                .ok_or("unterminated manifest character reference")?;
            let digits = std::str::from_utf8(&rest[2..end])?;
            let value = if let Some(hex) = digits.strip_prefix('x') {
                u32::from_str_radix(hex, 16)
            } else {
                digits.parse::<u32>()
            }
            .map_err(|_| "invalid manifest character reference")?;
            if !matches!(value, 9 | 10 | 13 | 0x20..=0xd7ff | 0xe000..=0xfffd | 0x10000..=0x10ffff)
            {
                return Err(format!(
                    "manifest character reference U+{value:04X} is invalid in XML 1.0"
                )
                .into());
            }
            at += end + 1;
        } else {
            at += 1;
        }
    }
    Ok(())
}

fn element(node: roxmltree::Node<'_, '_>, depth: usize, limit: usize) -> Result<Element> {
    if depth > limit {
        return Err("manifest exceeds XML element-depth limit".into());
    }
    if node.tag_name().namespace().is_some()
        || node.namespaces().len() != 0
        || node.attributes().any(|a| a.namespace().is_some())
    {
        return Err("manifest namespaces are unsupported by the current extraction schemas".into());
    }
    let mut result = Element {
        name: node.tag_name().name().into(),
        attributes: node
            .attributes()
            .map(|a| (a.name().into(), a.value().into()))
            .collect(),
        children: Vec::new(),
        text: String::new(),
    };
    for child in node.children() {
        if child.is_element() {
            result.children.push(element(child, depth + 1, limit)?);
        } else if let Some(text) = child.text().filter(|_| child.is_text()) {
            result.text.push_str(text);
        }
    }
    Ok(result)
}

pub fn read(path: &Path) -> Result<Element> {
    let limits = Limits::default();
    if fs::metadata(path)?.len() > limits.bytes as u64 {
        return Err("manifest exceeds XML byte limit".into());
    }
    let mut text = String::new();
    fs::File::open(path)?
        .take(limits.bytes as u64 + 1)
        .read_to_string(&mut text)?;
    parse(&text, limits)
}

pub fn preservation(node: &Element, scope: Option<&str>) -> Result<Vec<u8>> {
    node.shape(
        if scope.is_some() {
            &["encoding", "scope", "bytes", "sha256"]
        } else {
            &["encoding", "bytes", "sha256"]
        },
        &[],
        true,
    )?;
    if node.name != "preservation" || node.attribute("encoding")? != "hex" {
        return Err("manifest preservation requires hexadecimal encoding".into());
    }
    if let Some(scope) = scope {
        if node.attribute("scope")? != scope {
            return Err("manifest preservation scope mismatch".into());
        }
    }
    let count = node.number::<usize>("bytes")?;
    let mut bytes = Vec::new();
    let mut high = None;
    for c in node.text.bytes().filter(|c| !c.is_ascii_whitespace()) {
        let nibble = match c {
            b'0'..=b'9' => c - b'0',
            b'a'..=b'f' => c - b'a' + 10,
            b'A'..=b'F' => c - b'A' + 10,
            _ => return Err("manifest preservation contains non-hexadecimal data".into()),
        };
        if let Some(h) = high.take() {
            bytes.push(h * 16 + nibble);
        } else {
            high = Some(nibble);
        }
    }
    if high.is_some() || bytes.len() != count {
        return Err("manifest preservation byte count mismatch".into());
    }
    if !sha256_hex(&bytes).eq_ignore_ascii_case(node.attribute("sha256")?) {
        return Err("manifest preservation SHA-256 mismatch".into());
    }
    Ok(bytes)
}

/// Resolve an existing regular input under the extraction root. Canonical paths
/// catch symlink escapes; output paths must be constructed independently.
pub fn relative_file(root: &Path, base: &Path, reference: &str) -> Result<PathBuf> {
    let path = Path::new(reference);
    if reference.is_empty()
        || path.is_absolute()
        || reference.contains('\\')
        || reference.contains(':')
    {
        return Err(format!("manifest path must be relative: {reference:?}").into());
    }
    let root = root.canonicalize()?;
    let resolved = base.join(path).canonicalize()?;
    if !resolved.starts_with(&root) || !resolved.is_file() {
        return Err(format!(
            "manifest path escapes extraction root or is not a regular file: {reference:?}"
        )
        .into());
    }
    Ok(resolved)
}
