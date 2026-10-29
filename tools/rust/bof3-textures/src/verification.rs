//! Independent byte reconstruction and comparison of qualified graphics views.

use std::{fs, path::Path};

use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;
use serde_json::{Value, json};

use crate::{
    Error, Result, area, bank, bank_image,
    catalog::{self, Index, Resource},
    digest::sha256_hex,
    field, formats, package, resident, save_icon,
    sources::{self, Input, SourceFile},
    sprite_bank, sprite_definition,
};

#[derive(PartialEq, Eq)]
enum Content {
    Upload(Vec<u16>),
    Palette(Vec<u16>),
    SaveIcon(save_icon::Images),
    Resident(resident::Document),
    Area(area::Document),
    Definitions(sprite_definition::Document),
    Sprite(
        sprite_bank::Document,
        Vec<bank_image::Decoded>,
        Vec<sprite_bank::SelectionEdit>,
    ),
    DecodedBank(Vec<u8>),
}

fn content(resource: &Resource, bytes: &[u8]) -> Result<Option<Content>> {
    if let Some(upload) = &resource.upload {
        return Ok(Some(Content::Upload(upload.linear_words(bytes)?)));
    }
    if let Some(palette) = &resource.palette_upload {
        return Ok(Some(Content::Palette(palette.decode_words(bytes)?)));
    }
    if let Some(info) = &resource.save_icon {
        return Ok(Some(Content::SaveIcon(save_icon::decode(bytes, info)?)));
    }
    if resource.resident_animation.is_some() {
        return Ok(Some(Content::Resident(resident::decode(bytes)?)));
    }
    if resource.area_surface.is_some() {
        return Ok(Some(Content::Area(area::decode(bytes)?)));
    }
    if resource.sprite_definitions.is_some() {
        return Ok(Some(Content::Definitions(sprite_definition::decode(
            bytes,
        )?)));
    }
    if resource.sprite_bank.is_some() {
        let decoded = sprite_bytes(resource, bytes)?;
        let document = sprite_bank::decode(&decoded)?;
        let images = document
            .roots
            .iter()
            .flat_map(|root| &root.images)
            .map(|image| bank_image::decode(&decoded, image.offset, image.end))
            .collect::<Result<_>>()?;
        return Ok(Some(Content::Sprite(
            document,
            images,
            field::selections(resource, &decoded)?,
        )));
    }
    if resource.bank.is_some() {
        return Ok(Some(Content::DecodedBank(bank::decode(bytes)?.bytes)));
    }
    Ok(None)
}

fn sprite_bytes(resource: &Resource, source: &[u8]) -> Result<Vec<u8>> {
    if resource.bank.is_some() && resource.emi_type == Some(1) {
        return Ok(bank::decode(source)?.bytes);
    }
    if resource.bank.is_none() && resource.emi_type == Some(0) {
        return Ok(source.to_vec());
    }
    Err(Error::Codec(format!(
        "{} has an unsupported sprite transport",
        resource.id
    )))
}

fn inverse(resource: &Resource, original: &[u8]) -> Result<Option<Vec<u8>>> {
    match content(resource, original)? {
        Some(Content::Upload(words)) => {
            for bits in [4, 8] {
                if formats::words_from_indices(&formats::indices(&words, bits)?, bits)? != words {
                    return Err(Error::Codec("indexed upload inverse differs".into()));
                }
            }
            Ok(Some(
                resource
                    .upload
                    .as_ref()
                    .unwrap()
                    .encode_words(&words, original)?,
            ))
        }
        Some(Content::Palette(words)) => Ok(Some(
            resource
                .palette_upload
                .as_ref()
                .unwrap()
                .encode_words(&words, original)?,
        )),
        Some(Content::SaveIcon(images)) => Ok(Some(save_icon::encode(
            original,
            resource.save_icon.as_ref().unwrap(),
            &images,
        )?)),
        Some(Content::Resident(document)) => Ok(Some(resident::encode(original, &document)?)),
        Some(Content::Area(document)) => Ok(Some(area::encode(original, &document)?)),
        Some(Content::Definitions(document)) => {
            Ok(Some(sprite_definition::encode(original, &document)?))
        }
        Some(Content::Sprite(document, _, selections)) => {
            let decoded = sprite_bytes(resource, original)?;
            let mut planes = Vec::new();
            for (root, record) in document.roots.iter().enumerate() {
                for (image, descriptor) in record.images.iter().enumerate() {
                    let bytes =
                        bank_image::decode(&decoded, descriptor.offset, descriptor.end)?.bytes;
                    planes.push((root, image, bytes));
                }
            }
            let edits: Vec<_> = planes
                .iter()
                .map(|(root, image, bytes)| sprite_bank::ImageEdit {
                    root: *root,
                    image: *image,
                    bytes,
                })
                .collect();
            let encoded = sprite_bank::encode_images(&decoded, &document, &edits)?;
            let encoded = sprite_bank::encode_selections(&encoded, &selections)?;
            Ok(Some(if resource.bank.is_some() {
                bank::encode(&encoded, original)?
            } else {
                encoded
            }))
        }
        Some(Content::DecodedBank(bytes)) => Ok(Some(bank::encode(&bytes, original)?)),
        None => Ok(None),
    }
}

#[derive(Serialize)]
struct ResourceResult {
    resource: String,
    original_sha256: String,
    intended_sha256: String,
    output_sha256: Option<String>,
    decode: &'static str,
    unchanged_inverse: &'static str,
    editable_views: &'static str,
    failures: Vec<String>,
}

#[derive(Serialize)]
struct FileResult {
    source: String,
    original_sha256: String,
    intended_sha256: String,
    output_sha256: Option<String>,
    structure: &'static str,
    unchanged_reconstruction: &'static str,
    edited_reconstruction: &'static str,
    unrelated_bytes: &'static str,
    output_matches_intent: bool,
    resources: Vec<ResourceResult>,
    failures: Vec<String>,
}

fn payload<'a>(
    resource: &Resource,
    file: &'a [u8],
    archive: Option<&'a ArchiveImage>,
) -> Result<&'a [u8]> {
    if let Some(slot) = resource.entry {
        return Ok(archive
            .ok_or_else(|| Error::Invalid("output EMI structure is unavailable".into()))?
            .entry(slot)?);
    }
    let start = usize::try_from(resource.offset)
        .map_err(|_| Error::Invalid("resource offset exceeds host bounds".into()))?;
    let size = usize::try_from(resource.size)
        .map_err(|_| Error::Invalid("resource size exceeds host bounds".into()))?;
    file.get(
        start
            ..start
                .checked_add(size)
                .ok_or_else(|| Error::Invalid("resource end overflow".into()))?,
    )
    .ok_or_else(|| Error::Invalid("output resource exceeds its file".into()))
}

fn check_file(
    index: &Index,
    source: &SourceFile,
    original: &[u8],
    intended: &[u8],
    actual: Result<Vec<u8>>,
) -> Result<FileResult> {
    let original_archive = source
        .emi_entries
        .map(|_| ArchiveImage::from_bytes(original.to_vec()))
        .transpose()?;
    let intended_archive = source
        .emi_entries
        .map(|_| ArchiveImage::from_bytes(intended.to_vec()))
        .transpose()?;
    let mut failures = Vec::new();
    let actual = match actual {
        Ok(bytes) => Some(bytes),
        Err(error) => {
            failures.push(error.to_string());
            None
        }
    };
    let actual_archive = actual.as_ref().and_then(|bytes| {
        source.emi_entries?;
        match ArchiveImage::from_bytes(bytes.clone()) {
            Ok(archive) if Some(archive.entries().len()) == source.emi_entries => Some(archive),
            Ok(_) => {
                failures.push("output EMI entry count differs".into());
                None
            }
            Err(error) => {
                failures.push(error.to_string());
                None
            }
        }
    });
    let structure =
        if actual.is_some() && (source.emi_entries.is_none() || actual_archive.is_some()) {
            "verified"
        } else {
            "failed"
        };
    let mut resources = Vec::new();
    let mut inverse_payloads = Vec::new();
    for resource in index
        .resources
        .iter()
        .filter(|resource| resource.source == source.path)
    {
        let before = payload(resource, original, original_archive.as_ref())?;
        let expected = payload(resource, intended, intended_archive.as_ref())?;
        let observed = actual
            .as_ref()
            .map(|bytes| payload(resource, bytes, actual_archive.as_ref()));
        let mut result = ResourceResult {
            resource: resource.id.clone(),
            original_sha256: sha256_hex(before),
            intended_sha256: sha256_hex(expected),
            output_sha256: None,
            decode: "unsupported",
            unchanged_inverse: "unsupported",
            editable_views: "unsupported",
            failures: Vec::new(),
        };
        match inverse(resource, before) {
            Ok(Some(bytes)) => {
                result.unchanged_inverse = if bytes == before {
                    "byte-identical"
                } else {
                    "failed"
                };
                if bytes != before {
                    result
                        .failures
                        .push("unchanged resource inverse differs".into());
                }
                if let Some(slot) = resource.entry {
                    inverse_payloads.push((slot, bytes));
                }
            }
            Ok(None) => {}
            Err(error) => {
                result.unchanged_inverse = "failed";
                result.failures.push(error.to_string());
            }
        }
        match observed {
            Some(Ok(bytes)) => {
                result.output_sha256 = Some(sha256_hex(bytes));
                match (content(resource, expected), content(resource, bytes)) {
                    (Ok(Some(expected)), Ok(Some(observed))) => {
                        result.decode = if resource
                            .resident_animation
                            .as_ref()
                            .is_some_and(|info| info.unresolved_programs != 0)
                        {
                            "partial-unresolved-programs"
                        } else if resource.area_surface.is_some() {
                            "partial-area-metadata"
                        } else if resource.sprite_definitions.is_some() {
                            "partial-sprite-definitions"
                        } else {
                            "verified"
                        };
                        result.editable_views = if expected == observed {
                            "match"
                        } else {
                            "failed"
                        };
                        if expected != observed {
                            result
                                .failures
                                .push("decoded editable views differ from intent".into());
                        }
                    }
                    (Ok(None), Ok(None)) => {}
                    (Err(error), _) | (_, Err(error)) => {
                        result.decode = "failed";
                        result.editable_views = "failed";
                        result.failures.push(error.to_string());
                    }
                    _ => {
                        result.decode = "failed";
                        result.editable_views = "failed";
                        result
                            .failures
                            .push("output graphics interpretation differs".into());
                    }
                }
            }
            Some(Err(error)) => {
                result.decode = "failed";
                result.editable_views = "unavailable";
                result.failures.push(error.to_string());
            }
            None => {
                result.decode = "unavailable";
                result.editable_views = "unavailable";
            }
        }
        resources.push(result);
    }
    let borrowed: Vec<_> = inverse_payloads
        .iter()
        .map(|(slot, bytes)| (*slot, bytes.as_slice()))
        .collect();
    let rebuilt = original_archive
        .as_ref()
        .map(|archive| archive.replace_entries(&borrowed))
        .transpose()?;
    let unchanged_reconstruction = if rebuilt
        .as_ref()
        .is_none_or(|archive| archive.bytes() == original)
        && resources
            .iter()
            .all(|resource| resource.unchanged_inverse != "failed")
    {
        "byte-identical"
    } else {
        "failed"
    };
    let edited: Vec<_> = resources
        .iter()
        .filter(|resource| resource.original_sha256 != resource.intended_sha256)
        .collect();
    let edited_reconstruction = if edited.is_empty() {
        "unobserved"
    } else if edited
        .iter()
        .all(|resource| resource.editable_views == "match")
    {
        "verified"
    } else {
        "failed"
    };
    let unrelated_bytes = if let Some(actual) = &actual {
        if actual.len() != original.len() || intended.len() != original.len() {
            failures.push("output allocation differs from its original file".into());
            "failed"
        } else if let Some(offset) = original
            .iter()
            .zip(intended)
            .zip(actual)
            .position(|((old, new), actual)| old == new && new != actual)
        {
            failures.push(format!("unrelated output byte changed at 0x{offset:x}"));
            "failed"
        } else {
            "byte-identical"
        }
    } else {
        "unavailable"
    };
    let output_matches_intent = actual.as_deref() == Some(intended);
    if actual.is_some() && !output_matches_intent {
        failures.push("complete output bytes differ from intended reconstruction".into());
    }
    Ok(FileResult {
        source: source.path.clone(),
        original_sha256: source.sha256.clone(),
        intended_sha256: sha256_hex(intended),
        output_sha256: actual.as_ref().map(|bytes| sha256_hex(bytes)),
        structure,
        unchanged_reconstruction,
        edited_reconstruction,
        unrelated_bytes,
        output_matches_intent,
        resources,
        failures,
    })
}

fn report(index: &Index, files: Vec<FileResult>, scope: &str) -> Value {
    let coverage = catalog::coverage(index);
    let byte_checks_passed = files.iter().all(|file| {
        file.structure == "verified"
            && file.unchanged_reconstruction == "byte-identical"
            && file.edited_reconstruction != "failed"
            && file.unrelated_bytes == "byte-identical"
            && file.output_matches_intent
            && file
                .resources
                .iter()
                .all(|resource| resource.failures.is_empty())
    });
    let decode_failed = files
        .iter()
        .flat_map(|file| &file.resources)
        .any(|resource| matches!(resource.decode, "failed" | "unavailable"));
    let any_edit = files
        .iter()
        .any(|file| file.edited_reconstruction != "unobserved");
    json!({
        "schema": "bof3.textures-verification/v2", "release": index.release,
        "inventory_sha256": index.inventory_sha256, "interpretation_sha256": index.interpretation_sha256,
        "scope": scope, "complete": false, "byte_checks_passed": byte_checks_passed,
        "structure": {"status": if files.iter().all(|file| file.structure == "verified") { "verified" } else { "failed" }},
        "dependencies": {"status": "unresolved", "missing": coverage["unresolved_dependencies"]},
        "decode": {"status": if decode_failed { "failed" } else { "partial" },
            "scope": "source words, palettes, bank bytes, image index planes and parsed records; no rendered appearance",
            "verified_resources": files.iter().flat_map(|file| &file.resources).filter(|resource| resource.decode == "verified").count(),
            "partial_resources": files.iter().flat_map(|file| &file.resources).filter(|resource| matches!(resource.decode, "partial-unresolved-programs" | "partial-area-metadata")).count(),
            "unsupported_resources": files.iter().flat_map(|file| &file.resources).filter(|resource| resource.decode == "unsupported").count()},
        "unchanged_reconstruction": {"status": if files.iter().all(|file| file.unchanged_reconstruction == "byte-identical") { "byte-identical" } else { "failed" },
            "scope": "complete original files after supported unchanged inverses; unknown bytes preserved"},
        "edited_reconstruction": {"status": if !any_edit { "unobserved" } else if files.iter().all(|file| file.edited_reconstruction != "failed") { "verified" } else { "failed" },
            "scope": "intended edited source views decoded independently from actual output"},
        "unrelated_bytes": {"status": if files.iter().all(|file| file.unrelated_bytes == "byte-identical") { "byte-identical" } else { "failed" },
            "scope": "every file byte outside the original-to-intended change positions, including headers and gaps"},
        "runtime": {"status": "unobserved", "missing": ["consumer appearance, palette choice, placement, timing and loading observations"]},
        "files": files, "coverage": coverage,
    })
}

pub fn original(input: &Input) -> Result<Value> {
    let index = catalog::build(input)?;
    let mut files = Vec::new();
    for source in &index.files {
        let original = sources::verify_source(&input.source_path(source), source)?;
        files.push(check_file(
            &index,
            source,
            &original,
            &original,
            Ok(original.clone()),
        )?);
    }
    Ok(report(&index, files, "qualified-original-inputs"))
}

pub fn output(root: &Path, package_root: &Path) -> Result<Value> {
    let root = fs::canonicalize(root)?;
    if !root.is_dir() {
        return Err(Error::Invalid("verified output must be a directory".into()));
    }
    let package_root = fs::canonicalize(package_root)?;
    let manifest = package::load(&package_root)?;
    let mut files = Vec::new();
    for (source, snapshot) in manifest.index.files.iter().zip(&manifest.snapshots) {
        let reconstructed = package::reconstruct(&package_root, &manifest, source, snapshot)?;
        let actual = package::read_member(&root, &source.path, sources::MAX_FILE_BYTES);
        files.push(check_file(
            &manifest.index,
            source,
            &reconstructed.original,
            &reconstructed.output,
            actual,
        )?);
    }
    Ok(report(&manifest.index, files, "bound-package-output"))
}
