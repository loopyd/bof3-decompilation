//! Self-contained archive snapshots and validated, staged raw-upload edits.

use std::collections::{BTreeMap, BTreeSet};
use std::fs;
use std::path::{Component, Path, PathBuf};
use std::process::Command;
use std::time::{SystemTime, UNIX_EPOCH};

use emi_ex_v2::image::ArchiveImage;
use serde::{Deserialize, Serialize};

use crate::{
    Error, Result, bank, bank_image,
    catalog::{self, Index, Resource},
    digest::sha256_hex,
    formats,
    png::{self, IndexedImage},
    resident,
    sources::{self, Input},
    sprite_bank,
};

pub const SCHEMA: &str = "bof3.textures-package/v2";

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Snapshot {
    pub source: String,
    pub member: String,
    pub sha256: String,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct BankImage {
    pub root: usize,
    pub image: usize,
    pub member: String,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Export {
    pub resource: String,
    pub raw: String,
    pub words: Option<String>,
    pub indices_4: Option<String>,
    pub indices_8: Option<String>,
    pub decoded: Option<String>,
    pub animation: Option<String>,
    pub sprite: Option<String>,
    pub bank_images: Vec<BankImage>,
    pub palette_binding: String,
}

#[derive(Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Manifest {
    pub schema: String,
    pub index: Index,
    pub snapshots: Vec<Snapshot>,
    pub exports: Vec<Export>,
}

fn snapshots(index: &Index) -> Vec<Snapshot> {
    index
        .files
        .iter()
        .map(|file| Snapshot {
            source: file.path.clone(),
            member: format!("snapshots/{}.bin", file.sha256),
            sha256: file.sha256.clone(),
        })
        .collect()
}

fn exports(index: &Index) -> Vec<Export> {
    index
        .resources
        .iter()
        .enumerate()
        .map(|(number, resource)| {
            let directory = format!("resources/{number:05}");
            Export {
                resource: resource.id.clone(),
                raw: format!("{directory}/raw.bin"),
                words: (resource.upload.is_some() || resource.palette_upload.is_some())
                    .then(|| format!("{directory}/words.json")),
                indices_4: resource
                    .upload
                    .as_ref()
                    .map(|_| format!("{directory}/indices-4.png")),
                indices_8: resource
                    .upload
                    .as_ref()
                    .map(|_| format!("{directory}/indices-8.png")),
                decoded: resource
                    .bank
                    .as_ref()
                    .map(|_| format!("{directory}/decoded.bin")),
                animation: resource
                    .resident_animation
                    .as_ref()
                    .map(|_| format!("{directory}/animation.json")),
                sprite: resource
                    .sprite_bank
                    .as_ref()
                    .map(|_| format!("{directory}/sprite.json")),
                bank_images: resource
                    .sprite_bank
                    .as_ref()
                    .map(|bank| {
                        bank.image_modes
                            .iter()
                            .enumerate()
                            .flat_map(|(root, modes)| {
                                let directory = &directory;
                                modes
                                    .iter()
                                    .enumerate()
                                    .filter(|(_, mode)| matches!(mode, 1 | 2))
                                    .map(move |(image, _)| BankImage {
                                        root,
                                        image,
                                        member: format!(
                                            "{directory}/root-{root}/image-{image}.png"
                                        ),
                                    })
                            })
                            .collect()
                    })
                    .unwrap_or_default(),
                palette_binding: if resource.palette_upload.is_some() {
                    "palette-word source; draw CLUT selection and texture uses are unresolved"
                } else {
                    "unresolved; PNG palettes are diagnostic grayscale, not game CLUTs"
                }
                .into(),
            }
        })
        .collect()
}

fn grayscale() -> Vec<u8> {
    (0..=255)
        .flat_map(|value| [value, value, value, 255])
        .collect()
}

fn image(resource: &Resource, payload: &[u8], bits: u8) -> Result<IndexedImage> {
    let upload = resource
        .upload
        .as_ref()
        .ok_or_else(|| Error::Invalid("resource has no qualified raw-word layout".into()))?;
    Ok(IndexedImage {
        width: upload.width_words() * usize::from(16 / bits),
        height: upload.height(),
        indices: formats::indices(&upload.linear_words(payload)?, bits)?,
        palette_bgra: grayscale(),
    })
}

fn bank_image_view(source: &[u8], descriptor: &bank_image::Info) -> Result<IndexedImage> {
    if !matches!(descriptor.mode, 1 | 2) {
        return Err(Error::Invalid(
            "bank PNG requires an indexed mode-1 or mode-2 record".into(),
        ));
    }
    Ok(IndexedImage {
        width: usize::from(descriptor.width_words) * 2,
        height: usize::from(descriptor.height),
        indices: bank_image::decode(source, descriptor.offset, descriptor.end)?.bytes,
        palette_bgra: grayscale(),
    })
}

fn import_bank(root: &Path, export: &Export, original: &[u8]) -> Result<Vec<u8>> {
    let decoded = bank::decode(original)?.bytes;
    let Some(member) = &export.sprite else {
        return Ok(original.to_vec());
    };
    let document: sprite_bank::Document =
        serde_json::from_slice(&read_member(root, member, 64 * 1024 * 1024)?)?;
    let original_document = sprite_bank::decode(&decoded)?;
    let mut planes = Vec::with_capacity(export.bank_images.len());
    for view in &export.bank_images {
        let descriptor = &original_document.roots[view.root].images[view.image];
        let expected = bank_image_view(&decoded, descriptor)?;
        let imported = png::decode(&read_member(
            root,
            &view.member,
            (png::MAX_PIXELS * 2 + 65536) as u64,
        )?)?;
        if imported.width != expected.width
            || imported.height != expected.height
            || imported.palette_bgra != expected.palette_bgra
        {
            return Err(Error::Invalid(format!(
                "{}: preserve dimensions and diagnostic palette; edit exact palette words in sprite.json",
                view.member
            )));
        }
        planes.push(imported.indices);
    }
    let edits: Vec<_> = export
        .bank_images
        .iter()
        .zip(&planes)
        .map(|(view, bytes)| sprite_bank::ImageEdit {
            root: view.root,
            image: view.image,
            bytes,
        })
        .collect();
    let intended = sprite_bank::encode_images(&decoded, &document, &edits)?;
    let encoded = bank::encode(&intended, original)?;
    if bank::decode(&encoded)?.bytes != intended {
        return Err(Error::Invalid(
            "packed sprite bank differs from intended logical records".into(),
        ));
    }
    Ok(encoded)
}

pub fn unpack(input: &Input, destination: &Path) -> Result<Manifest> {
    let index = catalog::build(input)?;
    let stage = Stage::new(destination, &input.path)?;
    let manifest = Manifest {
        schema: SCHEMA.into(),
        snapshots: snapshots(&index),
        exports: exports(&index),
        index,
    };
    let mut written = BTreeSet::new();
    for (source, snapshot) in input.files.iter().zip(&manifest.snapshots) {
        let bytes = sources::verify_source(&input.source_path(source), source)?;
        if written.insert(&snapshot.member) {
            stage.write(&snapshot.member, &bytes)?;
        }
        let resources = manifest
            .index
            .resources
            .iter()
            .zip(&manifest.exports)
            .filter(|(resource, _)| resource.source == source.path);
        for (resource, export) in resources {
            let start = resource.offset as usize;
            let raw = bytes
                .get(start..start + resource.size as usize)
                .ok_or_else(|| Error::Invalid(format!("{} exceeds snapshot", resource.id)))?;
            stage.write(&export.raw, raw)?;
            if let Some(member) = &export.decoded {
                let decoded = bank::decode(raw)?.bytes;
                stage.write(member, &decoded)?;
                if let Some(member) = &export.sprite {
                    let document = sprite_bank::decode(&decoded)?;
                    stage.write(member, &serde_json::to_vec_pretty(&document)?)?;
                    for view in &export.bank_images {
                        let descriptor = &document.roots[view.root].images[view.image];
                        stage.write(
                            &view.member,
                            &png::encode(&bank_image_view(&decoded, descriptor)?)?,
                        )?;
                    }
                }
            }
            if let Some(member) = &export.animation {
                stage.write(member, &serde_json::to_vec_pretty(&resident::decode(raw)?)?)?;
            }
            if let (Some(palette), Some(member)) = (&resource.palette_upload, &export.words) {
                stage.write(
                    member,
                    &serde_json::to_vec_pretty(&palette.decode_words(raw)?)?,
                )?;
            }
            if let Some(upload) = &resource.upload {
                if let Some(member) = &export.words {
                    stage.write(
                        member,
                        &serde_json::to_vec_pretty(&upload.linear_words(raw)?)?,
                    )?;
                }
                for (bits, member) in [(4, &export.indices_4), (8, &export.indices_8)] {
                    if let Some(member) = member {
                        stage.write(member, &png::encode(&image(resource, raw, bits)?)?)?;
                    }
                }
            }
        }
    }
    stage.write("manifest.json", &serde_json::to_vec_pretty(&manifest)?)?;
    stage.write(
        "coverage.json",
        &serde_json::to_vec_pretty(&catalog::coverage(&manifest.index))?,
    )?;
    stage.publish()?;
    Ok(manifest)
}

pub fn load(root: &Path) -> Result<Manifest> {
    let manifest: Manifest = serde_json::from_slice(&read_member(
        root,
        "manifest.json",
        sources::MAX_FILE_BYTES,
    )?)?;
    if manifest.schema != SCHEMA
        || manifest.index.interpretation_sha256 != catalog::interpretation_hash()
    {
        return Err(Error::Invalid(
            "package schema or interpretation is stale; unpack again with this tool".into(),
        ));
    }
    if manifest.snapshots != snapshots(&manifest.index)
        || manifest.exports != exports(&manifest.index)
    {
        return Err(Error::Invalid(
            "package paths or resource descriptors differ from their bound catalog".into(),
        ));
    }
    let known = sources::qualification()?;
    for file in &manifest.index.files {
        if !known.files.contains(file) {
            return Err(Error::UnsupportedRelease(format!(
                "unqualified package source {}",
                file.path
            )));
        }
    }
    if manifest.index.files.is_empty()
        || (manifest.index.is_tree && manifest.index.files != known.files)
        || (!manifest.index.is_tree && manifest.index.files.len() != 1)
    {
        return Err(Error::Invalid(
            "package has an invalid qualified source inventory".into(),
        ));
    }
    let input = Input {
        path: manifest.index.input.clone(),
        is_tree: manifest.index.is_tree,
        files: manifest.index.files.clone(),
    };
    let index = catalog::build_from(&input, |file| {
        let member = format!("snapshots/{}.bin", file.sha256);
        let bytes = read_member(root, &member, sources::MAX_FILE_BYTES)?;
        if bytes.len() as u64 != file.size || sha256_hex(&bytes) != file.sha256 {
            return Err(Error::Invalid(format!(
                "altered original snapshot {}",
                file.path
            )));
        }
        Ok(bytes)
    })?;
    if index != manifest.index {
        return Err(Error::Invalid(
            "package catalog differs from original snapshot interpretation".into(),
        ));
    }
    Ok(manifest)
}

#[derive(Serialize)]
pub struct PackedFile {
    pub source: String,
    pub original_sha256: String,
    pub output_sha256: String,
    pub edited_entries: Vec<usize>,
    pub affected_resources: Vec<String>,
}

#[derive(Serialize)]
pub struct PackReport {
    pub schema: String,
    pub complete: bool,
    pub files: Vec<PackedFile>,
    pub unchanged_reconstruction: String,
    pub edited_reconstruction: String,
    pub unrelated_bytes: String,
    pub runtime: String,
}

pub fn pack(root: &Path, destination: &Path) -> Result<PackReport> {
    let root = fs::canonicalize(root)?;
    let manifest = load(&root)?;
    let stage = Stage::new(destination, &root)?;
    let mut reports = Vec::new();
    for (source, snapshot) in manifest.index.files.iter().zip(&manifest.snapshots) {
        let original = read_member(&root, &snapshot.member, sources::MAX_FILE_BYTES)?;
        if original.len() as u64 != source.size || sha256_hex(&original) != source.sha256 {
            return Err(Error::Invalid(format!(
                "altered original snapshot {}",
                source.path
            )));
        }
        let mut replacements: Vec<(usize, Vec<u8>)> = Vec::new();
        let mut affected = Vec::new();
        for (resource, export) in manifest
            .index
            .resources
            .iter()
            .zip(&manifest.exports)
            .filter(|(resource, _)| resource.source == source.path)
        {
            let raw =
                &original[resource.offset as usize..(resource.offset + resource.size) as usize];
            if let Some(member) = &export.decoded
                && read_member(&root, member, 1024 * 1024)? != bank::decode(raw)?.bytes
            {
                return Err(Error::Invalid(format!(
                    "{member}: decoded bank view is read-only; use qualified sprite.json and indexed PNG views"
                )));
            }
            let mut edit = ByteEdits::new(raw);
            let imported_raw = read_member(&root, &export.raw, resource.size.max(1))?;
            if resource.bank.is_some() && imported_raw != raw {
                return Err(Error::Invalid(format!(
                    "{}: compressed raw bank is read-only; use its qualified editable views",
                    export.raw
                )));
            }
            edit.add(&export.raw, imported_raw)?;
            if let Some(member) = &export.sprite {
                edit.add(member, import_bank(&root, export, raw)?)?;
            }
            if let Some(member) = &export.animation {
                let document: resident::Document =
                    serde_json::from_slice(&read_member(&root, member, 16 * 1024 * 1024)?)?;
                edit.add(member, resident::encode(raw, &document)?)?;
            }
            if let (Some(palette), Some(member)) = (&resource.palette_upload, &export.words) {
                let words: Vec<u16> =
                    serde_json::from_slice(&read_member(&root, member, 16 * 1024 * 1024)?)?;
                edit.add(member, palette.encode_words(&words, raw)?)?;
            }
            if let Some(upload) = &resource.upload {
                if let Some(member) = &export.words {
                    let words: Vec<u16> =
                        serde_json::from_slice(&read_member(&root, member, 16 * 1024 * 1024)?)?;
                    edit.add(member, upload.encode_words(&words, raw)?)?;
                }
                for (bits, member) in [(4, &export.indices_4), (8, &export.indices_8)] {
                    if let Some(member) = member {
                        let original_image = image(resource, raw, bits)?;
                        let imported = png::decode(&read_member(
                            &root,
                            member,
                            (png::MAX_PIXELS * 2 + 65536) as u64,
                        )?)?;
                        if imported.width != original_image.width
                            || imported.height != original_image.height
                            || imported.palette_bgra != original_image.palette_bgra
                        {
                            return Err(Error::Invalid(format!(
                                "{member}: preserve dimensions and diagnostic palette; a game CLUT edit requires a verified source binding"
                            )));
                        }
                        let words = formats::words_from_indices(&imported.indices, bits)?;
                        edit.add(member, upload.encode_words(&words, raw)?)?;
                    }
                }
            }
            let bytes = edit.finish();
            if resource.resident_animation.is_some()
                && resident::encode(raw, &resident::decode(&bytes)?)? != bytes
            {
                return Err(Error::Invalid(format!(
                    "{}: resident edits must preserve all bytes outside existing step pairs and qualified cell records",
                    resource.id
                )));
            }
            if bytes != raw {
                if resource.upload.is_none()
                    && resource.palette_upload.is_none()
                    && resource.resident_animation.is_none()
                    && resource.sprite_bank.is_none()
                {
                    return Err(Error::Invalid(format!(
                        "{} is preservation-only; editable layout is unresolved",
                        resource.id
                    )));
                }
                let entry = resource
                    .entry
                    .ok_or_else(|| Error::Invalid("non-archive edit is unresolved".into()))?;
                replacements.push((entry, bytes));
                affected.push(resource.id.clone());
            }
        }
        let output = if source.emi_entries.is_some() {
            let archive = ArchiveImage::from_bytes(original.clone())?;
            let borrowed: Vec<_> = replacements
                .iter()
                .map(|(slot, bytes)| (*slot, bytes.as_slice()))
                .collect();
            let output = archive.replace_entries(&borrowed)?;
            for (slot, intended) in &replacements {
                if output.entry(*slot)? != intended {
                    return Err(Error::Invalid(format!(
                        "{} entry {slot}: encoded output mismatch",
                        source.path
                    )));
                }
            }
            verify_unrelated(&archive, &output, &replacements)?;
            output.bytes().to_vec()
        } else {
            original.clone()
        };
        if replacements.is_empty() && output != original {
            return Err(Error::Invalid(format!(
                "{} unchanged reconstruction differs",
                source.path
            )));
        }
        stage.write(&source.path, &output)?;
        reports.push(PackedFile {
            source: source.path.clone(),
            original_sha256: source.sha256.clone(),
            output_sha256: sha256_hex(&output),
            edited_entries: replacements.iter().map(|(slot, _)| *slot).collect(),
            affected_resources: affected,
        });
    }
    let edited = reports.iter().any(|file| !file.edited_entries.is_empty());
    let report = PackReport {
        schema: "bof3.textures-pack-report/v1".into(),
        complete: true,
        files: reports,
        unchanged_reconstruction: if edited {
            "not-applicable-to-edited-files"
        } else {
            "byte-identical"
        }
        .into(),
        edited_reconstruction: if edited {
            "source-bytes-and-editable-views-verified"
        } else {
            "unobserved"
        }
        .into(),
        unrelated_bytes: "byte-identical".into(),
        runtime: "unobserved".into(),
    };
    stage.write(
        "textures-pack-report.json",
        &serde_json::to_vec_pretty(&report)?,
    )?;
    stage.publish()?;
    Ok(report)
}

fn verify_unrelated(
    original: &ArchiveImage,
    output: &ArchiveImage,
    replacements: &[(usize, Vec<u8>)],
) -> Result<()> {
    if original.bytes().len() != output.bytes().len() {
        return Err(Error::Invalid("archive allocation changed".into()));
    }
    let mut permitted = BTreeSet::new();
    for (slot, payload) in replacements {
        let entry = &original.entries()[*slot];
        permitted.extend(
            (entry.offset as usize..entry.offset as usize + payload.len())
                .filter(|offset| original.bytes()[*offset] != output.bytes()[*offset]),
        );
        // The EMI owner may update the cached leading word for an edited payload.
        permitted.extend(0x10 + slot * 0x10 + 8..0x10 + slot * 0x10 + 12);
    }
    if let Some(offset) = original
        .bytes()
        .iter()
        .zip(output.bytes())
        .enumerate()
        .find_map(|(offset, (a, b))| (a != b && !permitted.contains(&offset)).then_some(offset))
    {
        return Err(Error::Invalid(format!(
            "packing altered unrelated archive byte at 0x{offset:x}"
        )));
    }
    Ok(())
}

struct ByteEdits<'a> {
    original: &'a [u8],
    changes: BTreeMap<usize, (u8, String)>,
}

impl<'a> ByteEdits<'a> {
    fn new(original: &'a [u8]) -> Self {
        Self {
            original,
            changes: BTreeMap::new(),
        }
    }

    fn add(&mut self, member: &str, replacement: Vec<u8>) -> Result<()> {
        if replacement.len() != self.original.len() {
            return Err(Error::Invalid(format!(
                "{member}: edit changes original allocation"
            )));
        }
        for (offset, (old, new)) in self.original.iter().zip(replacement).enumerate() {
            if *old == new {
                continue;
            }
            if let Some((other, owner)) = self.changes.get(&offset)
                && *other != new
            {
                return Err(Error::Conflict(format!(
                    "{owner} and {member} disagree at source byte 0x{offset:x}"
                )));
            }
            self.changes.insert(offset, (new, member.into()));
        }
        Ok(())
    }

    fn finish(self) -> Vec<u8> {
        let mut output = self.original.to_vec();
        for (offset, (byte, _)) in self.changes {
            output[offset] = byte;
        }
        output
    }
}

pub fn read_member(root: &Path, member: &str, limit: u64) -> Result<Vec<u8>> {
    let relative = safe_relative(member)?;
    let mut path = root.to_path_buf();
    for component in relative.components() {
        path.push(component);
        if fs::symlink_metadata(&path)?.file_type().is_symlink() {
            return Err(Error::Invalid(format!(
                "package member {member} traverses a symlink"
            )));
        }
    }
    sources::read_bounded(&path, limit)
}

fn safe_relative(member: &str) -> Result<&Path> {
    let path = Path::new(member);
    if member.is_empty()
        || member.contains('\\')
        || !path
            .components()
            .all(|part| matches!(part, Component::Normal(_)))
    {
        return Err(Error::Invalid(format!("unsafe package member {member:?}")));
    }
    Ok(path)
}

struct Stage {
    temporary: PathBuf,
    destination: PathBuf,
}

impl Stage {
    fn new(destination: &Path, input: &Path) -> Result<Self> {
        if destination.try_exists()? || fs::symlink_metadata(destination).is_ok() {
            return Err(Error::Invalid(format!(
                "destination {} already exists",
                destination.display()
            )));
        }
        let parent = destination
            .parent()
            .filter(|path| !path.as_os_str().is_empty())
            .unwrap_or(Path::new("."));
        let parent = fs::canonicalize(parent)?;
        let name = destination
            .file_name()
            .ok_or_else(|| Error::Invalid("destination requires a directory name".into()))?;
        if name == "." || name == ".." {
            return Err(Error::Invalid("invalid destination name".into()));
        }
        let destination = parent.join(name);
        let input = fs::canonicalize(input)?;
        if destination.starts_with(&input) || input.starts_with(&destination) {
            return Err(Error::Invalid("input/output overlap is refused".into()));
        }
        let nonce = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map_err(|_| Error::Invalid("invalid host clock".into()))?
            .as_nanos();
        let temporary = parent.join(format!(".bof3-textures-{}-{nonce}", std::process::id()));
        fs::create_dir(&temporary)?;
        Ok(Self {
            temporary,
            destination,
        })
    }

    fn write(&self, member: &str, bytes: &[u8]) -> Result<()> {
        let path = self.temporary.join(safe_relative(member)?);
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent)?;
        }
        fs::write(path, bytes)?;
        Ok(())
    }

    fn publish(self) -> Result<()> {
        // GNU mv uses no-replace rename for -n. Both paths share a parent/filesystem;
        // --no-copy refuses any fallback that could publish a partial tree.
        let status = Command::new("mv")
            .args(["--no-clobber", "--no-copy", "--no-target-directory", "--"])
            .arg(&self.temporary)
            .arg(&self.destination)
            .status()?;
        if !status.success() || self.temporary.try_exists()? {
            return Err(Error::Invalid(
                "destination publication refused; no source data was changed".into(),
            ));
        }
        Ok(())
    }

    fn publish_file(self, member: &str) -> Result<()> {
        fs::hard_link(
            self.temporary.join(safe_relative(member)?),
            &self.destination,
        )?;
        Ok(())
    }
}

impl Drop for Stage {
    fn drop(&mut self) {
        let _ = fs::remove_dir_all(&self.temporary);
    }
}

pub fn publish_json(destination: &Path, input: &Path, value: &impl Serialize) -> Result<()> {
    let stage = Stage::new(destination, input)?;
    stage.write("document.json", &serde_json::to_vec_pretty(value)?)?;
    stage.publish_file("document.json")
}

/// Package authored build inputs. Archive publication never replaces a file.
pub fn source_package(root: &Path, destination: &Path) -> Result<usize> {
    let root = fs::canonicalize(root)?;
    let crate_root = root.join("tools/rust/bof3-textures");
    let stage = Stage::new(destination, &crate_root)?;
    if stage
        .destination
        .starts_with(root.join("tools/rust/emi-ex"))
    {
        return Err(Error::Invalid(
            "source-package destination overlaps the EMI crate".into(),
        ));
    }
    let mut members = vec![PathBuf::from("config/original-us-disc.json")];
    for name in ["bof3-textures", "emi-ex"] {
        let relative = PathBuf::from(format!("tools/rust/{name}"));
        let source = root.join(&relative);
        for required in ["Cargo.toml", "Cargo.lock", "README.md"] {
            members.push(relative.join(required));
        }
        collect_rust(&root, &relative.join("src"), &mut members)?;
        if name == "bof3-textures" {
            for file in ["LICENSE", "mise.toml", "rust-toolchain.toml"] {
                members.push(relative.join(file));
            }
        }
        if source.is_symlink() {
            return Err(Error::Invalid(format!(
                "source crate {} is a symlink",
                source.display()
            )));
        }
    }
    members.sort();
    let content = stage.temporary.join("content");
    fs::create_dir(&content)?;
    for member in &members {
        let bytes = read_member(&root, &member.to_string_lossy(), 16 * 1024 * 1024)?;
        let path = content.join("bof3-textures").join(member);
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent)?;
        }
        fs::write(path, bytes)?;
    }
    let archive = stage.temporary.join("source.zip");
    let files: Vec<_> = members
        .iter()
        .map(|member| Path::new("bof3-textures").join(member))
        .collect();
    let status = Command::new("zip")
        .args(["-X", "-q"])
        .arg(&archive)
        .args(&files)
        .current_dir(&content)
        .status()?;
    if !status.success() {
        return Err(Error::Invalid(
            "source ZIP creation failed; installed zip is required".into(),
        ));
    }
    let status = Command::new("unzip")
        .args(["-t", "-qq"])
        .arg(&archive)
        .status()?;
    if !status.success() {
        return Err(Error::Invalid("source ZIP integrity check failed".into()));
    }
    stage.publish_file("source.zip")?;
    Ok(members.len())
}

fn collect_rust(root: &Path, relative: &Path, members: &mut Vec<PathBuf>) -> Result<()> {
    let path = root.join(relative);
    if fs::symlink_metadata(&path)?.file_type().is_symlink() {
        return Err(Error::Invalid(format!(
            "source input {} is a symlink",
            path.display()
        )));
    }
    for entry in fs::read_dir(path)? {
        let entry = entry?;
        let relative = relative.join(entry.file_name());
        let kind = entry.file_type()?;
        if kind.is_symlink() {
            return Err(Error::Invalid(format!(
                "source input {} is a symlink",
                relative.display()
            )));
        }
        if kind.is_dir() {
            collect_rust(root, &relative, members)?;
        }
        if kind.is_file()
            && relative
                .extension()
                .is_some_and(|extension| extension == "rs")
        {
            members.push(relative);
        }
    }
    Ok(())
}
