//! Verified literal documents retained alongside bank and heuristic readings.

use std::collections::BTreeMap;
use std::path::Path;

use serde::{Deserialize, Serialize};
use serde_json::{Value, json};

use crate::models::{Error, Result, invalid};

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq)]
pub struct Document {
    pub archive: String,
    pub contents: Value,
}

impl Document {
    pub fn rows(&self) -> usize {
        self.contents
            .get("names")
            .or_else(|| self.contents.get("strings"))
            .and_then(Value::as_array)
            .map_or(0, Vec::len)
    }

    pub fn entry(&self) -> Option<usize> {
        self.contents["entry"]
            .as_u64()
            .and_then(|entry| usize::try_from(entry).ok())
    }
}

fn document(archive: &str, contents: impl Serialize) -> Result<Document> {
    Ok(Document {
        archive: archive.to_owned(),
        contents: serde_json::to_value(contents)
            .map_err(|error| Error::Invalid(error.to_string()))?,
    })
}

/// Use the same source-bound parsers as direct query and corpus export.
pub fn collect(archive: &str, data: &[u8]) -> Result<Vec<Document>> {
    let path = Path::new(archive);
    if crate::executable::is_executable(data) {
        if !crate::executable::supports(data) {
            return Ok(Vec::new());
        }
        return Ok(vec![document(
            archive,
            crate::executable::extract(path, data)?,
        )?]);
    }
    if crate::records::supports(data) {
        return Ok(vec![document(
            archive,
            crate::records::extract(path, data)?,
        )?]);
    }
    let mut documents = Vec::new();
    for (entry, payload) in crate::lexagraph::entries(data)? {
        if crate::monsters::supports(entry, payload) {
            documents.push(document(
                archive,
                crate::monsters::extract(path, data, entry, payload)?,
            )?);
        } else if crate::inline::supports(entry, payload) {
            documents.push(document(
                archive,
                crate::inline::extract(path, data, entry, payload)?,
            )?);
        }
    }
    Ok(documents)
}

pub struct Selection<'a> {
    pub needle: Option<&'a str>,
    pub ignore_case: bool,
    pub match_offset: bool,
    pub script: Option<&'a str>,
    pub language: Option<&'a str>,
}

/// Literal codecs retain their own matching and byte-offset rules.
pub fn matches(document: &Document, selection: &Selection<'_>) -> Result<Vec<Value>> {
    let mut report = crate::query::literal(
        &document.contents,
        selection.needle,
        selection.ignore_case,
        selection.match_offset,
        false,
    )?;
    let rows = report["matches"]
        .take()
        .as_array()
        .cloned()
        .ok_or_else(|| Error::Invalid("literal query returned no matches array".into()))?;
    let metadata = report
        .as_object_mut()
        .ok_or_else(|| Error::Invalid("literal query returned no object".into()))?;
    for field in ["schema", "matches", "matched", "rows"] {
        metadata.remove(field);
    }
    let mut results = Vec::new();
    for row in rows {
        let text = row["text"]
            .as_str()
            .ok_or_else(|| Error::Invalid("literal query returned no text".into()))?;
        let (scripts, language) = if row["codec"] == json!(crate::inline::Codec::GameGlyphs) {
            crate::lexagraph::script_profile(text)
        } else {
            crate::lexagraph::literal_script_profile(text)
        };
        let script = scripts.first().map(|share| share.script.as_str());
        let language = language
            .as_ref()
            .filter(|guess| guess.reliable)
            .map(|guess| guess.language.as_str());
        if selection
            .script
            .is_some_and(|wanted| script != Some(wanted))
            || selection
                .language
                .is_some_and(|wanted| language != Some(wanted))
        {
            continue;
        }
        let mut hit = Value::Object(metadata.clone());
        hit.as_object_mut().expect("object").extend(
            row.as_object()
                .ok_or_else(|| Error::Invalid("literal query returned a non-object row".into()))?
                .clone(),
        );
        let entry = document
            .entry()
            .map_or(String::new(), |entry| format!("#{entry}"));
        let address = hit["address"]
            .as_u64()
            .ok_or_else(|| Error::Invalid("literal query returned no address".into()))?;
        hit["source"] = json!(format!("{}{entry}@{address:#010x}", document.archive));
        hit["archive"] = json!(document.archive);
        hit["class"] = Value::Null;
        hit["instances"] = json!(1);
        hit["offset"] = hit["file_offset"].clone();
        hit["length"] = hit["capacity"].clone();
        hit["script"] = json!(script);
        hit["language"] = json!(language);
        results.push(hit);
    }
    Ok(results)
}

/// Re-extract once per source, comparing every retained identity and allocation.
pub fn verify(
    document: &Document,
    root: &Path,
    sources: &[crate::textindex::ArchiveStamp],
    cache: &mut BTreeMap<String, Vec<Document>>,
) -> Result<()> {
    if !sources.iter().any(|source| source.path == document.archive) {
        return invalid(format!(
            "literal source {} is not a discovered corpus input; rebuild the index",
            document.archive
        ));
    }
    if !cache.contains_key(&document.archive) {
        let data = std::fs::read(root.join(&document.archive))?;
        cache.insert(document.archive.clone(), collect(&document.archive, &data)?);
    }
    if !cache[&document.archive].contains(document) {
        return invalid(format!(
            "literal index document {} does not reproduce from its source; rebuild the index",
            document.archive
        ));
    }
    Ok(())
}
