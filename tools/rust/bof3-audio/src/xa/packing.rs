//! XML-directed XA packing with source/overlap checks and verified publication.
use crate::{
    archive::MediaImage,
    archive::XaImage,
    archive::XA_SECTOR_BYTES,
    catalog::loader,
    catalog::model::Catalog,
    codec::quantization::{measure, Loss},
    digest::sha256_hex,
    document::manifest,
    document::manifest::Element,
    machine::executable::Executable,
    machine::profile::Profile,
    publication::require_absent,
    publication::write_new,
    publication::Publication,
    xa::Decoder,
    Result,
};
use serde::Serialize;
use serde_json::json;
use std::{
    borrow::Cow,
    collections::{BTreeMap, BTreeSet},
    fs,
    path::PathBuf,
};

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Padding {
    #[default]
    Reject,
    Zero,
}

pub struct Options {
    pub input: PathBuf,
    pub executable: Option<PathBuf>,
    pub output: PathBuf,
    pub padding: Padding,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub output: String,
    pub sources: Vec<SourceReport>,
    pub assets: Vec<AssetReport>,
    pub padding_policy: Padding,
    pub limitations: Vec<&'static str>,
}

#[derive(Debug, Serialize)]
pub struct SourceReport {
    pub source: String,
    pub path: String,
    pub source_sha256: String,
    pub output_sha256: String,
    pub byte_equal: bool,
    pub changed_sectors: Vec<usize>,
}

#[derive(Debug, Serialize)]
pub struct AssetReport {
    pub id: String,
    pub input_wav_sha256: String,
    pub reconstruction: crate::xa::selection::Report,
    pub padding: Option<PaddingReport>,
}

#[derive(Debug, Serialize)]
pub struct PaddingReport {
    pub input_frames: usize,
    pub padded_frames: usize,
    pub input_loss: Vec<Loss>,
    pub padding_loss: Vec<Loss>,
}

pub fn assets(options: &Options) -> Result<Report> {
    require_absent(&options.output)?;
    let root = options.input.canonicalize()?;
    if !root.is_dir() {
        return Err(
            "XA pack: --input must be an extraction root containing audio.xml and source snapshots"
                .into(),
        );
    }
    let document = manifest::read(&manifest::relative_file(&root, &root, "audio.xml")?)?;
    if document.name != "audio" {
        return Err("XA pack: expected <audio> root".into());
    }
    document.shape(&["schema", "mode", "kind"], &["source", "asset"], false)?;
    let kind = document.attribute("kind")?;
    if !matches!(kind, "xa_stream" | "xa_cue") {
        return Err("XA pack: unsupported extraction root kind".into());
    }
    document.scalars(
        &json!({"schema": "bof3.audio-extraction/v1", "mode": "audio", "kind": kind}),
        &[],
        &[],
    )?;
    let executable = options
        .executable
        .as_ref()
        .map(|path| -> Result<_> {
            let exe = Executable::from_bytes(fs::read(path)?)?;
            Profile::identify(&exe)?;
            Ok(exe)
        })
        .transpose()?;
    if kind == "xa_cue" && executable.is_none() {
        return Err("XA pack: cue manifests require --executable".into());
    }
    let mut sources = BTreeMap::new();
    let mut paths = BTreeSet::new();
    for source in document.children.iter().filter(|n| n.name == "source") {
        source.shape(&["id", "path", "sha256"], &[], false)?;
        let id = source.attribute("id")?.to_owned();
        source.attribute("sha256")?;
        let path = manifest::relative_file(&root, &root, source.attribute("path")?)?;
        if !paths.insert(path.clone()) || sources.insert(id, (path, source)).is_some() {
            return Err("XA pack: duplicate source identity or snapshot path".into());
        }
    }
    let mut grouped = BTreeMap::<String, Vec<(PathBuf, Element)>>::new();
    let mut ids = BTreeSet::new();
    let mut asset_paths = BTreeSet::new();
    for link in document.children.iter().filter(|n| n.name == "asset") {
        link.shape(&["id", "path"], &[], false)?;
        let path = manifest::relative_file(&root, &root, link.attribute("path")?)?;
        let node = manifest::read(&path)?;
        if node.child("runtime")?.attributes.contains_key("profile") && executable.is_none() {
            return Err(
                "XA pack: --executable is required to validate recorded runtime metadata".into(),
            );
        }
        let id = node.attribute("id")?;
        if id != link.attribute("id")? || node.attribute("kind")? != kind {
            return Err("XA pack: root/asset identity or kind conflict".into());
        }
        if !ids.insert(id.to_owned()) || !asset_paths.insert(path.clone()) {
            return Err("XA pack: duplicate asset identity or path".into());
        }
        let source = node.attribute("source")?.to_owned();
        if !sources.contains_key(&source) {
            return Err("XA pack: asset references an unlisted source".into());
        }
        grouped.entry(source).or_default().push((path, node));
    }
    if ids.is_empty() {
        return Err("XA pack: root contains no assets".into());
    }
    let transaction = Publication::new(&options.output)?;
    fs::create_dir(transaction.staging.join("streams"))?;
    let mut report = Report {
        schema: "bof3.xa-pack-report/v1", output: options.output.display().to_string(), sources: Vec::new(), assets: Vec::new(), padding_policy: options.padding,
        limitations: vec![
            "WAV PCM retains the encoded rate/channel layout and original sector capacity; shorter input requires explicit zero padding, while longer input, insertion, relocation and resampling are unsupported",
            "padding loss is separate from input loss, with one record per channel in WAV order; padding does not establish a new audible endpoint",
            "changed cues require compatible entry history and unchanged exit history before following audio; overlapping views must request identical encoded sectors",
            "decoder arithmetic and zero or continuous selected-stream export histories are explicit contexts; actual game seek/reset behavior and audible cue endpoints are not verified",
            "unknown source extents remain unverified; known truncated media is rejected",
            "disc-image rebuilding and complete PSX playback verification are not performed",
        ],
    };
    for (source, (source_path, link)) in sources {
        let snapshot = manifest::read(&source_path)?;
        if snapshot.name != "xa_source" {
            return Err("XA pack: expected <xa_source> snapshot".into());
        }
        snapshot.shape(
            &[
                "schema",
                "source",
                "sector_bytes",
                "sectors",
                "completeness",
            ],
            &["preservation"],
            false,
        )?;
        let image = XaImage::from_bytes(manifest::preservation(
            snapshot.child("preservation")?,
            None,
        )?)?;
        let hash = sha256_hex(image.bytes());
        let reference = crate::xa::reference::identify(&hash);
        if reference.as_ref().is_some_and(|r| r.is_truncated()) {
            return Err("XA pack: known truncated source; supply a complete original STR".into());
        }
        let completeness = if reference.is_some() {
            "known_complete_disc_extent"
        } else {
            "unrecognized_source_extent_not_verified"
        };
        snapshot.scalars(&json!({"schema": "bof3.xa-source/v1", "source": source, "sector_bytes": XA_SECTOR_BYTES, "sectors": image.sectors().len(), "completeness": completeness}), &[], &[])?;
        link.scalars(&json!({"id": source, "sha256": hash}), &[], &["path"])?;
        drop(snapshot);
        let mut catalog = Catalog::from_media(&source, MediaImage::Xa(image.clone()))?;
        let runtime = executable
            .as_ref()
            .map(|exe| loader::resolve(&mut catalog, exe))
            .transpose()?;
        let mut requested = BTreeMap::<usize, (Vec<u8>, String)>::new();
        for (path, node) in grouped.remove(&source).unwrap_or_default() {
            let source_ref = node.child("source_ref")?;
            crate::xa::manifest::leaf(source_ref, &json!({"sha256": hash}), &["path"])?;
            let base = path.parent().ok_or("XA pack: manifest parent absent")?;
            if manifest::relative_file(&root, base, source_ref.attribute("path")?)? != source_path {
                return Err(
                    "XA pack: asset source reference conflicts with root snapshot path".into(),
                );
            }
            let asset = crate::xa::manifest::validate(
                &node,
                &root,
                base,
                &image,
                &catalog,
                runtime.as_ref(),
            )?;
            let format = crate::xa::Format::from_coding(asset.stream.coding)?;
            let channels = usize::from(format.channels());
            let capacity = asset
                .indices
                .len()
                .checked_mul(format.frames_per_sector())
                .and_then(|frames| frames.checked_mul(channels))
                .ok_or("XA pack: selected capacity overflow")?;
            let input_samples = asset.wave.pcm.len();
            if input_samples < capacity && options.padding == Padding::Reject {
                return Err(format!("XA pack: selected capacity is {capacity} interleaved samples, received {input_samples}; shorter WAV input requires explicit --xa-padding zero").into());
            }
            if input_samples > capacity {
                return Err(format!("XA pack: selected capacity is {capacity} interleaved samples, received {input_samples}; longer WAV input cannot be truncated or relocated").into());
            }
            let mut pcm = Cow::Borrowed(asset.wave.pcm.as_slice());
            if input_samples < capacity && options.padding == Padding::Zero {
                pcm.to_mut().resize(capacity, 0);
            }
            let rebuilt = crate::xa::selection::encode(
                &image,
                asset.stream,
                &asset.indices,
                &pcm,
                asset.arithmetic,
                asset.initial_histories,
            )
            .map_err(|e| format!("{}: {e}", node.attribute("id").unwrap_or("unknown asset")))?;
            let padding = if pcm.len() > input_samples {
                let mut decoder =
                    Decoder::new(asset.stream, asset.arithmetic, asset.initial_histories)?;
                let mut decoded = Vec::with_capacity(capacity);
                for (_, bytes) in &rebuilt.patches {
                    decoded.extend(decoder.decode_sector(bytes)?);
                }
                Some(PaddingReport {
                    input_frames: input_samples / channels,
                    padded_frames: (capacity - input_samples) / channels,
                    input_loss: channel_loss(
                        &pcm[..input_samples],
                        &decoded[..input_samples],
                        channels,
                    ),
                    padding_loss: channel_loss(
                        &pcm[input_samples..],
                        &decoded[input_samples..],
                        channels,
                    ),
                })
            } else {
                None
            };
            let id = node.attribute("id")?.to_owned();
            for (index, bytes) in rebuilt.patches {
                if let Some((previous, owner)) = requested.get(&index) {
                    if previous != &bytes {
                        return Err(format!("XA pack: conflicting overlapping views at sector {index}: {owner} and {id}; update all referring WAVs consistently").into());
                    }
                } else {
                    requested.insert(index, (bytes, id.clone()));
                }
            }
            report.assets.push(AssetReport {
                id,
                input_wav_sha256: asset.wav_sha256,
                reconstruction: rebuilt.report,
                padding,
            });
        }
        let mut bytes = image.bytes().to_vec();
        let mut changed_sectors = Vec::new();
        for (&index, (sector, _)) in &requested {
            if sector != image.sector(index)? {
                changed_sectors.push(index);
            }
            bytes[index * XA_SECTOR_BYTES..(index + 1) * XA_SECTOR_BYTES].copy_from_slice(sector);
        }
        let output = XaImage::from_bytes(bytes)?;
        if output.sectors() != image.sectors() {
            return Err("XA pack: sector identity or placement changed".into());
        }
        for sector in image.sectors() {
            let expected = requested
                .get(&sector.index)
                .map(|(b, _)| b.as_slice())
                .unwrap_or(image.sector(sector.index)?);
            if output.sector(sector.index)? != expected {
                return Err("XA pack: sector verification failed".into());
            }
        }
        if changed_sectors.is_empty() && output.bytes() != image.bytes() {
            return Err("XA pack: unchanged source failed whole-file equality".into());
        }
        let path = format!("streams/{}.STR", sha256_hex(source.as_bytes()));
        let destination = transaction.staging.join(&path);
        write_new(&destination, output.bytes())?;
        let read_back = XaImage::from_bytes(fs::read(destination)?)?;
        if read_back.bytes() != output.bytes() {
            return Err("XA pack: source read-back verification failed".into());
        }
        report.sources.push(SourceReport {
            source,
            path,
            source_sha256: hash,
            output_sha256: sha256_hex(output.bytes()),
            byte_equal: output.bytes() == image.bytes(),
            changed_sectors,
        });
    }
    let bytes = serde_json::to_vec_pretty(&report)?;
    write_new(&transaction.staging.join("pack.json"), &bytes)?;
    if fs::read(transaction.staging.join("pack.json"))? != bytes {
        return Err("XA pack: report read-back verification failed".into());
    }
    transaction.publish()?;
    Ok(report)
}

fn channel_loss(input: &[i16], decoded: &[i16], channels: usize) -> Vec<Loss> {
    (0..channels)
        .map(|channel| {
            let input = input
                .iter()
                .skip(channel)
                .step_by(channels)
                .copied()
                .collect::<Vec<_>>();
            let decoded = decoded
                .iter()
                .skip(channel)
                .step_by(channels)
                .copied()
                .collect::<Vec<_>>();
            measure(&input, &decoded)
        })
        .collect()
}
