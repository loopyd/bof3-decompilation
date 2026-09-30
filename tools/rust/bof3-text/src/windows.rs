//! The classified-window registry: byte ranges an existing owner already accounts for.
//!
//! Built by `bin/harness text windows` and read here by path, because the tool must not reach into
//! `config/targets` itself. Lookup is by `(archive, entry)` and rebased onto that subfile's payload,
//! which is the only coordinate space the scanner knows.

use std::collections::BTreeMap;
use std::fs;
use std::path::Path;

use serde::Deserialize;

use crate::lexagraph::Window;
use crate::models::{Error, Result};

/// The schema this build understands; a registry claiming another one is refused rather than
/// silently contributing no windows.
pub const WINDOW_SCHEMA: &str = "bof3.text-windows/v1";

#[derive(Debug, Deserialize)]
struct Entry {
    archive: String,
    entry: usize,
    start: usize,
    length: usize,
}

#[derive(Debug, Deserialize)]
struct Inputs {
    #[serde(default)]
    inspected_archives: Vec<String>,
}

#[derive(Debug, Deserialize)]
struct Registry {
    schema: String,
    #[serde(default)]
    inputs: Option<Inputs>,
    windows: Vec<Entry>,
}

/// Windows selected by substream, sorted and merged.
#[derive(Clone, Debug, Default)]
pub struct WindowIndex {
    by_subfile: BTreeMap<(String, usize), Vec<Window>>,
    count: usize,
    /// Digest of the registry file, so an index built under one registry is invalidated when the
    /// registry changes — the ownership rules are an input like any other.
    source_hash: String,
    /// Every archive the ownership source inspected. An archive here with no windows genuinely has
    /// none; an archive absent from here was never inspected, which is a different thing.
    inspected: Vec<String>,
    /// Set when the caller explicitly opted out of the check (`--no-windows`). That is a deliberate
    /// choice, not an unavailable ownership source, so archive validation must not apply.
    disabled: bool,
}

impl WindowIndex {
    /// No windows at all — used when the caller explicitly opts out.
    pub fn none() -> Self {
        Self {
            disabled: true,
            ..Self::default()
        }
    }

    /// Whether the caller explicitly turned the check off.
    pub fn disabled(&self) -> bool {
        self.disabled
    }

    /// Total windows loaded.
    pub fn len(&self) -> usize {
        self.count
    }

    /// Digest of the registry this index was loaded from.
    pub fn source_hash(&self) -> &str {
        &self.source_hash
    }

    /// Whether the ownership source inspected this archive, matching by path suffix like `resolve`.
    pub fn inspected(&self, path: &str) -> bool {
        let normalized = path.replace('\\', "/");
        let components: Vec<&str> = normalized
            .split('/')
            .filter(|part| !part.is_empty())
            .collect();
        for take in 1..=components.len() {
            let suffix = components[components.len() - take..].join("/");
            if self
                .inspected
                .iter()
                .any(|archive| archive.ends_with(&suffix))
            {
                return true;
            }
        }
        false
    }

    /// The key naming this archive, or `None` when it owns no windows.
    ///
    /// The two cases are different and the callers must treat them differently: an inspected archive
    /// with no windows legitimately latches unwindowed, while an archive this source never inspected
    /// is an unavailable ownership source.
    pub fn key_for(&self, path: &str) -> Result<Option<&str>> {
        self.resolve(path)
    }

    /// The registry key naming this archive, matching by path suffix when the caller passes a path.
    ///
    /// Registry keys are corpus-relative (`WORLD00/AREA000.EMI`) while a caller may name the archive
    /// by a longer path **or by its bare file name**; the shortest unique suffix wins and an ambiguous
    /// one is refused rather than guessed, because latching the wrong windows is worse than none.
    pub fn resolve(&self, path: &str) -> Result<Option<&str>> {
        let normalized = path.replace('\\', "/");
        let components: Vec<&str> = normalized
            .split('/')
            .filter(|part| !part.is_empty())
            .collect();
        // The shortest unique suffix wins, but a suffix that matches several archives is **not** an
        // error by itself: a longer suffix may still be unique, which is the whole point of naming the
        // archive by more of its path. Only a name that stays ambiguous at every length is refused.
        let mut ambiguous: Option<(String, usize)> = None;
        for take in 1..=components.len() {
            let suffix = components[components.len() - take..].join("/");
            // Several subfiles of one archive share its name, so the match is on **archives**.
            let mut matches: Vec<&str> = self
                .by_subfile
                .keys()
                .map(|(archive, _)| archive.as_str())
                .filter(|archive| archive.ends_with(&suffix))
                .collect();
            matches.sort_unstable();
            matches.dedup();
            match matches.len() {
                1 => return Ok(Some(matches[0])),
                0 => continue,
                count => ambiguous = Some((suffix, count)),
            }
        }
        match ambiguous {
            Some((suffix, count)) => Err(Error::Invalid(format!(
                "{path}: {count} archives end with {suffix}; name it unambiguously"
            ))),
            None => Ok(None),
        }
    }

    /// The windows of one subfile, merged, or an empty slice.
    pub fn get(&self, archive: &str, entry: usize) -> &[Window] {
        self.by_subfile
            .get(&(archive.to_string(), entry))
            .map(Vec::as_slice)
            .unwrap_or(&[])
    }

    /// Read a registry written by `bin/harness text windows`.
    pub fn load(path: &Path) -> Result<Self> {
        let bytes = fs::read(path)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        let mut digest = crate::textindex::Digest::new();
        digest.update(&bytes);
        let source_hash = digest.hex();
        let text = String::from_utf8(bytes)
            .map_err(|_| Error::Invalid(format!("{}: not valid UTF-8", path.display())))?;
        let registry: Registry = serde_json::from_str(&text)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        if registry.schema != WINDOW_SCHEMA {
            return Err(Error::Invalid(format!(
                "{}: unsupported window schema {}; expected {WINDOW_SCHEMA}",
                path.display(),
                registry.schema
            )));
        }
        let mut by_subfile: BTreeMap<(String, usize), Vec<Window>> = BTreeMap::new();
        let mut count = 0usize;
        for entry in registry.windows {
            if entry.length == 0 {
                continue;
            }
            by_subfile
                .entry((entry.archive, entry.entry))
                .or_default()
                .push((entry.start, entry.length));
            count += 1;
        }
        for windows in by_subfile.values_mut() {
            windows.sort_unstable();
        }
        Ok(WindowIndex {
            by_subfile,
            count,
            source_hash,
            inspected: registry
                .inputs
                .map(|inputs| inputs.inspected_archives)
                .unwrap_or_default(),
            disabled: false,
        })
    }
}
