//! Source derivatives from explicit sprite bindings or conditional atlas palettes.

use serde::{Deserialize, Serialize};

use crate::{
    Error, Result, bank_image,
    catalog::{Index, Resource},
    composition::{self, DrawContext, SampleState, SourceFrame, SourcePart, WordRegion},
    formats, interface,
    palette::Palette,
    png::{IndexedImage, WordImage},
    sprite_bank,
};

/// Explicit source families, with no format guessing or capture fallback.
#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(tag = "schema")]
pub enum FrameContext {
    #[serde(rename = "bof3.sprite-frame-context/v1")]
    SpriteBank(SpriteFrameContext),
    #[serde(rename = "bof3.resident-frame-context/v1")]
    Resident(ResidentFrameContext),
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct SpriteFrameContext {
    pub root: usize,
    pub image: usize,
    pub palette: usize,
    pub cell_offset: usize,
    pub entity: Vec<u8>,
    pub graph_type: u8,
    pub texture_origin: [usize; 2],
    pub palette_origin: [usize; 2],
    pub draw_mode: u32,
    pub texture_window: u32,
    pub part_order: Vec<usize>,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct ResidentFrameContext {
    pub texture_resource: String,
    pub palette_resource: String,
    pub cell_offset: usize,
    pub entity: Vec<u8>,
    pub graph_type: u8,
    pub draw_mode: u32,
    pub texture_window: u32,
    pub part_order: Vec<usize>,
}

struct CellContext<'a> {
    entity: &'a [u8],
    graph_type: u8,
    draw_mode: u32,
    texture_window: u32,
    part_order: &'a [usize],
}

impl CellContext<'_> {
    fn sample(
        &self,
        cell: &crate::cell::Cell,
        texture: &[WordRegion<'_>],
        palette: &[WordRegion<'_>],
    ) -> Result<SourceFrame> {
        let invalid = |reason: &str| Error::Invalid(format!("source-frame context: {reason}"));
        if !(0x60..=0x98).contains(&self.entity.len()) {
            return Err(invalid("entity must contain 96 through 152 bytes"));
        }
        if self.part_order.len() != cell.parts.len() {
            return Err(invalid(
                "part order must include every cell part exactly once",
            ));
        }
        let mut seen = vec![false; cell.parts.len()];
        for &index in self.part_order {
            let slot = seen
                .get_mut(index)
                .ok_or_else(|| invalid("part order index exceeds cell parts"))?;
            if std::mem::replace(slot, true) {
                return Err(invalid("part order repeats a cell part"));
            }
        }
        let state = SampleState::from_commands(self.draw_mode, self.texture_window)?;
        let geometry = composition::compose(
            cell,
            DrawContext::from_entity(self.entity, self.graph_type)?,
        )?;
        let parts: Vec<_> = self
            .part_order
            .iter()
            .map(|&index| SourcePart {
                geometry: &geometry[index],
                state,
                texture,
                palette,
            })
            .collect();
        composition::assemble(&parts)
    }
}

/// Sample caller-selected records, without inferring an active entity or upload.
pub fn sprite_frame(source: &[u8], context: &SpriteFrameContext) -> Result<SourceFrame> {
    let invalid = |reason: &str| Error::Invalid(format!("source-frame context: {reason}"));
    let document = sprite_bank::decode(source)?;
    let root = document
        .roots
        .get(context.root)
        .ok_or_else(|| invalid("missing root"))?;
    let image = root
        .images
        .get(context.image)
        .ok_or_else(|| invalid("missing image"))?;
    let palette = root
        .palettes
        .get(context.palette)
        .ok_or_else(|| invalid("missing palette"))?;
    let cell = root
        .cells
        .iter()
        .find(|cell| cell.offset == context.cell_offset)
        .ok_or_else(|| invalid("cell offset is not a declared cell in this root"))?;
    let decoded = bank_image::decode(source, image.offset, image.end)?;
    let texture = WordImage {
        width: usize::from(image.width_words),
        height: usize::from(image.height),
        words: decoded
            .bytes
            .as_chunks::<2>()
            .0
            .iter()
            .map(|bytes| u16::from_le_bytes(*bytes))
            .collect(),
    };
    let palette = WordImage {
        width: palette.words.len(),
        height: 1,
        words: palette.words.to_vec(),
    };
    let texture = [WordRegion::new(
        context.texture_origin[0],
        context.texture_origin[1],
        &texture,
    )?];
    let palette = [WordRegion::new(
        context.palette_origin[0],
        context.palette_origin[1],
        &palette,
    )?];
    CellContext {
        entity: &context.entity,
        graph_type: context.graph_type,
        draw_mode: context.draw_mode,
        texture_window: context.texture_window,
        part_order: &context.part_order,
    }
    .sample(cell, &texture, &palette)
}

/// Sample a declared resident cell from separate original upload sources.
/// Descriptor coordinates identify candidate words, not completed GPU transfers.
pub(crate) fn resident_frame(
    source: &[u8],
    context: &ResidentFrameContext,
    texture: (&formats::Upload, &[u8]),
    palette: (&crate::palette::PaletteUpload, &[u8]),
) -> Result<SourceFrame> {
    let invalid = |reason: &str| Error::Invalid(format!("resident source frame: {reason}"));
    let document = crate::resident::decode(source)?;
    let cell = document
        .cells
        .iter()
        .find(|cell| cell.offset == context.cell_offset)
        .ok_or_else(|| invalid("cell offset is not a declared resident cell"))?;
    let (upload, bytes) = texture;
    if upload.chunks.checked_mul(formats::CHUNK_BYTES) != Some(bytes.len()) {
        return Err(invalid("texture upload length differs from its source"));
    }
    let chunks: Vec<_> = bytes
        .as_chunks::<{ formats::CHUNK_BYTES }>()
        .0
        .iter()
        .enumerate()
        .filter(|(index, _)| !upload.skipped_chunks.contains(index))
        .map(|(index, bytes)| {
            (
                upload.position(index),
                WordImage {
                    width: 32,
                    height: 32,
                    words: bytes
                        .as_chunks::<2>()
                        .0
                        .iter()
                        .map(|b| u16::from_le_bytes(*b))
                        .collect(),
                },
            )
        })
        .collect();
    let texture = chunks
        .iter()
        .map(|((x, y), image)| WordRegion::new(*x, *y, image))
        .collect::<Result<Vec<_>>>()?;
    let (upload, bytes) = palette;
    let words = upload.decode_words(bytes)?;
    let mut column = upload.vram_x_words;
    let mut row = upload.vram_y;
    let mut rows = Vec::new();
    let mut remaining = words.as_slice();
    while !remaining.is_empty() {
        let count = remaining.len().min(256 - column);
        rows.push((
            column,
            row,
            WordImage {
                width: count,
                height: 1,
                words: remaining[..count].to_vec(),
            },
        ));
        remaining = &remaining[count..];
        column = 0;
        row += 1;
    }
    let palette = rows
        .iter()
        .map(|(x, y, image)| WordRegion::new(*x, *y, image))
        .collect::<Result<Vec<_>>>()?;
    CellContext {
        entity: &context.entity,
        graph_type: context.graph_type,
        draw_mode: context.draw_mode,
        texture_window: context.texture_window,
        part_order: &context.part_order,
    }
    .sample(cell, &texture, &palette)
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub member: String,
    pub palette_resource: String,
    pub bits: u8,
    pub first_word: usize,
}

pub(crate) fn exports(index: &Index, resource: &Resource, directory: &str) -> Result<Vec<Info>> {
    interface::preview_bindings(resource)
        .into_iter()
        .enumerate()
        .map(|(number, (entry, bits, first_word))| {
            let palette = index
                .resources
                .iter()
                .find(|other| other.source == resource.source && other.entry == Some(entry))
                .ok_or_else(|| {
                    Error::Invalid(format!("{}: missing preview palette", resource.id))
                })?;
            Ok(Info {
                member: format!("{directory}/preview-{number}.png"),
                palette_resource: palette.id.clone(),
                bits,
                first_word,
            })
        })
        .collect()
}

pub(crate) fn image(
    index: &Index,
    resource: &Resource,
    view: &Info,
    snapshot: &[u8],
) -> Result<IndexedImage> {
    if !matches!(view.bits, 4 | 8) {
        return Err(Error::Invalid(
            "preview requires 4-bit or 8-bit indices".into(),
        ));
    }
    let palette_resource = index
        .resources
        .iter()
        .find(|other| other.id == view.palette_resource && other.source == resource.source)
        .ok_or_else(|| Error::Invalid("preview palette is outside its source snapshot".into()))?;
    let payload = |item: &Resource| {
        let end = item
            .offset
            .checked_add(item.size)
            .and_then(|end| usize::try_from(end).ok());
        usize::try_from(item.offset)
            .ok()
            .zip(end)
            .and_then(|(start, end)| snapshot.get(start..end))
            .ok_or_else(|| Error::Invalid(format!("{} exceeds preview snapshot", item.id)))
    };
    let words = palette_resource
        .palette_upload
        .as_ref()
        .ok_or_else(|| Error::Invalid("preview palette has no qualified word layout".into()))?
        .decode_words(payload(palette_resource)?)?;
    let end = view
        .first_word
        .checked_add(1usize << view.bits)
        .ok_or_else(|| Error::Invalid("preview palette range overflow".into()))?;
    let palette = Palette {
        words: words
            .get(view.first_word..end)
            .ok_or_else(|| Error::Invalid("preview palette exceeds its source words".into()))?
            .to_vec(),
    };
    let upload = resource
        .upload
        .as_ref()
        .ok_or_else(|| Error::Invalid("preview atlas has no qualified word layout".into()))?;
    palette.image(
        upload.width_words() * usize::from(16 / view.bits),
        upload.height(),
        formats::indices(&upload.linear_words(payload(resource)?)?, view.bits)?,
    )
}
