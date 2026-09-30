//! Direct-media inventory. Physical identities do not imply runtime associations.

pub mod content;
pub mod loader;
pub mod model;
pub mod music;

use crate::catalog::model::{Asset, AssetData, Catalog, Entry, SectorRange, Source, XaStream};
use crate::{archive::MediaImage, bank::Bank, digest::sha256_hex, sequence::SequenceSet, Result};
use std::{
    collections::{BTreeMap, BTreeSet},
    path::{Path, PathBuf},
};

impl Catalog {
    /// Inventory an XML-preserved source without reopening its historical path.
    pub fn from_media(source: &str, media: MediaImage) -> Result<Self> {
        let mut catalog = Self::empty();
        catalog.append(source, media)?;
        Ok(catalog)
    }

    fn empty() -> Self {
        Self { schema: "bof3.audio-catalog/v1", sources: Vec::new(), assets: Vec::new(), runtime_mapping: None,
            unresolved: vec![
                "game bank/song IDs and bank-to-body/sequence associations require runtime evidence; candidates are not resolved links",
                "SFX/vocal classification is unresolved; XA cue extents require a verified executable and known media",
                "shared header hashes show byte-identical headers, not necessarily identical sample bodies",
                "sequence event semantics and sample playback are not validated by inventory",
            ] }
    }

    pub fn select_cue(&self, cue_id: u16) -> Result<&Asset> {
        let mapping = self
            .runtime_mapping
            .as_ref()
            .ok_or("cue selection requires --executable")?;
        let cue = mapping
            .music
            .cues
            .iter()
            .find(|cue| cue.game_song_id == cue_id)
            .ok_or_else(|| format!("cue {cue_id}: outside verified US music cues 0..164"))?;
        match cue.sequence_assets.as_slice() {
            [id] => self
                .assets
                .iter()
                .find(|asset| &asset.id == id)
                .ok_or_else(|| "mapped cue asset is absent from inventory".into()),
            [] => Err(format!(
                "cue {cue_id}: {}; original {} is required for content-based identity",
                cue.resolution, cue.disc_path
            )
            .into()),
            ids => Err(format!(
                "cue {cue_id}: ambiguous supplied sources; use --id with one of {}",
                ids.join(", ")
            )
            .into()),
        }
    }

    pub fn read(root: Option<&Path>, archives: &[PathBuf]) -> Result<Self> {
        if root.is_some() == !archives.is_empty() {
            return Err("provide either --disc-root or one or more --archive paths".into());
        }
        let mut paths = Vec::new();
        if let Some(root) = root {
            if !root.is_dir() {
                return Err("disc root must be a directory".into());
            }
            discover(root, &mut paths)?;
        } else {
            paths.extend_from_slice(archives);
        }
        paths.sort();
        if paths.is_empty() {
            return Err("no EMI/STR/XA media found".into());
        }
        let mut seen = BTreeSet::new();
        let mut catalog = Self::empty();
        for path in paths {
            let canonical = path.canonicalize()?;
            if !seen.insert(canonical.clone()) {
                return Err(format!("duplicate input media: {}", path.display()).into());
            }
            let identity_path = if let Some(root) = root {
                path.strip_prefix(root)?
            } else {
                &canonical
            };
            let source = identity_path
                .to_str()
                .ok_or("media identity path must be UTF-8")?
                .to_owned();
            let media = MediaImage::read(&path).map_err(|e| format!("{}: {e}", path.display()))?;
            catalog
                .append(&source, media)
                .map_err(|e| format!("{source}: {e}"))?;
        }
        Ok(catalog)
    }

    pub fn select(&self, mode: &str, kind: Option<&str>, id: &str) -> Result<&Asset> {
        let numeric = id.strip_prefix("0x").map_or_else(
            || id.parse::<usize>().ok(),
            |hex| usize::from_str_radix(hex, 16).ok(),
        );
        let matches: Vec<_> = self
            .assets
            .iter()
            .filter(|asset| {
                asset.matches_mode(mode)
                    && kind.is_none_or(|kind| asset.kind() == kind)
                    && (asset.id == id
                        || numeric.is_some_and(|number| asset.numeric_id() == Some(number)))
            })
            .collect();
        match matches.as_slice() {
            [asset] => Ok(asset),
            [] => Err(format!("no asset matches {id:?} in selected mode/kind").into()),
            _ => Err(format!(
                "ambiguous ID {id:?}: {} matches; use a source-qualified ID, e.g. {}",
                matches.len(),
                matches
                    .iter()
                    .take(5)
                    .map(|a| a.id.as_str())
                    .collect::<Vec<_>>()
                    .join(", ")
            )
            .into()),
        }
    }

    fn append(&mut self, source: &str, media: MediaImage) -> Result<()> {
        let mut inventory = Source {
            source: source.into(),
            bytes: media.bytes().len(),
            container: "emi",
            entries: Vec::new(),
            sha256: Some(sha256_hex(media.bytes())),
        };
        match media {
            MediaImage::Emi(image) => {
                for (entry, data) in image.entries().iter().enumerate() {
                    inventory.entries.push(Entry {
                        id: entry_id(source, entry),
                        entry,
                        file_type: data.file_type,
                        offset: data.offset,
                        bytes: data.size,
                        load_argument: data.ram_ptr,
                        body_sha256: if data.file_type == 7 {
                            Some(sha256_hex(image.entry(entry)?))
                        } else {
                            None
                        },
                    });
                }
                let candidates = |file_type| {
                    inventory
                        .entries
                        .iter()
                        .filter(|e| e.file_type == file_type)
                        .map(|e| e.id.clone())
                        .collect::<Vec<_>>()
                };
                for entry in &inventory.entries {
                    let bytes = image.entry(entry.entry)?;
                    match entry.file_type {
                        6 => {
                            let bank = Bank::parse(bytes)
                                .map_err(|e| format!("entry {}: {e}", entry.entry))?;
                            for sample in &bank.samples {
                                self.push(
                                    source,
                                    Some(entry.entry),
                                    format!("{}/sample={}", entry.id, sample.sample_id),
                                    AssetData::Sample {
                                        bank: entry.id.clone(),
                                        metadata: sample.clone(),
                                    },
                                );
                            }
                            self.push(
                                source,
                                Some(entry.entry),
                                entry.id.clone(),
                                AssetData::Bank {
                                    header_sha256: sha256_hex(bytes),
                                    body_candidates: candidates(7),
                                    content: None,
                                    metadata: bank,
                                },
                            );
                        }
                        9 | 10 => {
                            let song = SequenceSet::parse(bytes)
                                .map_err(|e| format!("entry {}: {e}", entry.entry))?;
                            for sequence in &song.sequences {
                                self.push(
                                    source,
                                    Some(entry.entry),
                                    format!("{}/sequence={}", entry.id, sequence.sequence_index),
                                    AssetData::Sequence {
                                        song: entry.id.clone(),
                                        metadata: sequence.clone(),
                                    },
                                );
                            }
                            self.push(
                                source,
                                Some(entry.entry),
                                entry.id.clone(),
                                AssetData::Song {
                                    bank_candidates: candidates(6),
                                    metadata: song,
                                },
                            );
                        }
                        _ => {}
                    }
                }
            }
            MediaImage::Xa(image) => {
                inventory.container = "xa";
                let content_type = "unresolved";
                let mut streams = BTreeMap::new();
                for sector in image.sectors().iter().filter(|s| s.is_audio()) {
                    let stream = streams
                        .entry((sector.file, sector.channel, sector.coding))
                        .or_insert_with(|| XaStream {
                            file: sector.file,
                            channel: sector.channel,
                            coding: sector.coding,
                            channels: match sector.coding & 3 {
                                0 => Some(1),
                                1 => Some(2),
                                _ => None,
                            },
                            sample_rate: match sector.coding >> 2 & 3 {
                                0 => Some(37800),
                                1 => Some(18900),
                                _ => None,
                            },
                            bits_per_sample: match sector.coding >> 4 & 3 {
                                0 => Some(4),
                                1 => Some(8),
                                _ => None,
                            },
                            sector_ranges: Vec::new(),
                            eof_sectors: Vec::new(),
                            cue_boundaries: "unresolved",
                        });
                    if let Some(last) = stream
                        .sector_ranges
                        .last_mut()
                        .filter(|r| r.end_exclusive == sector.index)
                    {
                        last.end_exclusive += 1;
                    } else {
                        stream.sector_ranges.push(SectorRange {
                            start: sector.index,
                            end_exclusive: sector.index + 1,
                        });
                    }
                    if sector.submode & 0x80 != 0 {
                        stream.eof_sectors.push(sector.index);
                    }
                }
                for stream in streams.into_values() {
                    let first = stream.sector_ranges.first().unwrap().start;
                    let end = stream.sector_ranges.last().unwrap().end_exclusive;
                    let id = format!(
                        "{}#stream={}/channel={}/coding={:02x}/sectors={first}..{end}",
                        escape_source(source),
                        stream.file,
                        stream.channel,
                        stream.coding
                    );
                    self.push(source, None, id, AssetData::XaStream(stream));
                    self.assets.last_mut().unwrap().content_type = content_type;
                }
            }
        }
        self.sources.push(inventory);
        Ok(())
    }

    fn push(&mut self, source: &str, entry: Option<usize>, id: String, data: AssetData) {
        self.assets.push(Asset {
            id,
            source: source.into(),
            entry,
            game_bank_id: None,
            game_song_ids: Vec::new(),
            content_type: "unresolved",
            data,
        });
    }
}

fn entry_id(source: &str, entry: usize) -> String {
    format!("{}#entry={entry}", escape_source(source))
}

pub(crate) fn escape_source(source: &str) -> String {
    source.replace('%', "%25").replace('#', "%23")
}

fn discover(root: &Path, paths: &mut Vec<PathBuf>) -> Result<()> {
    for entry in std::fs::read_dir(root)? {
        let entry = entry?;
        let kind = entry.file_type()?;
        if kind.is_dir() {
            discover(&entry.path(), paths)?;
        } else if kind.is_symlink() {
            return Err(format!(
                "disc-root traversal does not follow symlinks: {}",
                entry.path().display()
            )
            .into());
        } else if kind.is_file()
            && entry
                .path()
                .extension()
                .and_then(|v| v.to_str())
                .is_some_and(|ext| {
                    ["emi", "str", "xa"]
                        .iter()
                        .any(|supported| ext.eq_ignore_ascii_case(supported))
                })
        {
            paths.push(entry.path());
        }
    }
    Ok(())
}
