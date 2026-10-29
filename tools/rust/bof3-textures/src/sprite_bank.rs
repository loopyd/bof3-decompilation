//! Three-root banks and standalone roots with the same relative sections.
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

/// One caller-selected physical slot. This does not establish the valid selector
/// domain or make the surrounding selection tail editable.
#[derive(Clone, Debug, Serialize, PartialEq, Eq)]
pub struct Selection {
    pub triple_offset: usize,
    pub image_index: u8,
    /// None follows the original image-skip branch; image_index is then opaque.
    pub image: Option<bank_image::Info>,
    pub program_index: u8,
    pub program: Program,
    /// Copied unchanged to entity byte 0x2A, not a palette selector.
    pub mirror_flag: u8,
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
/// Decode original three-root banks or standalone ordered-section roots, within 1 MiB.
/// Image ends are bounds, not reusable allocation. Edits use consumed bytes only.
/// Palette records contain 32 exact PSX words. No draw-CLUT binding is implied.
/// Cells use the existing original part layout. Selection bytes remain opaque.
pub fn decode(source: &[u8]) -> Result<Document> {
    if source.len() > 1024 * 1024 {
        return Err(invalid("bank exceeds 1 MiB"));
    }
    let (roots, table_end) = match u32at(source, 0)? {
        12 => (
            (0..3)
                .map(|i| u32at(source, i * 4))
                .collect::<Result<Vec<_>>>()?,
            12,
        ),
        16 => (vec![0], 0),
        _ => return Err(invalid("unsupported root layout")),
    };
    if roots
        .iter()
        .any(|offset| *offset < table_end || *offset >= source.len())
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
/// Resolve one explicit selector through SLUS_004.22@0x8014DC08.
/// selector is the final entity byte at +0x4B, not the entry request to
/// SLUS_004.22@0x8014D6B8. The caller must qualify it, the root and image-skip
/// condition from the same consumer. Extent checks do not prove a valid selector.
/// skip_image represents entity byte 0 bit 1, not entity byte 0x24.
pub fn select(
    source: &[u8],
    root_index: usize,
    selector: u8,
    skip_image: bool,
) -> Result<Selection> {
    let document = decode(source)?;
    let root = root_at(&document, root_index)?;
    // decode bounds the whole source to 1 MiB and selector to one byte.
    let triple_offset = root.selection_offset + usize::from(selector) * 3;
    let end = triple_offset + 3;
    let triple = source
        .get(triple_offset..end)
        .filter(|_| end <= root.end)
        .ok_or_else(|| invalid("selected triple exceeds its root extent"))?;
    let image = if skip_image {
        None
    } else {
        Some(
            root.images
                .get(usize::from(triple[0]))
                .ok_or_else(|| invalid("selected image exceeds its table"))?
                .clone(),
        )
    };
    let program = root
        .programs
        .get(usize::from(triple[1]))
        .ok_or_else(|| invalid("selected program exceeds its table"))?
        .clone();
    Ok(Selection {
        triple_offset,
        image_index: triple[0],
        image,
        program_index: triple[1],
        program,
        mirror_flag: triple[2],
    })
}

/// Change palette words, animation pairs and cell parts in their existing records.
/// Preserve all header/table pointers, selection bytes and unknown tails. Changed
/// aliases must agree on the complete record; unchanged aliases do not override.
pub fn encode(source: &[u8], edited: &Document) -> Result<Vec<u8>> {
    let original = decode(source)?;
    if edited.schema != SCHEMA || edited.roots.len() != original.roots.len() {
        return Err(invalid("edit changes root allocation"));
    }
    let mut palettes = crate::aliases::Aliases::new();
    let mut programs = crate::aliases::Aliases::new();
    let mut cells = crate::aliases::Aliases::new();
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
                palettes.add(
                    palette.offset,
                    palette,
                    format!("root-{root_index}/palette-{index}"),
                );
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
                programs.add(
                    program.offset,
                    program,
                    format!("root-{root_index}/program-{index}"),
                );
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
                cells.add(cell.offset, cell, format!("root-{root_index}/cell-{index}"));
            }
        }
    }
    let palettes = palettes.finish("sprite")?;
    let programs = programs.finish("sprite")?;
    let cells = cells.finish("sprite")?;
    let mut output = source.to_vec();
    for palette in palettes.values() {
        for (index, word) in palette.words.iter().enumerate() {
            let start = palette.offset + 2 * index;
            output[start..start + 2].copy_from_slice(&word.to_le_bytes());
        }
    }
    for program in programs.values() {
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
    for (&offset, cell) in &cells {
        cell::encode_cells(
            &mut output,
            std::slice::from_ref(raw_cells[&offset]),
            std::slice::from_ref(*cell),
        )?;
    }
    validate_program_metadata(source, &output, &original)?;
    decode(&output)?;
    Ok(output)
}

fn validate_program_metadata(source: &[u8], output: &[u8], original: &Document) -> Result<()> {
    for program in original.roots.iter().flat_map(|root| &root.programs) {
        animation::decode(source, program.offset, program.end)?
            .validate_preserved(source, output)?;
    }
    Ok(())
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
    let mut images = crate::aliases::Aliases::new();
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
            images.add(
                image.offset,
                edit.bytes,
                format!("root-{}/image-{}", edit.root, edit.image),
            );
        }
    }
    let images = images.finish("sprite")?;
    let mut output = encode(source, edited)?;
    for (offset, bytes) in images {
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
    validate_program_metadata(source, &output, &original)?;
    decode(&output)?;
    Ok(output)
}
