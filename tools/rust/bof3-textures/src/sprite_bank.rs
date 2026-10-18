//! Three-root bank variant selected by GAME.EMI#0@0x801C1254.
//! Selection tails remain opaque until their valid runtime indices are known.
use crate::cell;
use crate::{Error, Result, animation, bank_image};
use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, BTreeSet};

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Palette {
    pub offset: usize,
    pub words: [u16; 32],
}
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Program {
    pub offset: usize,
    pub end: usize,
    pub steps: Vec<animation::Step>,
}
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Root {
    pub offset: usize,
    pub end: usize,
    pub image_table: usize,
    pub palette_table: usize,
    pub program_table: usize,
    pub selection_offset: usize,
    pub images: Vec<bank_image::Info>,
    pub palettes: Vec<Palette>,
    pub programs: Vec<Program>,
    pub cells: Vec<cell::Cell>,
}
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Document {
    pub schema: String,
    pub roots: Vec<Root>,
}
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub roots: usize,
    pub images: usize,
    pub image_modes: Vec<Vec<u32>>,
    pub palettes: usize,
    pub programs: usize,
    pub cells: usize,
    pub parts: usize,
}

impl Document {
    pub fn info(&self) -> Info {
        Info {
            roots: self.roots.len(),
            images: self.roots.iter().map(|root| root.images.len()).sum(),
            image_modes: self
                .roots
                .iter()
                .map(|root| root.images.iter().map(|image| image.mode).collect())
                .collect(),
            palettes: self.roots.iter().map(|root| root.palettes.len()).sum(),
            programs: self.roots.iter().map(|root| root.programs.len()).sum(),
            cells: self.roots.iter().map(|root| root.cells.len()).sum(),
            parts: self
                .roots
                .iter()
                .flat_map(|root| &root.cells)
                .map(|cell| cell.parts.len())
                .sum(),
        }
    }
}

pub const SCHEMA: &str = "bof3.sprite-bank/v1";
fn invalid(message: &str) -> Error {
    Error::Codec(format!("sprite bank: {message}"))
}
fn u32at(source: &[u8], offset: usize) -> Result<usize> {
    let bytes = source
        .get(
            offset
                ..offset
                    .checked_add(4)
                    .ok_or_else(|| invalid("offset overflow"))?,
        )
        .ok_or_else(|| invalid("truncated word"))?;
    Ok(u32::from_le_bytes(bytes.try_into().unwrap()) as usize)
}
fn relative(base: usize, value: usize) -> Result<usize> {
    base.checked_add(value)
        .ok_or_else(|| invalid("relative offset overflow"))
}
fn extent(offset: usize, starts: &BTreeSet<usize>, end: usize) -> usize {
    starts.range((offset + 1)..).next().copied().unwrap_or(end)
}
fn offsets(source: &[u8], table: usize, end: usize, word_bytes: usize) -> Result<Vec<usize>> {
    let first = if word_bytes == 4 {
        u32at(source, table)?
    } else {
        let header = source
            .get(table..table + 2)
            .ok_or_else(|| invalid("truncated program table"))?;
        usize::from(u16::from_le_bytes(header.try_into().unwrap()))
    };
    if first == 0 || !first.is_multiple_of(word_bytes) || first / word_bytes > 256 {
        return Err(invalid("table exceeds its byte selector"));
    }
    let data_start = relative(table, first)?;
    if data_start > end {
        return Err(invalid("table exceeds its section"));
    }
    let bytes = source
        .get(table..data_start)
        .ok_or_else(|| invalid("truncated offset table"))?;
    bytes
        .chunks_exact(word_bytes)
        .map(|bytes| {
            let value = if word_bytes == 4 {
                u32::from_le_bytes(bytes.try_into().unwrap()) as usize
            } else {
                usize::from(u16::from_le_bytes(bytes.try_into().unwrap()))
            };
            let offset = relative(table, value)?;
            if offset < data_start || offset >= end {
                return Err(invalid("record exceeds its section"));
            }
            Ok(offset)
        })
        .collect()
}
/// Decode only the original three-root, ordered-section variant, within 1 MiB.
/// Image ends are bounds, not reusable allocation. Edits use consumed bytes only.
/// Palette records contain 32 exact PSX words. No draw-CLUT binding is implied.
/// Cells use the existing original part layout. Selection bytes remain opaque.
pub fn decode(source: &[u8]) -> Result<Document> {
    if source.len() > 1024 * 1024 || u32at(source, 0)? != 12 {
        return Err(invalid("unsupported root table"));
    }
    let roots = (0..3)
        .map(|i| u32at(source, i * 4))
        .collect::<Result<Vec<_>>>()?;
    if roots
        .iter()
        .any(|offset| *offset < 12 || *offset >= source.len())
    {
        return Err(invalid("root exceeds its bank"));
    }
    let starts = roots.iter().copied().collect::<BTreeSet<_>>();
    let mut decoded = Vec::new();
    for root in roots {
        let end = extent(root, &starts, source.len());
        let header = source
            .get(root..end)
            .and_then(|bytes| bytes.get(..16))
            .ok_or_else(|| invalid("truncated root header"))?;
        let sections = header
            .as_chunks::<4>()
            .0
            .iter()
            .map(|bytes| relative(root, u32::from_le_bytes(*bytes) as usize))
            .collect::<Result<Vec<_>>>()?;
        let [image_table, palette_table, program_table, selection_offset] =
            sections.try_into().unwrap();
        if image_table != root + 16
            || image_table >= palette_table
            || palette_table >= program_table
            || program_table >= selection_offset
            || selection_offset >= end
        {
            return Err(invalid("unsupported section order"));
        }
        let image_offsets = offsets(source, image_table, palette_table, 4)?;
        let image_starts = image_offsets.iter().copied().collect();
        let mut images = Vec::new();
        for offset in image_offsets {
            let bound = extent(offset, &image_starts, palette_table);
            images.push(bank_image::decode(source, offset, bound)?.info);
        }
        let palette_offsets = offsets(source, palette_table, program_table, 4)?;
        let palette_starts = palette_offsets.iter().copied().collect();
        let mut palettes = Vec::new();
        for offset in palette_offsets {
            let bound = extent(offset, &palette_starts, program_table);
            let bytes = source
                .get(offset..bound)
                .and_then(|bytes| bytes.get(..64))
                .ok_or_else(|| invalid("truncated 32-word palette"))?;
            let mut words = [0; 32];
            for (word, bytes) in words.iter_mut().zip(bytes.as_chunks::<2>().0.iter()) {
                *word = u16::from_le_bytes(*bytes);
            }
            palettes.push(Palette { offset, words });
        }
        let program_offsets = offsets(source, program_table, selection_offset, 2)?;
        let program_starts = program_offsets.iter().copied().collect();
        let mut programs = Vec::new();
        let mut cells = BTreeMap::new();
        for offset in program_offsets {
            let end = extent(offset, &program_starts, selection_offset);
            let program = animation::decode(source, offset, end)?;
            program.start(0)?;
            for token in program.steps().iter().filter(|token| token.duration >= 128) {
                program.start(usize::from(token.operand))?;
            }
            cell::decode_cells(source, offset, end, &program, &mut cells)?;
            programs.push(Program {
                offset,
                end,
                steps: program.steps().to_vec(),
            });
        }
        decoded.push(Root {
            offset: root,
            end,
            image_table,
            palette_table,
            program_table,
            selection_offset,
            images,
            palettes,
            programs,
            cells: cells.into_values().collect(),
        });
    }
    Ok(Document {
        schema: SCHEMA.into(),
        roots: decoded,
    })
}

fn root_at(document: &Document, index: usize) -> Result<&Root> {
    document
        .roots
        .get(index)
        .ok_or_else(|| invalid("missing root index"))
}
fn agree<'a, T: PartialEq + ?Sized>(
    edits: &mut BTreeMap<usize, (&'a T, String)>,
    offset: usize,
    value: &'a T,
    view: String,
) -> Result<()> {
    if let Some((previous, owner)) = edits.get(&offset)
        && *previous != value
    {
        return Err(Error::Conflict(format!(
            "sprite aliases {owner} and {view} disagree at decoded byte {offset}"
        )));
    }
    edits.insert(offset, (value, view));
    Ok(())
}
/// Change palette words, animation pairs and cell parts in their existing records.
/// Preserve all header/table pointers, selection bytes and unknown tails. Changed
/// aliases must agree on the complete record; unchanged aliases do not override.
pub fn encode(source: &[u8], edited: &Document) -> Result<Vec<u8>> {
    let original = decode(source)?;
    if edited.schema != SCHEMA || edited.roots.len() != original.roots.len() {
        return Err(invalid("edit changes root allocation"));
    }
    let mut palettes = BTreeMap::new();
    let mut programs = BTreeMap::new();
    let mut cells = BTreeMap::new();
    for (root_index, (root, raw)) in edited.roots.iter().zip(&original.roots).enumerate() {
        if (
            root.offset,
            root.end,
            root.image_table,
            root.palette_table,
            root.program_table,
            root.selection_offset,
        ) != (
            raw.offset,
            raw.end,
            raw.image_table,
            raw.palette_table,
            raw.program_table,
            raw.selection_offset,
        ) || root.images != raw.images
            || root.palettes.len() != raw.palettes.len()
            || root.programs.len() != raw.programs.len()
            || root.cells.len() != raw.cells.len()
        {
            return Err(invalid("edit changes a root, table or image descriptor"));
        }
        for (index, (palette, raw)) in root.palettes.iter().zip(&raw.palettes).enumerate() {
            if palette.offset != raw.offset {
                return Err(invalid("edit changes palette offset"));
            }
            if palette != raw {
                agree(
                    &mut palettes,
                    palette.offset,
                    palette,
                    format!("root-{root_index}/palette-{index}"),
                )?;
            }
        }
        for (index, (program, raw)) in root.programs.iter().zip(&raw.programs).enumerate() {
            if program.offset != raw.offset
                || program.end != raw.end
                || program.steps.len() != raw.steps.len()
            {
                return Err(invalid("edit changes program allocation"));
            }
            if program != raw {
                agree(
                    &mut programs,
                    program.offset,
                    program,
                    format!("root-{root_index}/program-{index}"),
                )?;
            }
        }
        for (index, (cell, raw)) in root.cells.iter().zip(&raw.cells).enumerate() {
            if cell.offset != raw.offset
                || cell.end != raw.end
                || cell.parts.len() != raw.parts.len()
            {
                return Err(invalid("edit changes cell allocation"));
            }
            if cell != raw {
                agree(
                    &mut cells,
                    cell.offset,
                    cell,
                    format!("root-{root_index}/cell-{index}"),
                )?;
            }
        }
    }
    let mut output = source.to_vec();
    for (palette, _) in palettes.values() {
        for (index, word) in palette.words.iter().enumerate() {
            let start = palette.offset + 2 * index;
            output[start..start + 2].copy_from_slice(&word.to_le_bytes());
        }
    }
    for (program, _) in programs.values() {
        let encoded = animation::encode_steps(source, program.offset, program.end, &program.steps)?;
        let start = program.offset + 2;
        let end = start + program.steps.len() * 2;
        output[start..end].copy_from_slice(&encoded[start..end]);
    }
    let raw_cells = original
        .roots
        .iter()
        .flat_map(|root| &root.cells)
        .map(|cell| (cell.offset, cell))
        .collect::<BTreeMap<_, _>>();
    for (&offset, (cell, _)) in &cells {
        cell::encode_cells(
            &mut output,
            std::slice::from_ref(raw_cells[&offset]),
            std::slice::from_ref(*cell),
        )?;
    }
    decode(&output)?;
    Ok(output)
}

/// One explicit index-plane view of an existing image record.
pub struct ImageEdit<'a> {
    pub root: usize,
    pub image: usize,
    pub bytes: &'a [u8],
}
/// Combine document and image edits in decoded-bank coordinates. Image streams
/// may use only their original consumed bytes; gaps and selection tails remain
/// opaque. Repeated changed views must agree on their complete index planes.
pub fn encode_images(source: &[u8], edited: &Document, edits: &[ImageEdit<'_>]) -> Result<Vec<u8>> {
    let original = decode(source)?;
    if edits.len()
        > original
            .roots
            .iter()
            .map(|root| root.images.len())
            .sum::<usize>()
    {
        return Err(invalid("more image edits than qualified image indices"));
    }
    let mut images = BTreeMap::new();
    for edit in edits {
        let image = root_at(&original, edit.root)?
            .images
            .get(edit.image)
            .ok_or_else(|| invalid("missing image index"))?;
        let raw = bank_image::decode(source, image.offset, image.end)?;
        if edit.bytes.len() != raw.bytes.len() {
            return Err(invalid("image edit changes dimensions"));
        }
        if edit.bytes != raw.bytes {
            agree(
                &mut images,
                image.offset,
                edit.bytes,
                format!("root-{}/image-{}", edit.root, edit.image),
            )?;
        }
    }
    let mut output = encode(source, edited)?;
    for (offset, (bytes, _)) in images {
        let image = original
            .roots
            .iter()
            .flat_map(|root| &root.images)
            .find(|image| image.offset == offset)
            .unwrap();
        let end = offset + image.consumed_bytes;
        let encoded = bank_image::encode(bytes, source, offset, end)?;
        output[offset..end].copy_from_slice(&encoded[offset..end]);
    }
    decode(&output)?;
    Ok(output)
}
