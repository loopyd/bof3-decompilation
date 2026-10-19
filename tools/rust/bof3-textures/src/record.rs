//! Source-bound records inside validated sprite, resident and save-icon layouts.

use std::collections::BTreeMap;

use serde::{Deserialize, Serialize};

use crate::{
    Error, Result, animation,
    catalog::{Dependency, QueryFields, Queryable, Resource},
    cell,
    digest::sha256_hex,
    resident, save_icon, sprite_bank,
};

#[derive(Clone, Copy, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(rename_all = "kebab-case")]
pub enum CoordinateSpace {
    DecodedBank,
    EntryPayload,
}

impl CoordinateSpace {
    fn label(self) -> &'static str {
        match self {
            Self::DecodedBank => "decoded-bank",
            Self::EntryPayload => "entry-payload",
        }
    }
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Record {
    pub id: String,
    pub parent: String,
    pub source: String,
    pub entry: Option<usize>,
    pub coordinate_space: CoordinateSpace,
    pub coordinate_source_sha256: String,
    pub offset: usize,
    pub size: usize,
    pub sha256: String,
    pub kind: String,
    pub categories: Vec<String>,
    pub storage: String,
    pub decode: String,
    pub pack: String,
    pub evidence_status: String,
    pub evidence: Vec<String>,
    pub dependencies: Vec<Dependency>,
    pub uses: Vec<String>,
}

impl Queryable for Record {
    fn query_fields(&self) -> QueryFields<'_> {
        QueryFields {
            id: &self.id,
            source: &self.source,
            categories: &self.categories,
            storage: &self.storage,
            kind: &self.kind,
            evidence_status: &self.evidence_status,
            dependencies: &self.dependencies,
            is_palette: self.kind == "palette-record",
            is_animation: self.kind == "animation-program",
        }
    }
}

struct Builder<'a> {
    parent: &'a Resource,
    source: &'a [u8],
    space: CoordinateSpace,
    source_sha256: String,
    records: BTreeMap<String, Record>,
}

impl<'a> Builder<'a> {
    fn id(&self, kind: &str, offset: usize) -> String {
        format!(
            "{}/{}/{kind}@{offset:08x}",
            self.parent.id,
            self.space.label()
        )
    }

    fn add(&mut self, kind: &str, offset: usize, size: usize, usage: String) -> Result<String> {
        let end = offset
            .checked_add(size)
            .ok_or_else(|| Error::Invalid("bank record extent overflows its coordinates".into()))?;
        let bytes = self.source.get(offset..end).ok_or_else(|| {
            Error::Invalid("bank record extent exceeds its coordinate source".into())
        })?;
        let id = self.id(kind, offset);
        if let Some(record) = self.records.get_mut(&id) {
            if record.size != size {
                return Err(Error::Invalid(
                    "aliased bank record has conflicting extents".into(),
                ));
            }
            record.uses.push(usage);
            return Ok(id);
        }
        let consumer = match kind {
            "bank-image" => "SLUS_004.22@0x8014E6D0:image-records",
            "palette-record" if self.parent.save_icon.is_some() => {
                "BIN/ETC/SHOP.EMI#8@0x801EFE68:save-header-palette-source"
            }
            "palette-record" => "SLUS_004.22@0x80166C48:32-word-palette-copy",
            "save-icon-frame" => "BIN/ETC/SHOP.EMI#8@0x801EF27C:save-header-frame-source",
            "animation-program" => "SLUS_004.22@0x8014D910:program-initialization",
            "sprite-cell" => "SLUS_004.22@0x8014C6F8:five-byte-cell-parts",
            _ => unreachable!(),
        };
        self.records.insert(id.clone(), Record {
            id: id.clone(), parent: self.parent.id.clone(), source: self.parent.source.clone(),
            entry: self.parent.entry, coordinate_space: self.space,
            coordinate_source_sha256: self.source_sha256.clone(), offset, size,
            sha256: sha256_hex(bytes), kind: kind.into(), categories: self.parent.categories.clone(),
            storage: format!("{}-record", self.parent.storage), decode: "record-layout-verified".into(),
            pack: "parent-package-controls-edits".into(), evidence_status: "source-qualified".into(),
            evidence: vec![consumer.into()], uses: vec![usage],
            dependencies: vec![
                Dependency { kind: "source".into(), target: Some(self.parent.id.clone()),
                    status: "verified".into(), reason: "record bytes are bound to this physical resource and coordinate source".into() },
                Dependency { kind: "palette".into(), target: None,
                    status: "unresolved".into(), reason: "draw CLUT associations and full neighboring colors are not established by record structure".into() },
            ],
        });
        Ok(id)
    }

    fn new(parent: &'a Resource, source: &'a [u8], space: CoordinateSpace) -> Self {
        Self {
            parent,
            source,
            space,
            source_sha256: sha256_hex(source),
            records: BTreeMap::new(),
        }
    }

    fn cells(&mut self, cells: &[cell::Cell], usage: &str) -> Result<()> {
        for cell in cells {
            self.add(
                "sprite-cell",
                cell.offset,
                1 + 5 * cell.parts.len(),
                format!("{usage}/cell@{:08x}", cell.offset),
            )?;
        }
        Ok(())
    }

    fn program(&mut self, offset: usize, end: usize, usage: String) -> Result<()> {
        let decoded = animation::decode(self.source, offset, end)?;
        let cell_table = offset + 2 + decoded.steps().len() * 2;
        let program_id = self.add(
            "animation-program",
            offset,
            cell_table - offset + decoded.cell_offsets().len() * 2,
            usage,
        )?;
        for (slot, relative) in decoded.cell_offsets().iter().enumerate() {
            let offset = cell_table + usize::from(*relative);
            let cell_id = self.id("sprite-cell", offset);
            if !self.records.contains_key(&cell_id) {
                return Err(Error::Invalid(
                    "program refers to a cell missing from its validated source".into(),
                ));
            }
            self.edge(
                &program_id,
                "cell",
                cell_id.clone(),
                format!("cell slot {slot} is a u16 offset relative to the program cell table"),
            );
            self.edge(
                &cell_id,
                "animation",
                program_id.clone(),
                "this program cell table references the record; execution is not inferred".into(),
            );
        }
        for alias in decoded.metadata_cells() {
            let cell_id = self.id("sprite-cell", alias.cell_offset);
            let cell = self.records.get_mut(&cell_id).ok_or_else(|| {
                Error::Invalid("metadata alias selects a missing source-bound cell".into())
            })?;
            cell.evidence.push(format!(
                "SLUS_004.22@0x8014DBFC:u16 lookup at byte {} stores {}; zero part count at byte {} lies in the declared cell-offset table; setup at 0x8014C6FC precedes the zero-part branch",
                alias.lookup_offset, alias.relative_offset, alias.cell_offset
            ));
            self.edge(
                &program_id,
                "metadata-cell",
                cell_id.clone(),
                format!("normal operand {} resolves a zero-part metadata cell without adding a declared offset", alias.operand),
            );
            self.edge(
                &cell_id,
                "animation-metadata",
                program_id.clone(),
                "the zero count overlaps this program's immutable declared offset table; execution and rendering are unresolved".into(),
            );
            self.edge(
                &cell_id,
                "metadata-lookup",
                self.parent.id.clone(),
                format!("immutable {} lookup bytes {}..{} store relative offset {}; all alias-bearing program steps are read-only", self.space.label(), alias.lookup_offset, alias.lookup_offset + 2, alias.relative_offset),
            );
        }
        Ok(())
    }

    fn finish(self) -> Vec<Record> {
        let mut records: Vec<_> = self.records.into_values().collect();
        for record in &mut records {
            record.uses.sort();
            record.uses.dedup();
            record.dependencies.sort_by(|a, b| {
                (&a.kind, &a.target, &a.status, &a.reason)
                    .cmp(&(&b.kind, &b.target, &b.status, &b.reason))
            });
            record.dependencies.dedup();
        }
        records
    }

    fn edge(&mut self, from: &str, kind: &str, to: String, reason: String) {
        self.records
            .get_mut(from)
            .unwrap()
            .dependencies
            .push(Dependency {
                kind: kind.into(),
                target: Some(to),
                status: "verified".into(),
                reason,
            });
    }
}

/// The caller supplies a document decoded from the same bounded coordinate source.
pub(crate) fn sprite_records(
    parent: &Resource,
    source: &[u8],
    space: CoordinateSpace,
    document: &sprite_bank::Document,
) -> Result<Vec<Record>> {
    let mut builder = Builder::new(parent, source, space);
    for (root_index, root) in document.roots.iter().enumerate() {
        for (slot, image) in root.images.iter().enumerate() {
            builder.add(
                "bank-image",
                image.offset,
                image.consumed_bytes,
                format!("root-{root_index}/image-slot-{slot}"),
            )?;
        }
        for (slot, palette) in root.palettes.iter().enumerate() {
            builder.add(
                "palette-record",
                palette.offset,
                64,
                format!("root-{root_index}/palette-slot-{slot}"),
            )?;
        }
        builder.cells(&root.cells, &format!("root-{root_index}"))?;
        for (slot, program) in root.programs.iter().enumerate() {
            builder.program(
                program.offset,
                program.end,
                format!("root-{root_index}/program-slot-{slot}"),
            )?;
        }
    }
    Ok(builder.finish())
}

pub(crate) fn resident_records(
    parent: &Resource,
    source: &[u8],
    document: &resident::Document,
) -> Result<Vec<Record>> {
    let mut builder = Builder::new(parent, source, CoordinateSpace::EntryPayload);
    builder.cells(&document.cells, "resident")?;
    for (group_slot, group) in document.groups.iter().enumerate() {
        for (program_slot, program) in group.programs.iter().enumerate() {
            builder.program(
                program.offset,
                program.end,
                format!("group-{group_slot}/program-slot-{program_slot}"),
            )?;
        }
    }
    for record in builder.records.values_mut() {
        record.dependencies.push(Dependency { kind: "resident-base".into(), target: None, status: "unresolved".into(),
            reason: "the resident program/cell source does not bind its image producer, active uploads or texture page".into() });
    }
    Ok(builder.finish())
}

pub(crate) fn save_icon_records(
    parent: &Resource,
    source: &[u8],
    info: &save_icon::Info,
) -> Result<Vec<Record>> {
    let mut builder = Builder::new(parent, source, CoordinateSpace::EntryPayload);
    let palette = builder.add(
        "palette-record",
        info.palette_offset,
        32,
        "save-header/palette".into(),
    )?;
    for (slot, offset) in info.frame_offsets.iter().copied().enumerate() {
        let frame = builder.add(
            "save-icon-frame",
            offset,
            128,
            format!("save-header/frame-{slot}"),
        )?;
        builder.edge(&frame, "palette", palette.clone(), "the original producer copies this index plane and the shared 16-word palette into the same SC header".into());
        builder.edge(
            &palette,
            "frame",
            frame.clone(),
            format!("original save-header frame {slot} uses this palette"),
        );
        builder
            .records
            .get_mut(&palette)
            .unwrap()
            .uses
            .push(format!("save-header/frame-{slot}"));
    }
    for record in builder.records.values_mut() {
        record
            .dependencies
            .retain(|edge| !(edge.kind == "palette" && edge.target.is_none()));
        record.dependencies.push(Dependency { kind: "save-header-consumer".into(), target: None, status: "unresolved".into(),
            reason: "source copies do not establish BIOS frame timing, original loading or edited display".into() });
    }
    Ok(builder.finish())
}
