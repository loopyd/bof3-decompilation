//! Read-only atlas derivatives from conditional original palette associations.

use serde::{Deserialize, Serialize};

use crate::{
    Error, Result,
    catalog::{Index, Resource},
    formats, interface,
    palette::Palette,
    png::IndexedImage,
};

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
