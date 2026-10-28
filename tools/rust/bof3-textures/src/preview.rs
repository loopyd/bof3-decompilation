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

/// Explicit caller bindings for a three-root bank source derivative.
#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct FrameContext {
    pub schema: String,
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

/// Sample caller-selected records, without inferring an active entity or upload.
pub fn sprite_frame(source: &[u8], context: &FrameContext) -> Result<SourceFrame> {
    let invalid = |reason: &str| Error::Invalid(format!("source-frame context: {reason}"));
    if context.schema != "bof3.sprite-frame-context/v1" {
        return Err(invalid("unsupported schema"));
    }
    if !(0x60..=0x98).contains(&context.entity.len()) {
        return Err(invalid("entity must contain 96 through 152 bytes"));
    }
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
    if context.part_order.len() != cell.parts.len() {
        return Err(invalid(
            "part order must include every cell part exactly once",
        ));
    }
    let mut seen = vec![false; cell.parts.len()];
    for &index in &context.part_order {
        let slot = seen
            .get_mut(index)
            .ok_or_else(|| invalid("part order index exceeds cell parts"))?;
        if std::mem::replace(slot, true) {
            return Err(invalid("part order repeats a cell part"));
        }
    }
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
    let texture = WordRegion::new(
        context.texture_origin[0],
        context.texture_origin[1],
        &texture,
    )?;
    let palette = WordRegion::new(
        context.palette_origin[0],
        context.palette_origin[1],
        &palette,
    )?;
    let state = SampleState::from_commands(context.draw_mode, context.texture_window)?;
    let geometry = composition::compose(
        cell,
        DrawContext::from_entity(&context.entity, context.graph_type)?,
    )?;
    let parts: Vec<_> = context
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
