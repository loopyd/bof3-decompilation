//! The retained corpus text index.
//!
//! One artifact records where text lives across every archive, so a later corpus query is a
//! scan of this file rather than a scan of the disc. Three properties are deliberate:
//!
//! - **Deterministic.** No timestamp, no map iteration order, archives sorted by path and
//!   instances sorted by `(archive, entry, offset)`. Two runs over unchanged inputs must
//!   produce byte-identical output, and that is asserted by the corpus evidence.
//! - **Fresh.** The header carries a digest over every archive's path, size and content, so
//!   [`current_hash`] can prove whether an existing index still describes the inputs. A
//!   content digest is used rather than mtime because mtime alone reported "fresh" for this
//!   repository's other staleness checks while the content had changed.
//! - **Honest about which field is searchable.** [`Instance::reading`] is the
//!   command-transparent reading and is *the* search field; [`Instance::text`] is decoded
//!   text for display only and contains raw token spellings, so searching it would match
//!   command bytes.
//!
//! Banked subfiles contribute their rows; every other subfile contributes the scanner's
//! latches, so a non-text subfile contributes only its text-like runs. An index over one
//! corpus is not a claim about any other build.

use std::collections::BTreeMap;
use std::fs;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::command;
use crate::extract;
use crate::lexagraph;
use crate::models::{Error, Result, TextClass};
use crate::search;

/// Schema identifier of the index artifact.
pub const INDEX_SCHEMA: &str = "bof3.text-index/v1";
/// Index schema version.
pub const INDEX_VERSION: u32 = 1;

/// A non-cryptographic 128-bit FNV-1a digest.
///
/// Freshness detection only: it answers "did the inputs change", not "was this forged". Two
/// lanes with distinct seeds make an accidental collision far less likely than a single
/// 64-bit lane, and it needs no new dependency.
#[derive(Clone, Debug)]
pub struct Digest {
    lanes: [u64; 2],
}

impl Default for Digest {
    fn default() -> Self {
        Self::new()
    }
}

impl Digest {
    const PRIME: u64 = 0x0000_0100_0000_01b3;

    pub fn new() -> Self {
        Digest {
            lanes: [0xcbf2_9ce4_8422_2325, 0x8422_2325_cbf2_9ce4],
        }
    }

    pub fn update(&mut self, bytes: &[u8]) {
        for (lane_index, lane) in self.lanes.iter_mut().enumerate() {
            let mut hash = *lane;
            for byte in bytes {
                hash ^= u64::from(*byte).wrapping_add(lane_index as u64);
                hash = hash.wrapping_mul(Self::PRIME);
            }
            *lane = hash;
        }
    }

    pub fn hex(&self) -> String {
        format!("{:016x}{:016x}", self.lanes[0], self.lanes[1])
    }
}

/// One located text instance.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Instance {
    /// Archive path relative to the index root, with `/` separators.
    pub archive: String,
    /// TOC entry index within the archive.
    pub entry: usize,
    /// TOC load argument, as `0x…`.
    pub address: String,
    /// Text class.
    pub class: TextClass,
    /// Bank index, for a banked row.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub bank: Option<usize>,
    /// One-based slot number within the bank, for a banked row.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub number: Option<usize>,
    /// Absolute byte offset of the instance in the archive file.
    pub offset: usize,
    /// Instance length in bytes.
    pub length: usize,
    /// **The search field**: the command-transparent reading, whitespace collapsed.
    ///
    /// This is the only text stored. The decoded form with its `{token}` spellings is
    /// deliberately **not** stored: it would triple the artifact (one 54 KB row of `{end}`
    /// padding alone) and, worse, it is the one form that must never be searched. A hit is
    /// rendered by re-reading that single archive, not by keeping the tokens here.
    pub reading: String,
    /// The second reading (commands dropped), stored **only when it differs** from
    /// [`Instance::reading`]. Without it an index query would silently miss a word a command
    /// splits with no displayed break, while a per-archive query finds it — a recall gap
    /// hiding behind a consistent-looking surface.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub joined: Option<String>,
    /// Most frequent Unicode script of the literal text, when there is any.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub script: Option<String>,
    /// `whatlang`'s ISO 639-3 guess, recorded **only when it reported the sample reliable**.
    /// An unreliable guess is noise — the class-discovery evidence shows the model answering
    /// `cym` for punctuation filler — so it is dropped rather than stored as if it meant
    /// something.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub language: Option<String>,
}

/// One archive's cheap identity: path, size and modification time.
///
/// This exists so a corpus query can notice that the inputs moved without reading 260 MB on
/// every query. It is a **heuristic**: a content change that preserves both size and mtime is
/// invisible to it. The content digest in [`Inputs::hash`] remains the authority and is
/// verified by `build-index --check` and by `search --verify`.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct ArchiveStamp {
    /// Archive path relative to the index root.
    pub path: String,
    pub size: u64,
    /// Modification time in whole seconds since the Unix epoch, when the platform reports it.
    pub mtime: Option<u64>,
}

/// What the index was built from.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Inputs {
    /// Directory that was walked.
    pub root: String,
    /// Archives found.
    pub archives: usize,
    /// Total archive bytes read.
    pub bytes: u64,
    /// Digest over every archive's relative path, size and content.
    pub hash: String,
    /// Digest of the window registry the index was built under; a changed registry invalidates it.
    #[serde(default)]
    pub windows_hash: String,
    /// Selection policy, payload inventory, vocabulary and filter opt-outs.
    /// Older indexes without this binding must be rebuilt before use.
    #[serde(default)]
    pub filters_hash: String,
    /// Identified per-window removals, each stamped with its archive and entry.
    #[serde(default)]
    pub windows_detail: Vec<crate::lexagraph::WindowDetail>,
    /// What the classified-window check removed while latching. Merged barriers are counted, so this
    /// is one per banked subfile plus one per reviewed range that applied.
    pub windows: crate::lexagraph::Tally,
    /// Cheap stat identity of each archive, for a fast freshness check.
    pub manifest: Vec<ArchiveStamp>,
}

/// The retained index artifact.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Index {
    pub schema: String,
    pub version: u32,
    /// Producing tool and version, for review.
    pub tool: String,
    /// No generation timestamp is stored: it would make repeat runs differ.
    pub inputs: Inputs,
    /// Instance count per class, class name to count.
    pub counts: BTreeMap<String, usize>,
    /// Subfiles of a known class that failed to parse, as `archive#entry`, so a gap is
    /// visible instead of silent.
    pub skipped: Vec<String>,
    pub instances: Vec<Instance>,
}

impl Index {
    /// Whether archive bytes and every index-building filter still agree.
    pub fn is_fresh(&self, current: &Inputs, filters_hash: &str) -> bool {
        self.inputs.hash == current.hash && self.inputs.filters_hash == filters_hash
    }
}

/// An archive path relative to the root, with `/` separators.
pub fn relative_archive(root: &Path, path: &Path) -> String {
    relative_name(root, path)
}

fn relative_name(root: &Path, path: &Path) -> String {
    path.strip_prefix(root)
        .unwrap_or(path)
        .to_string_lossy()
        .replace('\\', "/")
}

/// The cheap stat identity of one archive.
fn stamp(root: &Path, path: &Path) -> Result<ArchiveStamp> {
    let metadata = fs::metadata(path)
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
    let mtime = metadata
        .modified()
        .ok()
        .and_then(|time| time.duration_since(std::time::UNIX_EPOCH).ok())
        .map(|elapsed| elapsed.as_secs());
    Ok(ArchiveStamp {
        path: relative_name(root, path),
        size: metadata.len(),
        mtime,
    })
}

/// How a stored manifest differs from the current one, as human-readable lines. Empty means
/// they agree on every path, size and modification time.
pub fn manifest_diff(
    stored: &[ArchiveStamp],
    current: &[ArchiveStamp],
    limit: usize,
) -> Vec<String> {
    let mut differences = Vec::new();
    for (before, now) in stored.iter().zip(current.iter()) {
        if before != now {
            let mut parts = Vec::new();
            if before.size != now.size {
                parts.push(format!("size {} -> {}", before.size, now.size));
            }
            if before.mtime != now.mtime {
                parts.push(format!("mtime {:?} -> {:?}", before.mtime, now.mtime));
            }
            if before.path != now.path {
                parts.push(format!("path {} -> {}", before.path, now.path));
            }
            differences.push(format!("{}: {}", now.path, parts.join(", ")));
        }
        if differences.len() >= limit {
            return differences;
        }
    }
    if stored.len() != current.len() {
        differences.push(format!(
            "{} archive(s) indexed, {} present now",
            stored.len(),
            current.len()
        ));
    }
    differences
}

fn absorb(digest: &mut Digest, root: &Path, path: &Path, data: &[u8]) {
    digest.update(relative_name(root, path).as_bytes());
    digest.update(&(data.len() as u64).to_le_bytes());
    digest.update(data);
}

/// Every `*.EMI` under `root`, sorted by path.
pub fn archives(root: &Path) -> Result<Vec<PathBuf>> {
    if !root.is_dir() {
        return Err(Error::Invalid(format!(
            "index root {} is not a directory",
            root.display()
        )));
    }
    let mut found = Vec::new();
    let mut stack = vec![root.to_path_buf()];
    while let Some(directory) = stack.pop() {
        let entries = fs::read_dir(&directory)
            .map_err(|error| Error::Invalid(format!("{}: {error}", directory.display())))?;
        for entry in entries {
            let entry = entry.map_err(|error| Error::Invalid(error.to_string()))?;
            let path = entry.path();
            if path.is_dir() {
                stack.push(path);
            } else if path
                .extension()
                .is_some_and(|value| value.eq_ignore_ascii_case("EMI"))
            {
                found.push(path);
            }
        }
    }
    found.sort();
    Ok(found)
}

/// The two readings of an instance: the primary one, and the joined one only when it differs.
fn readings(encoded: &[u8]) -> (String, Option<String>) {
    let (spaced, joined) = search::readings(encoded);
    let joined = (joined.text != spaced.text).then_some(joined.text);
    (spaced.text, joined)
}

/// The compact signal stored per instance: dominant script, and a language only when the
/// detector called its own guess reliable.
fn signal(encoded: &[u8]) -> (Option<String>, Option<String>) {
    let (scripts, language) = lexagraph::script_profile(&command::deserialize(encoded));
    (
        scripts.first().map(|share| share.script.clone()),
        language
            .filter(|guess| guess.reliable)
            .map(|guess| guess.language),
    )
}

fn collect(
    archive: &str,
    data: &[u8],
    out: &mut Vec<Instance>,
    skipped: &mut Vec<String>,
    filters: &crate::filters::ScanFilters<'_>,
) -> Result<(crate::lexagraph::Tally, Vec<crate::lexagraph::WindowDetail>)> {
    let mut tally = crate::lexagraph::Tally::default();
    let mut details: Vec<crate::lexagraph::WindowDetail> = Vec::new();
    let mut preceding: Vec<&[u8]> = Vec::new();
    for (entry, payload) in lexagraph::entries(data)? {
        let address = format!("{:#010x}", entry.ram_ptr);
        match TextClass::from_load_argument(entry.ram_ptr) {
            Some(class) => {
                let section = match extract::parse_section(payload) {
                    Ok(section) => section,
                    Err(error) => {
                        skipped.push(format!("{archive}#{}: {error}", entry.index));
                        continue;
                    }
                };
                for bank in &section.banks {
                    for row in &bank.rows {
                        let Some((start, end)) = row.span else {
                            continue;
                        };
                        let (script, language) = signal(&row.encoded);
                        let (reading, joined) = readings(&row.encoded);
                        out.push(Instance {
                            archive: archive.to_string(),
                            entry: entry.index,
                            address: address.clone(),
                            class,
                            bank: Some(bank.index),
                            number: Some(row.number),
                            // A row span is relative to its bank, and a two-bank
                            // subfile's bank 1 starts after the 8-byte header, so the
                            // bank's own start must be added: without it every bank-1
                            // offset is short and the entry cannot be resolved.
                            offset: entry.offset + bank.start + start,
                            length: end - start,
                            reading,
                            joined,
                            script,
                            language,
                        });
                    }
                }
            }
            None => {
                // No structural class: record the scanner's latches rather than a whole
                // subfile, so a non-text subfile contributes only its text-like runs.
                let input = crate::models::PreFilterInput {
                    archive,
                    entry: entry.index,
                    payload,
                    // This branch is the one taken when no structural class claims the subfile.
                    class: None,
                    // The container's earlier payloads, so the pairing rule works on this path too.
                    preceding: &preceding,
                };
                let (latches, subfile) = filters.scan_bytes(&input, lexagraph::MIN_RUN);
                preceding.push(payload);
                tally.add(&subfile);
                details.extend(crate::lexagraph::details(archive, entry.index, &subfile));
                for latch in latches {
                    let span = &payload[latch.offset..latch.offset + latch.length];
                    let (script, language) = signal(span);
                    let (reading, joined) = readings(span);
                    out.push(Instance {
                        archive: archive.to_string(),
                        entry: entry.index,
                        address: address.clone(),
                        class: TextClass::Heuristic,
                        bank: None,
                        number: None,
                        offset: entry.offset + latch.offset,
                        length: latch.length,
                        reading,
                        joined,
                        script,
                        language,
                    });
                }
            }
        }
    }
    Ok((tally, details))
}

/// Build the index over every archive under `root`.
pub fn build(
    root: &Path,
    windows: &crate::windows::WindowIndex,
    payloads: &crate::filters::KnownPayload,
    vocabulary: &crate::filters::Vocabulary,
    readability: crate::filters::Readability,
) -> Result<Index> {
    let filters = crate::filters::compose(windows, payloads, vocabulary, readability);
    let paths = archives(root)?;
    let mut digest = Digest::new();
    let mut manifest = Vec::with_capacity(paths.len());
    let mut instances = Vec::new();
    let mut skipped = Vec::new();
    let mut tally = crate::lexagraph::Tally::default();
    let mut windows_detail: Vec<crate::lexagraph::WindowDetail> = Vec::new();
    let mut bytes = 0u64;
    for path in &paths {
        let data = fs::read(path)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        bytes += data.len() as u64;
        absorb(&mut digest, root, path, &data);
        manifest.push(stamp(root, path)?);
        let archive = relative_name(root, path);
        let (subfile, detail) = collect(&archive, &data, &mut instances, &mut skipped, &filters)?;
        tally.add(&subfile);
        windows_detail.extend(detail);
    }
    instances.sort_by(|left, right| {
        (&left.archive, left.entry, left.offset).cmp(&(&right.archive, right.entry, right.offset))
    });
    let mut counts: BTreeMap<String, usize> = BTreeMap::new();
    for instance in &instances {
        *counts.entry(instance.class.name().to_string()).or_default() += 1;
    }
    skipped.sort();
    Ok(Index {
        schema: INDEX_SCHEMA.to_string(),
        version: INDEX_VERSION,
        tool: format!("bof3-text {}", env!("CARGO_PKG_VERSION")),
        inputs: Inputs {
            root: root.display().to_string(),
            archives: paths.len(),
            bytes,
            hash: digest.hex(),
            manifest,
            windows_hash: windows.source_hash().to_string(),
            filters_hash: filters.fingerprint()?,
            windows_detail,
            windows: tally,
        },
        counts,
        skipped,
        instances,
    })
}

/// What the current inputs hash to, without building anything.
pub fn current_hash(root: &Path) -> Result<Inputs> {
    let paths = archives(root)?;
    let mut digest = Digest::new();
    let mut bytes = 0u64;
    for path in &paths {
        let data = fs::read(path)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        bytes += data.len() as u64;
        absorb(&mut digest, root, path, &data);
    }
    Ok(Inputs {
        root: root.display().to_string(),
        archives: paths.len(),
        bytes,
        hash: digest.hex(),
        manifest: paths
            .iter()
            .map(|path| stamp(root, path))
            .collect::<Result<_>>()?,
        windows_hash: String::new(),
        filters_hash: String::new(),
        windows_detail: Vec::new(),
        windows: crate::lexagraph::Tally::default(),
    })
}

/// Write the index. Compact JSON, so the artifact stays reviewable without being huge.
pub fn write(index: &Index, path: &Path) -> Result<()> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|error| Error::Invalid(format!("{}: {error}", parent.display())))?;
    }
    let body = serde_json::to_string(index)
        .map_err(|error| Error::Invalid(format!("cannot serialize index: {error}")))?;
    fs::write(path, format!("{body}\n"))
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
    Ok(())
}

/// Load an index, refusing a schema or version this build does not understand.
pub fn load(path: &Path) -> Result<Index> {
    let text = fs::read_to_string(path)
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
    let index: Index = serde_json::from_str(&text)
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
    if index.schema != INDEX_SCHEMA {
        return Err(Error::Invalid(format!(
            "unsupported index schema {}; expected {INDEX_SCHEMA}",
            index.schema
        )));
    }
    if index.version != INDEX_VERSION {
        return Err(Error::Invalid(format!(
            "unsupported index version {}; expected {INDEX_VERSION}",
            index.version
        )));
    }
    Ok(index)
}
