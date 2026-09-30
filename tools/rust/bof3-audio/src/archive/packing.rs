//! Verify and stage fixed-allocation EMI replacements for bank and music packing.
use crate::{digest::sha256_hex, publication::write_new, Result};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use std::{collections::BTreeMap, fs, path::Path};

#[derive(Debug, Serialize)]
pub struct ArchiveReport {
    pub source: String,
    pub path: String,
    pub source_sha256: String,
    pub output_sha256: String,
    pub byte_equal: bool,
    pub changed_entries: Vec<usize>,
}

pub(crate) struct Pending {
    pub image: ArchiveImage,
    pub entries: BTreeMap<usize, Vec<u8>>,
}

pub(crate) fn stage(
    staging: &Path,
    pending: BTreeMap<String, Pending>,
) -> Result<Vec<ArchiveReport>> {
    let mut reports = Vec::new();
    fs::create_dir(staging.join("archives"))?;
    for (source, pending) in pending {
        let replacements = pending
            .entries
            .iter()
            .map(|(index, bytes)| (*index, bytes.as_slice()))
            .collect::<Vec<_>>();
        let image = pending.image.replace_entries(&replacements)?;
        let changed_entries = replacements
            .iter()
            .filter_map(|(index, bytes)| {
                (pending.image.entry(*index).ok() != Some(*bytes)).then_some(*index)
            })
            .collect::<Vec<_>>();
        if changed_entries.is_empty() && image.bytes() != pending.image.bytes() {
            return Err("pack: unchanged archive failed whole-file byte equality".into());
        }
        for (index, _) in image.entries().iter().enumerate() {
            let expected = pending
                .entries
                .get(&index)
                .map(Vec::as_slice)
                .unwrap_or(pending.image.entry(index)?);
            if image.entry(index)? != expected {
                return Err(format!("pack: entry {index} verification failed").into());
            }
        }
        // Historical source paths are identities, never output paths.
        let path = format!("archives/{}.EMI", sha256_hex(source.as_bytes()));
        let output = staging.join(&path);
        write_new(&output, image.bytes())?;
        let read_back = ArchiveImage::from_bytes(fs::read(output)?)?;
        if read_back.bytes() != image.bytes() {
            return Err("pack: archive read-back verification failed".into());
        }
        reports.push(ArchiveReport {
            source,
            path,
            source_sha256: sha256_hex(pending.image.bytes()),
            output_sha256: sha256_hex(image.bytes()),
            byte_equal: image.bytes() == pending.image.bytes(),
            changed_entries,
        });
    }
    Ok(reports)
}
