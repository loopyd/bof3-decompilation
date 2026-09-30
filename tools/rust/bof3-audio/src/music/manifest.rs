//! Compare music XML to a freshly regenerated source-authoritative document.
use crate::{
    document::manifest::{self, Element},
    Result,
};
use serde_json::{json, Value};
use std::{
    collections::BTreeSet,
    path::{Path, PathBuf},
};

pub(crate) fn documents(root: &Path) -> Result<Vec<(PathBuf, Option<Element>)>> {
    if root.join("music.xml").exists() && root.join("song.xml").exists() {
        return Err("music pack: both music.xml and song.xml exist; input is ambiguous".into());
    }
    if !root.join("music.xml").exists() {
        return Ok(vec![(
            manifest::relative_file(root, root, "song.xml")?,
            None,
        )]);
    }
    let node = manifest::read(&manifest::relative_file(root, root, "music.xml")?)?;
    if node.name != "music" {
        return Err("music pack: expected <music> root".into());
    }
    node.shape(&["schema"], &["song"], false)?;
    node.scalars(&json!({"schema":"bof3.music-extraction/v1"}), &[], &[])?;
    if node.children.is_empty() {
        return Err("music pack: root contains no songs".into());
    }
    let mut paths = BTreeSet::new();
    let mut documents = Vec::new();
    for link in node.children {
        link.shape(&["id", "bank", "path", "source_sha256"], &[], false)?;
        let path = manifest::relative_file(root, root, link.attribute("path")?)?;
        if !paths.insert(path.clone()) {
            return Err("music pack: duplicate manifest path".into());
        }
        documents.push((path, Some(link)));
    }
    Ok(documents)
}

/// Paths are editable references. Preservation and descriptive limitation prose
/// are handled separately; every executable mapping and source field must agree.
pub(crate) fn validate(actual: &Element, expected: &Element, path: &str) -> Result<()> {
    if actual.name != expected.name {
        return Err(format!(
            "music pack: {path}: expected <{}>, found <{}>",
            expected.name, actual.name
        )
        .into());
    }
    if actual.name == "preservation" {
        if manifest::preservation(actual, Some("entire_source_archive"))?
            != manifest::preservation(expected, Some("entire_source_archive"))?
        {
            return Err("music pack: preserved archive changed during validation".into());
        }
        return Ok(());
    }
    let mut attributes = expected.attributes.clone();
    if matches!(path, "song/bank/soundfont" | "song/sequences/sequence") {
        attributes.insert("path".into(), actual.attribute("path")?.into());
    }
    actual.scalars(&attributes, &[], &[])?;
    if actual.children.len() != expected.children.len() {
        return Err(format!("music pack: {path}: missing or added manifest children").into());
    }
    if expected
        .attributes
        .get("encoding")
        .is_some_and(|v| v == "json")
    {
        let mut value: Value = serde_json::from_str(&actual.text)?;
        let mut source: Value = serde_json::from_str(&expected.text)?;
        if actual.name == "limitations" {
            require_notes(&value)?;
        } else {
            // Limitation wording can evolve without changing schema or assets.
            // It is descriptive evidence, never an input to reconstruction.
            if actual.name == "translation" {
                require_notes(
                    value
                        .get("limitations")
                        .ok_or("music pack: missing translation limitations")?,
                )?;
                value
                    .as_object_mut()
                    .ok_or("music pack: translation must be an object")?
                    .remove("limitations");
                source
                    .as_object_mut()
                    .ok_or("music pack: invalid reference translation")?
                    .remove("limitations");
            }
            if value != source {
                return Err(
                    format!("music pack: {path}: edited or conflicting JSON evidence").into(),
                );
            }
        }
    } else if actual.text.trim_matches([' ', '\t', '\n', '\r'])
        != expected.text.trim_matches([' ', '\t', '\n', '\r'])
    {
        return Err(format!("music pack: {path}: unexpected text edit").into());
    }
    for (child, reference) in actual.children.iter().zip(&expected.children) {
        validate(child, reference, &format!("{path}/{}", reference.name))?;
    }
    Ok(())
}

fn require_notes(value: &Value) -> Result<()> {
    if value
        .as_array()
        .is_some_and(|a| a.iter().all(Value::is_string))
    {
        Ok(())
    } else {
        Err("music pack: limitations must be a JSON array of descriptive strings".into())
    }
}
