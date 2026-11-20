//! Consume source-bound graphics roles from the texture component.

use std::io::Write;
use std::path::PathBuf;
use std::process::{Command, Stdio};

use serde::{Deserialize, Serialize};

use crate::digest::sha256_hex;
use crate::models::{Entry, Error, Result};

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
struct Identity {
    schema: String,
    interpretation_sha256: String,
    qualification_sha256: String,
}

#[derive(Clone, Debug)]
pub struct Owner {
    binary: PathBuf,
    identity: Identity,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct Response {
    schema: String,
    interpretation_sha256: String,
    qualification_sha256: String,
    source: Option<Source>,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct Source {
    sha256: String,
    size: usize,
    executable_ranges: Vec<Range>,
    entries: Vec<Payload>,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct Payload {
    entry: Entry,
    sha256: String,
    whole_vram_tiles: bool,
    ranges: Vec<Range>,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Range {
    pub offset: usize,
    pub length: usize,
    pub evidence: String,
}

pub struct ArchiveContext {
    pub original_us: bool,
    source: Option<Source>,
}

fn invalid(message: impl Into<String>) -> Error {
    Error::Invalid(format!("texture owner: {}", message.into()))
}

impl Owner {
    pub fn load() -> Result<Self> {
        let binary = std::env::var_os("PSX_BOF3_TEXTURES")
            .map(PathBuf::from)
            .unwrap_or_else(|| PathBuf::from("bof3-textures"));
        let output = Command::new(&binary)
            .args(["ranges", "--identity", "--json"])
            .output()
            .map_err(|error| {
                invalid(format!(
                    "{}: {error}; run through bin/harness text or set PSX_BOF3_TEXTURES",
                    binary.display()
                ))
            })?;
        if !output.status.success() {
            return Err(invalid(String::from_utf8_lossy(&output.stderr)));
        }
        let identity: Identity =
            serde_json::from_slice(&output.stdout).map_err(|error| invalid(error.to_string()))?;
        if identity.schema != "bof3.texture-ranges/v1"
            || identity.qualification_sha256 != sha256_hex(crate::original::DISC.as_bytes())
            || identity.interpretation_sha256.len() != 64
            || !identity
                .interpretation_sha256
                .bytes()
                .all(|byte| byte.is_ascii_hexdigit())
        {
            return Err(invalid(
                "unsupported range protocol or source qualification",
            ));
        }
        Ok(Self { binary, identity })
    }

    pub fn fingerprint(&self) -> Result<String> {
        serde_json::to_vec(&self.identity)
            .map(|bytes| sha256_hex(&bytes))
            .map_err(|error| invalid(error.to_string()))
    }

    fn classify(&self, bytes: &[u8], original_us: bool) -> Result<Source> {
        let mut child = Command::new(&self.binary)
            .args(["ranges", "--stdin", "--json"])
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn()?;
        let written = child.stdin.take().unwrap().write_all(bytes);
        if let Err(error) = written {
            let _ = child.kill();
            child.wait()?;
            return Err(error.into());
        }
        let output = child.wait_with_output()?;
        if !output.status.success() {
            return Err(invalid(String::from_utf8_lossy(&output.stderr)));
        }
        let response: Response =
            serde_json::from_slice(&output.stdout).map_err(|error| invalid(error.to_string()))?;
        if response.schema != self.identity.schema
            || response.interpretation_sha256 != self.identity.interpretation_sha256
            || response.qualification_sha256 != self.identity.qualification_sha256
        {
            return Err(invalid(
                "range interpretation changed during the text operation",
            ));
        }
        let source = response
            .source
            .ok_or_else(|| invalid("original source was not qualified"))?;
        if source.size != bytes.len() || source.sha256 != sha256_hex(bytes) {
            return Err(invalid("range source does not match the current bytes"));
        }
        validate_ranges(&source.executable_ranges, bytes.len())?;
        if original_us {
            let entries = crate::lexagraph::entries(bytes)?;
            if entries.len() != source.entries.len() || !source.executable_ranges.is_empty() {
                return Err(invalid("range entry inventory does not match the archive"));
            }
            for ((entry, payload), supplied) in entries.iter().zip(&source.entries) {
                if entry != &supplied.entry || supplied.sha256 != sha256_hex(payload) {
                    return Err(invalid(
                        "range descriptor or payload does not match the archive",
                    ));
                }
                validate_ranges(&supplied.ranges, payload.len())?;
            }
        } else if !source.entries.is_empty() {
            return Err(invalid(
                "executable range response contains archive entries",
            ));
        }
        Ok(source)
    }
}

impl ArchiveContext {
    pub fn load(bytes: &[u8], owner: Option<&Owner>) -> Result<Self> {
        let original_us = crate::original::archive(bytes)?;
        let source = if let Some(owner) = owner
            && (original_us || sha256_hex(bytes) == crate::original::US_EXECUTABLE_SHA256)
        {
            Some(owner.classify(bytes, original_us)?)
        } else {
            None
        };
        Ok(Self {
            original_us,
            source,
        })
    }

    pub fn whole_vram_tiles(&self, entry: Entry) -> bool {
        self.source
            .as_ref()
            .and_then(|source| source.entries.get(entry.index))
            .is_some_and(|payload| payload.entry == entry && payload.whole_vram_tiles)
    }

    pub fn payload_ranges(&self, entry: Entry) -> &[Range] {
        self.source
            .as_ref()
            .and_then(|source| source.entries.get(entry.index))
            .filter(|payload| payload.entry == entry)
            .map_or(&[], |payload| &payload.ranges)
    }

    pub fn executable_ranges(&self) -> &[Range] {
        self.source
            .as_ref()
            .map_or(&[], |source| &source.executable_ranges)
    }
}

fn validate_ranges(ranges: &[Range], size: usize) -> Result<()> {
    let mut spans = Vec::with_capacity(ranges.len());
    for range in ranges {
        let end = range
            .offset
            .checked_add(range.length)
            .filter(|&end| end <= size && range.length > 0 && !range.evidence.is_empty())
            .ok_or_else(|| invalid("range is empty or outside its source"))?;
        spans.push((range.offset, end));
    }
    spans.sort_unstable();
    if spans.windows(2).any(|pair| pair[0].1 > pair[1].0) {
        return Err(invalid("range response contains overlapping fields"));
    }
    Ok(())
}
