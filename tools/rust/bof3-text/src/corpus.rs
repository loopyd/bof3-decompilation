//! Corpus export of supported text layouts, with per-source results.

use std::fs;
use std::io::Read;
use std::path::Path;

use serde::Serialize;

use crate::digest::sha256_hex;
use crate::models::{Error, Result, Source, TextClass};

#[derive(Serialize)]
pub struct Export {
    schema: &'static str,
    scope: &'static str,
    exported: usize,
    pub failures: usize,
    sources: Vec<serde_json::Value>,
}

/// Export supported layouts. Unknown consumers remain the coverage command's gaps.
pub fn extract(root: &Path, output: &Path) -> Result<Export> {
    if root.to_str().is_none() {
        return Err(Error::Invalid("corpus paths must be UTF-8".into()));
    }
    let inputs = crate::coverage::files_under(root)?
        .into_iter()
        .map(|path| {
            let name = crate::coverage::relative(root, &path)?;
            Ok((path, name))
        })
        .collect::<Result<Vec<_>>>()?;
    let input_root = fs::canonicalize(root)?;
    // Require an existing parent so resolution cannot create directories inside the input tree.
    let parent = output
        .parent()
        .filter(|path| !path.as_os_str().is_empty())
        .unwrap_or(Path::new("."));
    let parent = fs::canonicalize(parent)?;
    if parent.starts_with(&input_root) {
        return Err(Error::Invalid(
            "corpus output must be outside the source tree".into(),
        ));
    }
    fs::create_dir(output).map_err(|error| {
        Error::Invalid(format!(
            "{}: {error}; choose a new destination with an existing parent",
            output.display()
        ))
    })?;
    let mut report = Export {
        schema: "bof3.text-export/v1",
        scope: "verified EMI banks, supported inline strings, record names and executable ASCII; use coverage for unresolved consumers",
        exported: 0,
        failures: 0,
        sources: Vec::new(),
    };
    for (path, name) in &inputs {
        let result = (|| {
            if !path.extension().is_some_and(|extension| extension == "EMI") {
                let mut header = Vec::with_capacity(8);
                fs::File::open(path)?.take(8).read_to_end(&mut header)?;
                if !crate::executable::is_executable(&header) {
                    return Ok(Vec::new());
                }
            }
            export_archive(root, path, output, name)
        })();
        match result {
            Ok(rows) => {
                report.exported += rows.len();
                report.sources.extend(rows);
            }
            Err(error) => {
                report.failures += 1;
                report.sources.push(serde_json::json!({"archive": name, "status": "failed", "error": error.to_string()}));
            }
        }
    }
    let manifest =
        serde_json::to_vec_pretty(&report).map_err(|error| Error::Invalid(error.to_string()))?;
    fs::write(output.join("manifest.json"), manifest)?;
    Ok(report)
}

fn export_archive(
    root: &Path,
    path: &Path,
    output: &Path,
    name: &str,
) -> Result<Vec<serde_json::Value>> {
    let data = fs::read(path)?;
    if crate::executable::is_executable(&data) {
        let document = crate::executable::extract(path, &data)?;
        let body = crate::executable::render(&document)?;
        let imported = crate::executable::parse(&body)?;
        if crate::executable::rebuild(path, &data, &imported)? != data {
            return Err(Error::Invalid(format!("{name} does not round-trip")));
        }
        let relative = format!("{name}.json");
        let destination = output.join(&relative);
        fs::create_dir_all(destination.parent().unwrap())?;
        fs::write(destination, body)?;
        return Ok(vec![
            serde_json::json!({"archive": name, "archive_sha256": sha256_hex(&data), "document": relative, "class": "executable_ascii", "status": "exported", "round_trip": true}),
        ]);
    }
    let entries = crate::lexagraph::entries(&data)?;
    // Validate and stage every document before publishing any part of this archive's export.
    let mut documents = Vec::new();
    if crate::records::supports(&data) {
        let document = crate::records::extract(path, &data)?;
        let body = crate::records::render(&document)?;
        let imported = crate::records::parse(&body)?;
        if crate::records::rebuild(path, &data, &imported)? != data {
            return Err(Error::Invalid(format!("{name} does not round-trip")));
        }
        let relative = path.strip_prefix(root).unwrap().join("records.json");
        documents.push((
            relative,
            body,
            entries[0].0,
            serde_json::json!("record_names"),
        ));
    }

    for (entry, payload) in entries {
        if crate::monsters::supports(entry, payload) {
            let document = crate::monsters::extract(path, &data, entry, payload)?;
            let body = crate::monsters::render(&document)?;
            let imported = crate::monsters::parse(&body)?;
            if crate::monsters::rebuild(path, &data, &imported)? != data {
                return Err(Error::Invalid(format!(
                    "{name}#{} does not round-trip",
                    entry.index
                )));
            }
            let relative = path
                .strip_prefix(root)
                .unwrap()
                .join(format!("{}.json", entry.index));
            documents.push((relative, body, entry, serde_json::json!("monster_names")));
            continue;
        }
        if crate::inline::supports(&data, entry, payload) {
            let document = crate::inline::extract(path, &data, entry, payload)?;
            let body = crate::inline::render(&document)?;
            let imported = crate::inline::parse(&body)?;
            if crate::inline::rebuild(path, &data, &imported)? != data {
                return Err(Error::Invalid(format!(
                    "{name}#{} does not round-trip",
                    entry.index
                )));
            }
            let relative = path
                .strip_prefix(root)
                .unwrap()
                .join(format!("{}.json", entry.index));
            documents.push((relative, body, entry, serde_json::json!("inline_text")));
            continue;
        }
        let Some(class) = TextClass::from_load_argument(entry.ram_ptr) else {
            continue;
        };
        let section = crate::extract::parse_section(payload)?;
        let source = Source::new(
            path.to_string_lossy(),
            Some(entry.index),
            entry.ram_ptr,
            class,
        );
        let document = crate::extract::document(&section, &source, &data);
        let body = crate::syntax::render_json(&document)?;
        let imported = crate::syntax::parse_json(&body)?;
        if crate::pack::rebuild(&section, &imported)? != payload {
            return Err(Error::Invalid(format!(
                "{name}#{} does not round-trip",
                entry.index
            )));
        }
        let relative = path
            .strip_prefix(root)
            .unwrap()
            .join(format!("{}.json", entry.index));
        documents.push((relative, body.into_bytes(), entry, serde_json::json!(class)));
    }
    let mut results = Vec::new();
    for (relative, body, entry, class) in documents {
        let destination = output.join(&relative);
        fs::create_dir_all(destination.parent().unwrap())?;
        fs::write(destination, body)?;
        results.push(serde_json::json!({"archive": name, "archive_sha256": sha256_hex(&data), "entry": entry.index, "class": class, "document": relative.to_string_lossy(), "status": "exported", "round_trip": true}));
    }
    Ok(results)
}
