//! Source associations for qualified resident-cell consumer observations.

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

/// Retain one observed use without resolving every use of its source cell.
pub(crate) fn annotate(resources: &mut [Resource], records: &mut [Record]) -> Result<()> {
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
