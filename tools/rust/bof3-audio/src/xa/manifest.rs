//! Verify XA extraction manifests against preserved sectors and runtime tables.
use crate::{
    archive::XaImage, catalog::model::Catalog, digest::sha256_hex, document::manifest,
    document::manifest::Element, interchange::wave::Wave, xa::Arithmetic, xa::Decoder,
    xa::Histories, xa::Stream, Result,
};
use serde_json::json;
use std::{collections::BTreeSet, fs, path::Path};

pub(crate) struct Validated {
    pub stream: Stream,
    pub indices: Vec<usize>,
    pub arithmetic: Arithmetic,
    pub wave: Wave,
    pub wav_sha256: String,
}

pub(crate) fn validate(
    node: &Element,
    root: &Path,
    base: &Path,
    image: &XaImage,
    catalog: &Catalog,
    runtime: Option<&crate::catalog::model::Report>,
) -> Result<Validated> {
    if node.name != "xa_asset" {
        return Err("XA pack: expected <xa_asset>".into());
    }
    node.shape(
        &["schema", "id", "source", "kind", "content_type"],
        &[
            "source_ref",
            "stream",
            "decoder",
            "final_history",
            "wave",
            "sectors",
            "runtime",
            "cue",
        ],
        false,
    )?;
    let kind = node.attribute("kind")?;
    if !matches!(kind, "xa_stream" | "xa_cue") {
        return Err("XA pack: unsupported asset kind".into());
    }
    let asset = catalog.select("audio", Some(kind), node.attribute("id")?)?;
    node.scalars(&json!({"schema": "bof3.xa-asset/v1", "id": asset.id, "source": asset.source, "kind": asset.kind(), "content_type": "unresolved"}), &[], &[])?;
    let (stream, indices) = crate::xa::extraction::selection(asset, image)?;
    let decoder_node = node.child("decoder")?;
    let arithmetic = match decoder_node.attribute("arithmetic")? {
        "split_floor" => Arithmetic::SplitFloor,
        "combined_rounded" => Arithmetic::CombinedRounded,
        _ => return Err("XA pack: unsupported decoder arithmetic".into()),
    };
    leaf(
        decoder_node,
        &json!({"arithmetic": arithmetic, "initial_history": "zero_per_exported_asset", "resampling": "none", "audible_endpoint": "unverified"}),
        &[],
    )?;
    let mut decoder = Decoder::new(stream, arithmetic, Histories::default())?;
    let format = decoder.format();
    let stream_node = node.child("stream")?;
    let mut expected = serde_json::to_value(stream)?;
    expected
        .as_object_mut()
        .unwrap()
        .extend(serde_json::to_value(format)?.as_object().unwrap().clone());
    leaf(stream_node, &expected, &[])?;
    let sectors = node.child("sectors")?;
    sectors.shape(&[], &["sector"], false)?;
    if sectors.children.len() != indices.len() {
        return Err("XA pack: edited sector selection count unsupported".into());
    }
    let frames = format.frames_per_sector();
    let mut pcm = Vec::new();
    let mut encoded = Vec::new();
    for ((ordinal, &index), entry) in indices.iter().enumerate().zip(&sectors.children) {
        let sector = image.sector(index)?;
        leaf(
            entry,
            &json!({"index": index, "frame_start": ordinal * frames, "frames": frames, "submode": sector[2]}),
            &[],
        )?;
        pcm.extend(decoder.decode_sector(sector)?);
        encoded.extend_from_slice(sector);
    }
    let final_history = node.child("final_history")?;
    final_history.shape(&[], &["left", "right"], false)?;
    leaf(final_history.child("left")?, &decoder.histories().left, &[])?;
    leaf(
        final_history.child("right")?,
        &decoder.histories().right,
        &[],
    )?;
    let original = Wave::new(format.channels(), format.sample_rate(), pcm)?;
    let wave_node = node.child("wave")?;
    leaf(
        wave_node,
        &json!({"frames": original.frames(), "sha256": sha256_hex(&original.to_bytes()?), "selected_sectors_sha256": sha256_hex(&encoded)}),
        &["path"],
    )?;
    let path = manifest::relative_file(root, base, wave_node.attribute("path")?)?;
    let bytes = fs::read(path)?;
    let wave = Wave::from_bytes(&bytes)?;
    if wave.channels != original.channels || wave.sample_rate != original.sample_rate {
        return Err(
            "XA pack: WAV rate/channel changes are unsupported; retain encoded format".into(),
        );
    }
    if wave.sampler.is_some() || wave.opaque_chunks().next().is_some() || wave.has_extended_fact() {
        return Err("XA pack: WAV sampler/ancillary metadata has no XA representation; remove it explicitly".into());
    }
    validate_runtime(node, &indices, stream, frames, &asset.source, runtime)?;
    Ok(Validated {
        stream,
        indices,
        arithmetic,
        wave,
        wav_sha256: sha256_hex(&bytes),
    })
}

pub(crate) fn leaf(
    node: &Element,
    expected: &impl serde::Serialize,
    extras: &[&str],
) -> Result<()> {
    node.scalars(expected, &[], extras)?;
    node.shape(
        &node
            .attributes
            .keys()
            .map(String::as_str)
            .collect::<Vec<_>>(),
        &[],
        false,
    )
}

fn validate_runtime(
    node: &Element,
    indices: &[usize],
    stream: Stream,
    frames: usize,
    source: &str,
    runtime: Option<&crate::catalog::model::Report>,
) -> Result<()> {
    let declared = node.child("runtime")?;
    let cues = node
        .children
        .iter()
        .filter(|n| n.name == "cue")
        .collect::<Vec<_>>();
    if declared.attributes.contains_key("resolution") {
        leaf(
            declared,
            &json!({"resolution": "not_supplied", "cue_placement": "unresolved"}),
            &[],
        )?;
        if node.attribute("kind")? == "xa_cue" || !cues.is_empty() {
            return Err("XA pack: cue placement requires verified runtime metadata".into());
        }
        return Ok(());
    }
    let runtime = runtime
        .ok_or("XA pack: --executable is required to validate recorded runtime/cue metadata")?;
    leaf(
        declared,
        &json!({"profile": runtime.profile.id, "executable_sha256": runtime.profile.exe_sha256, "selector": runtime.xa.selector}),
        &[],
    )?;
    let expected = runtime
        .xa
        .cues
        .iter()
        .filter_map(|mapped| {
            if mapped.cue.filter_file != stream.file
                || mapped.cue.channel != stream.channel
                || !mapped
                    .bindings
                    .iter()
                    .any(|b| b.source == source && b.asset.is_some())
            {
                return None;
            }
            Some((
                &mapped.cue,
                indices.binary_search(&mapped.cue.sector_start).ok()?,
                indices.binary_search(&mapped.cue.last_sector).ok()?,
            ))
        })
        .collect::<Vec<_>>();
    if cues.len() != expected.len() {
        return Err("XA pack: edited runtime cue count unsupported".into());
    }
    let mut seen = BTreeSet::new();
    for cue in cues {
        let id = cue.number::<u16>("packed_id")?;
        let &(original, first, last) = expected
            .iter()
            .find(|(c, _, _)| c.packed_id == id)
            .ok_or("XA pack: unknown runtime cue")?;
        if !seen.insert(id) {
            return Err("XA pack: duplicate runtime cue".into());
        }
        cue.scalars(
            original,
            &["runtime_start_bcd", "runtime_stop_threshold_bcd"],
            &["frame_start", "frame_end_exclusive"],
        )?;
        cue.shape(
            &cue.attributes
                .keys()
                .map(String::as_str)
                .collect::<Vec<_>>(),
            &["runtime_start_bcd", "runtime_stop_threshold_bcd"],
            false,
        )?;
        if cue.number::<usize>("frame_start")? != first * frames
            || cue.number::<usize>("frame_end_exclusive")? != (last + 1) * frames
        {
            return Err("XA pack: edited cue frame placement unsupported".into());
        }
        for (name, value) in [
            ("runtime_start_bcd", original.runtime_start_bcd),
            (
                "runtime_stop_threshold_bcd",
                original.runtime_stop_threshold_bcd,
            ),
        ] {
            leaf(
                cue.child(name)?,
                &json!({"minute": value[0], "second": value[1], "frame": value[2]}),
                &[],
            )?;
        }
    }
    Ok(())
}
