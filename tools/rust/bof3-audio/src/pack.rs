//! Bank folder/root packing with immutable identity checks and staged publication.
use crate::{
    archive::packing, archive::packing::ArchiveReport, archive::packing::Pending,
    archive::MediaImage, catalog::loader, catalog::model::Catalog, digest::sha256_hex,
    document::manifest, document::manifest::Element, machine::executable::Executable,
    publication::require_absent, publication::write_new, publication::Publication, Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use serde_json::json;
use std::{
    collections::{BTreeMap, BTreeSet},
    fs,
    path::{Path, PathBuf},
};

pub struct Options {
    pub input: PathBuf,
    pub executable: PathBuf,
    pub output: PathBuf,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub output: String,
    pub archives: Vec<ArchiveReport>,
    pub banks: Vec<BankReport>,
    pub limitations: Vec<&'static str>,
}

#[derive(Debug, Serialize)]
pub struct BankReport {
    pub id: String,
    pub body_entry: usize,
    pub samples: Vec<crate::bank::packing::Report>,
}

pub fn banks(options: &Options) -> Result<Report> {
    require_absent(&options.output)?;
    let root = options.input.canonicalize()?;
    if !root.is_dir() {
        return Err("pack: --input must be one bank folder or an extraction root".into());
    }
    let documents = documents(&root)?;
    let executable = Executable::from_bytes(fs::read(&options.executable)?)?;
    crate::machine::profile::Profile::identify(&executable)?;
    let mut pending = BTreeMap::<String, Pending>::new();
    let mut ids = BTreeSet::new();
    let mut report = Report {
        schema: "bof3.audio-pack-report/v1", output: options.output.display().to_string(), archives: Vec::new(), banks: Vec::new(),
        limitations: vec![
            "only bank PCM and forward smpl loop edits within existing allocations are supported; XML musical/runtime metadata is validated, not editable",
            "sample reference rates are export contexts; no resampling, tuning, program or allocation edits are inferred",
            "sample loss uses zero-history integer SPU decoding; complete original-runtime playback is not verified",
            "shared content at distinct archive identities is independent; only listed bank folders are edited",
            "music folders use --mode music; disc image rebuilding is outside scope",
        ],
    };
    for (path, link) in documents {
        let node = manifest::read(&path)?;
        let bytes =
            manifest::preservation(node.child("preservation")?, Some("entire_source_archive"))?;
        if let Some(link) = link {
            link.scalars(
                &json!({"id": node.attribute("id")?, "source_sha256": sha256_hex(&bytes)}),
                &[],
                &["path"],
            )?;
        }
        let image = ArchiveImage::from_bytes(bytes)?;
        let source = node.attribute("source")?;
        let mut catalog = Catalog::from_media(source, MediaImage::Emi(image.clone()))?;
        let mapping = loader::resolve(&mut catalog, &executable)?;
        let validated = crate::bank::manifest::validate(&node, &catalog, &mapping)?;
        if !ids.insert(validated.asset.id.clone()) {
            return Err(format!("pack: duplicate bank identity {}", validated.asset.id).into());
        }
        let original = image.entry(validated.body_entry)?;
        let mut body = original.to_vec();
        let mut samples = Vec::new();
        for sample in &validated.bank.samples {
            let metadata = node
                .child("samples")?
                .children
                .iter()
                .find(|n| n.number::<u16>("sample_id").ok() == Some(sample.sample_id))
                .ok_or("pack: sample missing")?;
            let wav_path = manifest::relative_file(
                &root,
                path.parent().ok_or("pack: manifest has no parent")?,
                metadata.attribute("path")?,
            )?;
            let range = sample.body_offset
                ..sample
                    .body_offset
                    .checked_add(sample.encoded_bytes)
                    .ok_or("pack: sample range overflow")?;
            let packed = crate::bank::packing::pack(
                metadata,
                sample,
                original
                    .get(range.clone())
                    .ok_or("pack: sample exceeds body")?,
                validated.rate,
                &fs::read(wav_path)?,
            )
            .map_err(|e| format!("{} sample {}: {e}", validated.asset.id, sample.sample_id))?;
            body[range].copy_from_slice(&packed.bytes);
            samples.push(packed.report);
        }
        let entry = pending.entry(source.into()).or_insert_with(|| Pending {
            image: image.clone(),
            entries: BTreeMap::new(),
        });
        if entry.image.bytes() != image.bytes() {
            return Err(format!("pack: conflicting preserved archives for source {source}").into());
        }
        if let Some(existing) = entry.entries.insert(validated.body_entry, body.clone()) {
            if existing != body {
                return Err(format!(
                    "pack: conflicting edits for source {source} entry {}",
                    validated.body_entry
                )
                .into());
            }
        }
        report.banks.push(BankReport {
            id: validated.asset.id.clone(),
            body_entry: validated.body_entry,
            samples,
        });
    }
    let transaction = Publication::new(&options.output)?;
    report.archives = packing::stage(&transaction.staging, pending)?;
    let bytes = serde_json::to_vec_pretty(&report)?;
    write_new(&transaction.staging.join("pack.json"), &bytes)?;
    if fs::read(transaction.staging.join("pack.json"))? != bytes {
        return Err("pack: report read-back verification failed".into());
    }
    transaction.publish()?;
    Ok(report)
}

fn documents(root: &Path) -> Result<Vec<(PathBuf, Option<Element>)>> {
    let audio = root.join("audio.xml");
    let bank = root.join("bank.xml");
    if audio.exists() && bank.exists() {
        return Err("pack: folder contains both audio.xml and bank.xml; input is ambiguous".into());
    }
    if !audio.exists() {
        return Ok(vec![(
            manifest::relative_file(root, root, "bank.xml")?,
            None,
        )]);
    }
    let node = manifest::read(&manifest::relative_file(root, root, "audio.xml")?)?;
    if node.name != "audio" {
        return Err("pack: expected <audio> root".into());
    }
    node.shape(&["schema", "mode", "kind"], &["bank"], false)?;
    node.scalars(
        &json!({"schema": "bof3.audio-extraction/v1", "mode": "audio", "kind": "bank"}),
        &[],
        &[],
    )?;
    if node.children.is_empty() {
        return Err("pack: root contains no banks".into());
    }
    let mut paths = BTreeSet::new();
    let mut result = Vec::new();
    for link in node.children {
        link.shape(&["id", "path", "source_sha256"], &[], false)?;
        let path = manifest::relative_file(root, root, link.attribute("path")?)?;
        if !paths.insert(path.clone()) {
            return Err("pack: duplicate manifest path".into());
        }
        result.push((path, Some(link)));
    }
    Ok(result)
}
