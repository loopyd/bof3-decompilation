//! Source associations for qualified field consumer observations.

use crate::{
    Error, Result,
    catalog::{Dependency, Resource},
    record::{CoordinateSpace, Record},
};

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
    let word = records
        .iter_mut()
        .find(|record| {
            record.parent == resources[metadata].id
                && record.coordinate_space == CoordinateSpace::EntryPayload
                && record.offset == 58_300
                && record.size == 4
                && record.kind == "surface-word"
                && record.coordinate_source_sha256 == metadata_hash
                && record.sha256
                    == "029d35fb54e74199d5abd87e1aa7e3d4a62cc37ad1f672e6198c216d0df7a0ff"
        })
        .ok_or_else(|| {
            Error::Invalid("AREA024 observed surface differs from its qualified record".into())
        })?;
    word.dependencies.extend([
        Dependency {
            kind: "texture-source".into(),
            target: Some(resources[texture].id.clone()),
            status: "conditional".into(),
            reason: "Original grid record 563 reaches the coordinate consumer with word 0x5380007a. Its immediate post-call fields contain page 0x0097 and UVs (160,112), (175,112), (160,127), (175,127). A later packet from the same physical buffer reaches GPU ingress with matching page and UV fields after a zero texture-window command. Its vertices lie above the captured draw area. All declared entry 7 upload bytes match serialized VRAM; the nominal UV region lies in chunk 54. Buffer ownership between construction and submission, intervening writes, actual sampling and upload history remain unresolved.".into(),
        },
        Dependency {
            kind: "palette".into(),
            target: Some(resources[palette].id.clone()),
            status: "conditional".into(),
            reason: "Original grid record 563 produces CLUT 0x7980. A later packet from the same physical buffer carries that CLUT at GPU ingress. Serialized VRAM row 486 at word X=0 matches entry 8 words 768..1023, and the entire entry matches resident RAM. Buffer ownership across that interval, actual palette reads and source uniqueness remain unresolved. Other palette rows retain observed differences; their mutation causes and inverse edits remain unresolved.".into(),
        },
    ]);
    word.dependencies
        .sort_by(|a, b| (&a.kind, &a.target, &a.status).cmp(&(&b.kind, &b.target, &b.status)));
    word.evidence.extend([
        "SLUS_004.22@0x80154358:AREA024-grid-563-original-metadata-call".into(),
        "SLUS_004.22@0x80154360:AREA024-grid-563-primitive-and-resident-bytes".into(),
        "SLUS_004.22@0x8017B9CC:AREA024-same-buffer-quad-at-GPU-ingress-after-zero-texture-window"
            .into(),
    ]);
    word.evidence.sort();
    word.evidence.dedup();
    for index in [texture, palette] {
        resources[index].uses.push(word.id.clone());
        resources[index].uses.sort();
        resources[index].uses.dedup();
    }
    Ok(())
}
