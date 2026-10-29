//! Source associations for qualified field consumer observations.

use crate::{
    Error, Result,
    catalog::{Dependency, Resource},
    record::{CoordinateSpace, Record},
    sprite_bank::{self, SelectionEdit},
    stream_family,
};

pub(crate) fn has_selections(resource: &Resource) -> bool {
    resource.load_argument == Some(0x8003b800)
        && matches!(resource.emi_type, Some(0 | 1))
        && resource
            .sprite_bank
            .as_ref()
            .is_some_and(|bank| bank.roots == 3)
        && stream_family::request(&resource.source).is_some_and(|request| request.family == 0)
}

pub(crate) fn observed_selection(resource: &Resource, slot: &SelectionEdit) -> bool {
    resource.source == "BIN/PLCHAR/PL27A.EMI"
        && resource.entry == Some(3)
        && resource.offset == 73728
        && resource.size == 239772
        && resource.load_argument == Some(0x8003b800)
        && resource.sha256 == "396ae98f20eefc3212ad657b9be8787db3445cf97fe80160baab1c5a21e8e5c1"
        && slot.root == 2
        && slot.selector == 5
        && !slot.skip_image
}

/// Conditional GAME route slots, with a separately captured PL27A context.
/// The image-enabled view validates references without asserting entity state.
pub(crate) fn selections(resource: &Resource, decoded: &[u8]) -> Result<Vec<SelectionEdit>> {
    if !has_selections(resource) {
        return Ok(Vec::new());
    }
    let document = sprite_bank::decode(decoded)?;
    if document.roots.len() != 3 {
        return Err(Error::Invalid(
            "field selection bank must have three roots".into(),
        ));
    }
    let mut slots = Vec::with_capacity(24);
    for (root, record) in document.roots.iter().enumerate() {
        for selector in 0..8 {
            let selected = sprite_bank::select_from(decoded, record, selector, false)?;
            let slot = SelectionEdit {
                root,
                selector,
                skip_image: false,
                mirror_flag: selected.mirror_flag,
            };
            if observed_selection(resource, &slot)
                && (decoded.len() != 310136
                    || selected.triple_offset != 310043
                    || selected.image_index != 0
                    || selected.program_index != 1)
            {
                return Err(Error::Invalid(
                    "PL27A qualified selection binding differs".into(),
                ));
            }
            slots.push(slot);
        }
    }
    Ok(slots)
}

const AREA024: &str = "BIN/WORLD00/AREA024.EMI";
const METADATA_SHA256: &str = "1b36c8e96987f4c2256daa9e24e477d90c73616242be2588906525a4c8dc2c99";
const TEXTURE_SHA256: &str = "1593f9f633500b23d18d924ec68b1bce902222657721ae9f3d549c471c2ab1c5";
const PALETTE_SHA256: &str = "a2b7fbb4ec966bac3a0031ff61390960822075c425457d88a3a497bfcefec4ab";
const CELL_SHA256: &str = "60acf2e269b8b4145a599fc00e382e4d091310114d0c4d05d3c7e6461e1f7978";

fn resource(resources: &[Resource], entry: usize, hash: &str) -> Option<usize> {
    resources.iter().position(|resource| {
        resource.source == AREA024 && resource.entry == Some(entry) && resource.sha256 == hash
    })
}

/// Retain qualified observations without resolving every use of their records.
pub(crate) fn annotate(resources: &mut [Resource], records: &mut [Record]) -> Result<()> {
    annotate_resident(resources, records)?;
    annotate_surface(resources, records)
}

fn annotate_resident(resources: &mut [Resource], records: &mut [Record]) -> Result<()> {
    let Some(metadata) = resource(resources, 4, METADATA_SHA256) else {
        return Ok(());
    };
    let texture = resource(resources, 5, TEXTURE_SHA256).ok_or_else(|| {
        Error::Invalid("AREA024 cell association lacks its qualified texture".into())
    })?;
    let palette = resource(resources, 6, PALETTE_SHA256).ok_or_else(|| {
        Error::Invalid("AREA024 cell association lacks its qualified palette".into())
    })?;
    let cell = records
        .iter_mut()
        .find(|record| {
            record.parent == resources[metadata].id
                && record.coordinate_space == CoordinateSpace::EntryPayload
                && record.offset == 0x0c51
                && record.size == 11
                && record.kind == "sprite-cell"
                && record.coordinate_source_sha256 == METADATA_SHA256
                && record.sha256 == CELL_SHA256
        })
        .ok_or_else(|| {
            Error::Invalid("AREA024 observed cell differs from its qualified record".into())
        })?;

    cell.dependencies.extend([
        Dependency {
            kind: "texture-source".into(),
            target: Some(resources[texture].id.clone()),
            status: "conditional".into(),
            reason: "Observed cell 0x0c51 uses 8-bit page 0x0089 after a zero texture-window command. Its inclusive UV envelope U=128..176, V=16..32 is backed by original entry 5 chunks 6 and 22 at both capture boundaries. Actual sampled texels, transient residency and other draw contexts remain unresolved.".into(),
        },
        Dependency {
            kind: "palette".into(),
            target: Some(resources[palette].id.clone()),
            status: "conditional".into(),
            reason: "Observed cell 0x0c51 selects CLUT 0x7c02, VRAM (32, 496). The qualified UV envelope contains only indices 0..31, selecting entry 6 words 32..63. Index 17 selects source word 49. This does not resolve other CLUTs or all uses of this cell.".into(),
        },
    ]);
    cell.dependencies
        .sort_by(|a, b| (&a.kind, &a.target, &a.status).cmp(&(&b.kind, &b.target, &b.status)));
    cell.evidence.extend([
        "SLUS_004.22@0x8014C6F8:AREA024-cell-0x0c51-source-binding".into(),
        "SLUS_004.22@0x8014C704:observed-clut-0x7c02".into(),
        "SLUS_004.22@0x8014CD90:two-source-bound-quad-payloads-observed-at-GPU-ingress".into(),
    ]);
    cell.evidence.sort();
    cell.evidence.dedup();
    for index in [texture, palette] {
        resources[index].uses.push(cell.id.clone());
        resources[index].uses.sort();
        resources[index].uses.dedup();
    }
    Ok(())
}

struct SurfaceObservation {
    offset: usize,
    sha256: &'static str,
    texture_reason: &'static str,
    palette_reason: &'static str,
    evidence: &'static [&'static str],
}

fn annotate_surface(resources: &mut [Resource], records: &mut [Record]) -> Result<()> {
    let metadata_hash = "a50ad6234f7e548bb5ab3779d8c9252965e34f7e2a7fa267ec3561cc9f7f11dc";
    let Some(metadata) = resource(resources, 13, metadata_hash) else {
        return Ok(());
    };
    let texture = resource(
        resources,
        7,
        "a5f1cc1215c479fb702a2a65bf5430ce26c39cf0efaabd8312f5fc68fbef7092",
    )
    .ok_or_else(|| {
        Error::Invalid("AREA024 surface association lacks its qualified texture".into())
    })?;
    let palette = resource(
        resources,
        8,
        "e9fcb1395884aff7dc732517fd685e9fb26fe87ccf78f757055e410cda8e9e18",
    )
    .ok_or_else(|| {
        Error::Invalid("AREA024 surface association lacks its qualified palette".into())
    })?;
    let observations = [
        SurfaceObservation {
            offset: 58_300,
            sha256: "029d35fb54e74199d5abd87e1aa7e3d4a62cc37ad1f672e6198c216d0df7a0ff",
            texture_reason: "Original grid record 563 reaches the coordinate consumer with word 0x5380007a. Its immediate post-call fields contain page 0x0097 and UVs (160,112), (175,112), (160,127), (175,127). Recorded CPU writes connect the constructed fields to the main-loop DrawOTag entry. The subsequent native GPU payload matches the submitted bytes in the same execution, after a zero texture-window command. Its vertices lie above the captured draw area. All declared entry 7 upload bytes match serialized VRAM; the nominal UV region lies in chunk 54. RAM writes after the DrawOTag entry, actual sampling and upload history remain unresolved.",
            palette_reason: "Original grid record 563 produces CLUT 0x7980. Its recorded CPU store persists through the main-loop DrawOTag entry, and the subsequent native GPU payload carries that CLUT in the same execution. Serialized VRAM row 486 at word X=0 matches entry 8 words 768..1023, and the entire entry matches resident RAM. RAM writes after the DrawOTag entry, actual palette reads and source uniqueness remain unresolved. Other palette rows retain observed differences; their mutation causes and inverse edits remain unresolved.",
            evidence: &[
                "SLUS_004.22@0x80154358:AREA024-grid-563-original-metadata-call",
                "SLUS_004.22@0x80154360:AREA024-grid-563-primitive-and-resident-bytes",
                "SLUS_004.22@0x8015584C:AREA024-grid-563-committed-writes-before-GPU-submission",
                "SLUS_004.22@0x8017B9CC:AREA024-same-buffer-quad-at-GPU-ingress-after-zero-texture-window",
            ],
        },
        SurfaceObservation {
            offset: 58_940,
            sha256: "908632fe4ac7aaa714985ee62156200a340d2292b5859fc48367636c1bb03044",
            texture_reason: "Original grid record 723 reaches the coordinate consumer with word 0x13800095. Recorded CPU writes connect its constructed fields to the main-loop DrawOTag entry. The subsequent native GPU payload matches the submitted bytes in the same execution, with page 0x0097 and UVs (80,144), (95,144), (80,159), (95,159). Its encoded vertices lie inside the known zero-offset draw area with a zero texture window. All declared entry 7 upload bytes match final serialized VRAM; the nominal UV region lies in chunk 69. RAM writes after the DrawOTag entry remain unobserved. Final residency alone does not prove earlier sampling or upload history. Independent original and edited media boots with the same opening input schedule show a magenta surface patch after replacing the 16x16 indices with 179 and entry 8 word 947 with 0x7c1f. The metadata entry and known packet match in both final RAM captures; later GPU consumption and all shared uses remain unresolved.",
            palette_reason: "Original grid record 723 produces CLUT 0x7980. Its recorded CPU store persists through the main-loop DrawOTag entry, and the subsequent native GPU payload carries that CLUT in the same execution. Final serialized VRAM row 486 at word X=0 matches entry 8 words 768..1023. The entire entry matches constructor, submission and final RAM. RAM writes after the DrawOTag entry, actual palette reads and source uniqueness remain unresolved. Other palette rows retain observed differences; their mutation causes and inverse edits remain unresolved. The combined texel and palette edit is visible on one surface in the opening mine. This does not isolate the palette change's contribution or prove every use of that source word.",
            evidence: &[
                "SLUS_004.22@0x80154358:AREA024-grid-723-original-metadata-call",
                "SLUS_004.22@0x8015584C:AREA024-grid-723-committed-writes-before-GPU-submission",
                "SLUS_004.22@0x8017B9CC:AREA024-grid-723-native-quad-with-interior-coordinates",
                "BIN/WORLD00/AREA024.EMI#7:combined-index-179-and-entry-8-word-947-visible-mine-edit",
            ],
        },
    ];
    for observation in observations {
        let word = records
            .iter_mut()
            .find(|record| {
                record.parent == resources[metadata].id
                    && record.coordinate_space == CoordinateSpace::EntryPayload
                    && record.offset == observation.offset
                    && record.size == 4
                    && record.kind == "surface-word"
                    && record.coordinate_source_sha256 == metadata_hash
                    && record.sha256 == observation.sha256
            })
            .ok_or_else(|| {
                Error::Invalid("AREA024 observed surface differs from its qualified record".into())
            })?;
        word.dependencies.extend([
            Dependency {
                kind: "texture-source".into(),
                target: Some(resources[texture].id.clone()),
                status: "conditional".into(),
                reason: observation.texture_reason.into(),
            },
            Dependency {
                kind: "palette".into(),
                target: Some(resources[palette].id.clone()),
                status: "conditional".into(),
                reason: observation.palette_reason.into(),
            },
        ]);
        word.dependencies
            .sort_by(|a, b| (&a.kind, &a.target, &a.status).cmp(&(&b.kind, &b.target, &b.status)));
        word.evidence
            .extend(observation.evidence.iter().map(|value| (*value).into()));
        word.evidence.sort();
        word.evidence.dedup();
        for index in [texture, palette] {
            resources[index].uses.push(word.id.clone());
            resources[index].uses.sort();
            resources[index].uses.dedup();
        }
    }
    Ok(())
}
