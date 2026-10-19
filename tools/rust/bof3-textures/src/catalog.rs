use std::path::{Path, PathBuf};

use emi_ex_v2::image::ArchiveImage;
use serde::{Deserialize, Serialize};

use crate::{
    Error, Result, bank,
    digest::sha256_hex,
    formats::Upload,
    palette::PaletteUpload,
    record::{self, CoordinateSpace, Record},
    resident, save_icon,
    sources::{self, Input, SourceFile},
    sprite_bank, stream_family,
};

pub const SCHEMA: &str = "bof3.textures-index/v5";
pub const MAX_INDEX_BYTES: u64 = 512 * 1024 * 1024;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Category {
    pub id: String,
    pub description: String,
}

pub fn categories() -> Vec<Category> {
    [
        (
            "field-character",
            "Field party figures, ages, actions, and directions",
        ),
        ("npc", "Non-player figures, masters, and fairies"),
        (
            "battle-character",
            "Party battle figures and personal transformations",
        ),
        ("enemy", "Regular enemy figures"),
        ("boss", "Boss and multipart figures"),
        ("dragon", "Dragon forms and shared transformations"),
        (
            "area-terrain",
            "Area and battle-arena terrain and mesh surfaces",
        ),
        ("world-map", "World terrain, overlays, and location markers"),
        ("object-decoration", "Props and sprite or mesh decorations"),
        (
            "animated-environment",
            "Environment uploads, scrolling, and palette changes",
        ),
        (
            "textured-effect",
            "Textured battle, spell, and field effects",
        ),
        ("font", "Glyphs, digits, and symbols"),
        (
            "interface",
            "Windows, icons, cursors, labels, and HUD graphics",
        ),
        ("portrait", "Portraits and their selection tables"),
        (
            "minigame",
            "Fishing, fairy-village, casino, and other minigame graphics",
        ),
        (
            "system-screen",
            "Title, warning, game-over, attract, and ending imagery",
        ),
        ("save-icon", "Memory-card icon frames and palettes"),
        (
            "unclassified-graphics",
            "Graphics whose game role is unresolved",
        ),
    ]
    .into_iter()
    .map(|(id, description)| Category {
        id: id.into(),
        description: description.into(),
    })
    .collect()
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Dependency {
    pub kind: String,
    pub target: Option<String>,
    pub status: String,
    pub reason: String,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Resource {
    pub id: String,
    pub source: String,
    pub entry: Option<usize>,
    pub offset: u64,
    pub size: u64,
    pub sha256: String,
    pub emi_type: Option<u16>,
    pub load_argument: Option<u32>,
    pub kind: String,
    pub categories: Vec<String>,
    pub storage: String,
    pub decode: String,
    pub pack: String,
    pub evidence_status: String,
    pub evidence: Vec<String>,
    pub dependencies: Vec<Dependency>,
    pub uses: Vec<String>,
    pub upload: Option<Upload>,
    pub palette_upload: Option<PaletteUpload>,
    pub bank: Option<bank::Info>,
    pub sprite_bank: Option<sprite_bank::Info>,
    pub stream_request: Option<stream_family::Request>,
    pub resident_animation: Option<resident::Info>,
    pub save_icon: Option<save_icon::Info>,
    pub findings: Vec<String>,
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Index {
    pub schema: String,
    pub release: String,
    pub input: PathBuf,
    pub is_tree: bool,
    pub interpretation_sha256: String,
    pub inventory_sha256: String,
    pub files: Vec<SourceFile>,
    pub categories: Vec<Category>,
    pub resources: Vec<Resource>,
    pub records: Vec<Record>,
}

pub fn interpretation_hash() -> String {
    sha256_hex(
        [
            SCHEMA,
            sources::QUALIFICATION,
            include_str!("catalog.rs"),
            include_str!("record.rs"),
            include_str!("formats.rs"),
            include_str!("package.rs"),
            include_str!("png.rs"),
            include_str!("preview.rs"),
            include_str!("sources.rs"),
            include_str!("palette.rs"),
            include_str!("packed.rs"),
            include_str!("lzss.rs"),
            include_str!("bank.rs"),
            include_str!("bank_image.rs"),
            include_str!("animation.rs"),
            include_str!("resident.rs"),
            include_str!("cell.rs"),
            include_str!("sprite_bank.rs"),
            include_str!("stream_family.rs"),
            include_str!("aliases.rs"),
            include_str!("interface.rs"),
            include_str!("save_icon.rs"),
            include_str!("verification.rs"),
        ]
        .join("\n")
        .as_bytes(),
    )
}

fn annotate_sprite(resource: &mut Resource, document: &sprite_bank::Document, storage: &str) {
    resource.kind = "sprite-bank".into();
    resource.categories = vec!["unclassified-graphics".into()];
    resource.storage = storage.into();
    resource.decode = "images-palettes-programs-and-cells-verified".into();
    resource.pack = "fixed-record-edits-verified".into();
    resource.sprite_bank = Some(document.info());
    resource.evidence.extend([
        "BIN/ETC/GAME.EMI#0@0x801C1254:three-root-selection".into(),
        "SLUS_004.22@0x8014DC08:bank-selection-consumer".into(),
        "SLUS_004.22@0x8014E6D0:image-records".into(),
        "SLUS_004.22@0x80166C48:32-word-palette-copy".into(),
        "SLUS_004.22@0x8014C6F8:five-byte-cell-parts".into(),
    ]);
    for (kind, reason) in [
        (
            "selection",
            "valid selection indices and skipped-upload states remain unresolved",
        ),
        (
            "palette",
            "32-word records do not establish complete draw CLUTs or neighboring colors",
        ),
        (
            "texture-page",
            "runtime page, resident uploads and consumer state remain unresolved",
        ),
        (
            "game-role",
            "filename families do not establish game categories",
        ),
    ] {
        resource.dependencies.push(Dependency {
            kind: kind.into(),
            target: None,
            status: "unresolved".into(),
            reason: reason.into(),
        });
    }
}

pub fn build(input: &Input) -> Result<Index> {
    build_from(input, |file| {
        sources::verify_source(&input.source_path(file), file)
    })
}

pub fn build_from(
    input: &Input,
    mut read: impl FnMut(&SourceFile) -> Result<Vec<u8>>,
) -> Result<Index> {
    let mut resources = Vec::new();
    let mut records = Vec::new();
    let qualification = sources::qualification()?;
    for file in &input.files {
        let bytes = read(file)?;
        if !qualification.files.contains(file)
            || bytes.len() as u64 != file.size
            || sha256_hex(&bytes) != file.sha256
        {
            return Err(Error::Invalid(format!(
                "{} is not an unchanged qualified original US source",
                file.path
            )));
        }
        if let Some(count) = file.emi_entries {
            let image = ArchiveImage::from_bytes(bytes)?;
            if image.version() != 1 || image.entries().len() != count {
                return Err(Error::Invalid(format!(
                    "{} disagrees with the original EMI inventory",
                    file.path
                )));
            }
            for (slot, entry) in image.entries().iter().enumerate() {
                let payload = image.entry(slot)?;
                let mut resource = Resource {
                    id: format!("{}:{}#entry-{slot:03}", sources::RELEASE, file.path),
                    source: file.path.clone(),
                    entry: Some(slot),
                    offset: entry.offset,
                    size: u64::from(entry.size),
                    sha256: sha256_hex(payload),
                    emi_type: Some(entry.file_type),
                    load_argument: Some(entry.ram_ptr),
                    kind: "unresolved-entry".into(),
                    categories: vec![],
                    storage: "opaque".into(),
                    decode: "unresolved".into(),
                    pack: "preservation-only".into(),
                    evidence_status: "source-qualified".into(),
                    evidence: vec!["qualified-us-disc".into()],
                    dependencies: vec![],
                    uses: vec![],
                    upload: None,
                    palette_upload: None,
                    bank: None,
                    sprite_bank: None,
                    stream_request: None,
                    resident_animation: None,
                    save_icon: None,
                    findings: vec!["logical records and consumers have not been enumerated".into()],
                };
                if let Some(storage) = match entry.file_type {
                    6 => Some("vab-header"),
                    7 => Some("vab-body"),
                    8 => Some("audio-auxiliary-container"),
                    10 => Some("sep-sequence-container"),
                    _ => None,
                } {
                    resource.kind = "audio-owned-container".into();
                    resource.storage = storage.into();
                    resource.decode = "delegated-to-audio".into();
                    resource.evidence_status = "owner-qualified".into();
                    resource.evidence.push(
                        "tools/rust/bof3-audio/src/catalog/mod.rs:original-us-container-ownership"
                            .into(),
                    );
                    if entry.file_type == 8 {
                        resource
                            .evidence
                            .push("SLUS_004.22@0x801629F0:audio-auxiliary-destination".into());
                    }
                    resource.findings.clear();
                }
                if entry.file_type == 0
                    && let Some(info) =
                        save_icon::qualify(&file.path, slot, entry.ram_ptr, payload)?
                {
                    let images = save_icon::decode(payload, &info)?;
                    if save_icon::encode(payload, &info, &images)? != payload {
                        return Err(Error::Invalid(format!(
                            "{} save-icon inverse mismatch",
                            resource.id
                        )));
                    }
                    resource.kind = "save-icon-source".into();
                    resource.categories = vec!["save-icon".into(), "unclassified-graphics".into()];
                    resource.storage = "embedded-4bit-save-icons".into();
                    resource.decode = "three-index-planes-and-palette-verified".into();
                    resource.pack = "fixed-image-ranges-verified".into();
                    resource.evidence.extend([
                        "BIN/ETC/SHOP.EMI#8@0x801EF27C:save-header-producer".into(),
                        "BIN/ETC/SHOP.EMI#0@0x801D6A08:three-frame-mode-in-delay-slot".into(),
                        "BIN/ETC/SHOP.EMI#8@0x801EFE68:palette".into(),
                        "BIN/ETC/SHOP.EMI#8@0x801EFE94:frame-0".into(),
                        "BIN/ETC/SHOP.EMI#8@0x801EFF54:frame-1".into(),
                        "BIN/ETC/SHOP.EMI#8@0x801F0014:frame-2".into(),
                    ]);
                    resource.dependencies.push(Dependency {
                        kind: "palette".into(), target: Some(resource.id.clone()),
                        status: "verified".into(),
                        reason: "16 words at payload offset 0x1268 are copied to header +0x60; three 16x16 index planes are copied to +0x80/+0x100/+0x180".into(),
                    });
                    resource.dependencies.push(Dependency {
                        kind: "save-header-consumer".into(), target: None,
                        status: "unresolved".into(),
                        reason: "generated original save header, physical source loading, BIOS animation timing and edited display have not been observed".into(),
                    });
                    resource
                        .uses
                        .push("save-header:0x800C1800:mode-0x13".into());
                    resource.save_icon = Some(info);
                    if let Some(info) = resource.save_icon.as_ref() {
                        records.extend(record::save_icon_records(&resource, payload, info)?);
                    }
                }
                if entry.file_type == 0
                    && let Some(palette) =
                        PaletteUpload::from_argument(entry.ram_ptr, payload.len())
                {
                    let words = palette.decode_words(payload)?;
                    if palette.encode_words(&words, payload)? != payload {
                        return Err(Error::Invalid(format!(
                            "{} palette inverse mismatch",
                            resource.id
                        )));
                    }
                    resource.kind = "palette-bank".into();
                    resource.categories = vec!["unclassified-graphics".into()];
                    resource.storage = "psx-palette-words".into();
                    resource.decode = "palette-words-verified".into();
                    resource.pack = "palette-words-verified".into();
                    resource.evidence.extend([
                        "SLUS_004.22@0x80162BAC:entry-destination".into(),
                        "SLUS_004.22@0x801625E4:type0-sector-transfer".into(),
                        "SLUS_004.22@0x80177154:cdrom-dma-words".into(),
                        "SLUS_004.22@0x8014E284:palette-staging".into(),
                        "SLUS_004.22@0x8014E22C:palette-upload".into(),
                    ]);
                    resource.dependencies.push(Dependency {
                        kind: "consumer-selection".into(),
                        target: None,
                        status: "unresolved".into(),
                        reason: "draw CLUT selection and texture associations are unresolved"
                            .into(),
                    });
                    resource.palette_upload = Some(palette);
                }
                if entry.file_type == 0 && entry.ram_ptr == 0x800d_3800 {
                    match resident::decode(payload) {
                        Ok(document) => {
                            resource.kind = "resident-animation-bank".into();
                            resource.categories = vec!["unclassified-graphics".into()];
                            resource.storage = "us-resident-animation".into();
                            resource.decode = "forward-programs-and-cell-records-verified".into();
                            resource.pack = "forward-steps-and-cell-records-verified".into();
                            resource.resident_animation = Some(document.info());
                            records
                                .extend(record::resident_records(&resource, payload, &document)?);
                            resource.evidence.extend([
                                "SLUS_004.22@0x8014EBB8:resident-base-initialization".into(),
                                "SLUS_004.22@0x8014D878:resident-program-selection".into(),
                                "SLUS_004.22@0x8014D910:program-initialization".into(),
                                "SLUS_004.22@0x8014D978:forward-loop-consumer".into(),
                                "SLUS_004.22@0x8014DAEC:forward-stop-consumer".into(),
                                "SLUS_004.22@0x8014C6F8:cell-part-count".into(),
                                "SLUS_004.22@0x8014C700:five-byte-cell-parts".into(),
                                "SLUS_004.22@0x8017F53C:cell-dimensions".into(),
                            ]);
                            for kind in ["texture-source", "palette", "reverse-animation"] {
                                resource.dependencies.push(Dependency {
                                    kind: kind.into(),
                                    target: None,
                                    status: "unresolved".into(),
                                    reason: "image, CLUT, reverse playback and game-role bindings require separate qualification".into(),
                                });
                            }
                        }
                        Err(error) => resource.findings.push(error.to_string()),
                    }
                }
                if entry.file_type == 1 && entry.ram_ptr == 0x8003_b800 {
                    let decoded = bank::decode(payload)?;
                    resource.kind = "compressed-bank".into();
                    resource.categories = vec!["unclassified-graphics".into()];
                    resource.storage = "us-disc-bank-lz".into();
                    resource.decode = "outer-bank-bytes-verified".into();
                    resource.bank = Some(decoded.info);
                    resource.evidence.extend([
                        "SLUS_004.22@0x80162618:type1-destination".into(),
                        "SLUS_004.22@0x80163238:bank-decoder-initialization".into(),
                        "SLUS_004.22@0x80164064:outer-bank-tokens".into(),
                        "SLUS_004.22@0x801642F4:forward-overlapping-copy".into(),
                    ]);
                    match sprite_bank::decode(&decoded.bytes) {
                        Ok(document) => {
                            annotate_sprite(&mut resource, &document, "us-three-root-sprite-bank");
                            records.extend(record::sprite_records(
                                &resource,
                                &decoded.bytes,
                                CoordinateSpace::DecodedBank,
                                &document,
                            )?);
                        }
                        Err(error) => {
                            resource.dependencies.push(Dependency {
                                kind: "logical-layout".into(),
                                target: None,
                                status: "unresolved".into(),
                                reason: error.to_string(),
                            });
                            resource.findings.push(
                                "decoded bank bytes are read-only; internal variant is unresolved"
                                    .into(),
                            );
                        }
                    }
                }
                if file.path == "BIN/BPLCHAR/BPLU349.EMI"
                    && slot == 0
                    && entry.file_type == 0
                    && entry.ram_ptr == 0x8003_b800
                    && resource.sha256
                        == "3633597f03d7f2129ff7d7fc1ad0fe7c2e6a0affb2562e789c4a0b97b5d14b3e"
                {
                    let document = sprite_bank::decode(payload)?;
                    annotate_sprite(&mut resource, &document, "us-raw-three-root-sprite-bank");
                    records.extend(record::sprite_records(
                        &resource,
                        payload,
                        CoordinateSpace::EntryPayload,
                        &document,
                    )?);
                    resource.evidence.extend([
                        "SLUS_004.22@0x801622A8:type0-dispatch".into(),
                        "SLUS_004.22@0x801625E4:direct-sector-copy".into(),
                        "SLUS_004.22@0x80162C14:bounded-copy-destination".into(),
                    ]);
                }
                if entry.file_type == 3 {
                    resource.kind = "texture-upload".into();
                    resource.categories = vec!["unclassified-graphics".into()];
                    resource.storage = "raw-vram-chunks".into();
                    resource
                        .evidence
                        .push("SLUS_004.22@0x80162698:packed-dispatch".into());
                    resource
                        .evidence
                        .push("SLUS_004.22@0x80163378:bounded-chunk-submission".into());
                    match Upload::from_argument(entry.ram_ptr, payload.len()) {
                        Ok(upload) => {
                            let words = upload.linear_words(payload)?;
                            if upload.encode_words(&words, payload)? != payload {
                                return Err(Error::Invalid(format!(
                                    "{} raw inverse mismatch",
                                    resource.id
                                )));
                            }
                            resource.upload = Some(upload);
                            resource.decode = "raw-words-verified".into();
                            resource.pack = "raw-words-verified".into();
                        }
                        Err(error) => resource.findings.push(error.to_string()),
                    }
                    for (kind, reason) in [
                        (
                            "palette",
                            "CLUT association and palette source are unresolved",
                        ),
                        (
                            "texture-page",
                            "draw-mode and texture-page consumers are unresolved",
                        ),
                        (
                            "resident-base",
                            "resident upload dependencies are unresolved",
                        ),
                        (
                            "upload-order",
                            "cross-entry loader ordering has not been qualified",
                        ),
                    ] {
                        resource.dependencies.push(Dependency {
                            kind: kind.into(),
                            target: None,
                            status: "unresolved".into(),
                            reason: reason.into(),
                        });
                    }
                }
                resources.push(resource);
            }
        } else {
            let findings = vec![
                "embedded, generated, or media-owned graphics disposition is unresolved".into(),
            ];
            resources.push(Resource {
                id: format!("{}:{}#file", sources::RELEASE, file.path),
                source: file.path.clone(),
                entry: None,
                offset: 0,
                size: file.size,
                sha256: file.sha256.clone(),
                emi_type: None,
                load_argument: None,
                kind: "unresolved-file".into(),
                categories: vec![],
                storage: "opaque".into(),
                decode: "unresolved".into(),
                pack: "preservation-only".into(),
                evidence_status: "source-qualified".into(),
                evidence: vec!["qualified-us-disc".into()],
                dependencies: vec![],
                uses: vec![],
                upload: None,
                palette_upload: None,
                bank: None,
                sprite_bank: None,
                stream_request: None,
                resident_animation: None,
                save_icon: None,
                findings,
            });
        }
    }
    crate::interface::annotate(&mut resources)?;
    stream_family::annotate(&mut resources);
    Ok(Index {
        schema: SCHEMA.into(),
        release: sources::RELEASE.into(),
        input: input.path.clone(),
        is_tree: input.is_tree,
        interpretation_sha256: interpretation_hash(),
        inventory_sha256: sha256_hex(&serde_json::to_vec(&input.files)?),
        files: input.files.clone(),
        categories: categories(),
        resources,
        records,
    })
}

impl Index {
    pub fn load(path: &Path) -> Result<Self> {
        let index: Self = serde_json::from_slice(&sources::read_bounded(
            path,
            MAX_INDEX_BYTES,
        )?)
        .map_err(|error| {
            if error.is_data() {
                return Error::StaleIndex(format!(
                    "catalog fields do not match the supported schema: {error}; rebuild with textures index ORIGINAL_INPUT --output NEW_INDEX"
                ));
            }
            Error::from(error)
        })?;
        index.check_fresh()?;
        Ok(index)
    }

    pub fn check_fresh(&self) -> Result<()> {
        let stale = |reason| {
            Error::StaleIndex(format!(
                "{reason}; rebuild with textures index {} --output NEW_INDEX",
                self.input.display()
            ))
        };
        if self.schema != SCHEMA
            || self.release != sources::RELEASE
            || self.interpretation_sha256 != interpretation_hash()
        {
            return Err(stale(
                "index schema, policy, format, or mapping evidence changed",
            ));
        }
        let source = (!self.is_tree)
            .then(|| self.files.first().map(|file| file.path.as_str()))
            .flatten();
        let input = sources::qualify(&self.input, source)
            .map_err(|_| stale("source inventory or bytes changed"))?;
        let fresh = build(&input).map_err(|_| stale("source layout changed"))?;
        if self != &fresh {
            return Err(stale(
                "catalog content differs from captured sources and current interpretation",
            ));
        }
        Ok(())
    }
}

#[derive(Default)]
pub struct Filter<'a> {
    pub category: Option<&'a str>,
    pub source: Option<&'a str>,
    pub asset: Option<&'a str>,
    pub storage: Option<&'a str>,
    pub kind: Option<&'a str>,
    pub palette: Option<&'a str>,
    pub animation: Option<&'a str>,
    pub dependency: Option<&'a str>,
    pub evidence: Option<&'a str>,
}

pub struct QueryFields<'a> {
    pub(crate) id: &'a str,
    pub(crate) source: &'a str,
    pub(crate) categories: &'a [String],
    pub(crate) storage: &'a str,
    pub(crate) kind: &'a str,
    pub(crate) evidence_status: &'a str,
    pub(crate) dependencies: &'a [Dependency],
    pub(crate) is_palette: bool,
    pub(crate) is_animation: bool,
}

pub trait Queryable {
    fn query_fields(&self) -> QueryFields<'_>;
}

impl Queryable for Resource {
    fn query_fields(&self) -> QueryFields<'_> {
        QueryFields {
            id: &self.id,
            source: &self.source,
            categories: &self.categories,
            storage: &self.storage,
            kind: &self.kind,
            evidence_status: &self.evidence_status,
            dependencies: &self.dependencies,
            is_palette: self.palette_upload.is_some(),
            is_animation: self.resident_animation.is_some()
                || self.sprite_bank.is_some()
                || self.save_icon.is_some(),
        }
    }
}

impl Filter<'_> {
    pub fn matches(&self, resource: &impl Queryable) -> bool {
        let resource = resource.query_fields();
        let has_reference = |kind: &str, target: &str| {
            resource
                .dependencies
                .iter()
                .any(|edge| edge.kind == kind && edge.target.as_deref() == Some(target))
        };
        self.category
            .is_none_or(|value| resource.categories.iter().any(|category| category == value))
            && self
                .source
                .is_none_or(|value| resource.source.contains(value))
            && self.asset.is_none_or(|value| resource.id == value)
            && self.storage.is_none_or(|value| resource.storage == value)
            && self.kind.is_none_or(|value| resource.kind == value)
            && self
                .evidence
                .is_none_or(|value| resource.evidence_status == value)
            && self.dependency.is_none_or(|value| {
                resource
                    .dependencies
                    .iter()
                    .any(|edge| edge.kind == value || edge.target.as_deref() == Some(value))
            })
            && self.palette.is_none_or(|value| {
                (resource.is_palette && resource.id == value) || has_reference("palette", value)
            })
            && self.animation.is_none_or(|value| {
                (resource.is_animation && resource.id == value) || has_reference("animation", value)
            })
    }
}

pub fn coverage(index: &Index) -> serde_json::Value {
    let raw = index
        .resources
        .iter()
        .filter(|resource| resource.upload.is_some())
        .count();
    let record_kinds = ["bank-image", "palette-record", "animation-program", "sprite-cell", "save-icon-frame"].into_iter().map(|kind| serde_json::json!({"kind": kind, "records": index.records.iter().filter(|record| record.kind == kind).count()})).collect::<Vec<_>>();
    let blockers = vec![
        "logical resource and consumer enumeration is incomplete",
        "category mappings and dependencies are unresolved",
        "edited and runtime acceptance remains incomplete",
    ];
    serde_json::json!({
        "schema": "bof3.textures-coverage/v2", "release": index.release,
        "full_support": false,
        "physical": {"resources": index.resources.len(), "files": index.files.len(), "emi_entries": index.files.iter().filter_map(|file| file.emi_entries).sum::<usize>(),
            "qualified_original_corpus": index.is_tree,
            "inventory_matches_qualification": true,
            "inventory_sha256": index.inventory_sha256},
        "logical": {"complete": false, "records": index.records.len(),
            "audio_owned_containers": index.resources.iter().filter(|resource| resource.kind == "audio-owned-container").count(),
            "audio_owned_container_bytes": index.resources.iter().filter(|resource| resource.kind == "audio-owned-container").map(|resource| resource.size).sum::<u64>(),
            "record_kinds": record_kinds,
            "record_byte_accounting": "logical extents can overlap; physical bytes are counted only by the input inventory",
            "raw_uploads": raw,
            "palette_banks": index.resources.iter().filter(|resource| resource.palette_upload.is_some()).count(),
            "decoded_compressed_banks": index.resources.iter().filter(|resource| resource.bank.is_some()).count(),
            "sprite_banks": index.resources.iter().filter(|resource| resource.sprite_bank.is_some()).count(),
            "resident_animation_banks": index.resources.iter().filter(|resource| resource.resident_animation.is_some()).count(),
            "save_icon_sources": index.resources.iter().filter(|resource| resource.save_icon.is_some()).count(),
            "unresolved_resources": index.resources.iter().filter(|resource| !resource.findings.is_empty()).map(|resource| &resource.id).collect::<Vec<_>>()},
        "categories": index.categories.iter().map(|category| serde_json::json!({"id": category.id, "disposition": "unresolved", "evidence": []})).collect::<Vec<_>>(),
        "unresolved_dependencies": index.resources.iter().flat_map(|resource| resource.dependencies.iter().filter(|edge| edge.status != "verified").map(|edge| serde_json::json!({"resource": resource.id, "dependency": edge})))
            .chain(index.records.iter().flat_map(|record| record.dependencies.iter().filter(|edge| edge.status != "verified").map(|edge| serde_json::json!({"resource": record.id, "dependency": edge})))).collect::<Vec<_>>(),
        "runtime": {"status": "unobserved", "consumer_families": []},
        "blockers": blockers
    })
}
