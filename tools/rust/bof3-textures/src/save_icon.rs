//! Embedded original-US icon data copied into the game's memory-card header.

use serde::{Deserialize, Serialize};

use crate::{Error, Result, digest::sha256_hex, formats, palette::Palette};

const PAYLOAD_SHA256: &str = "6e7b93d4fd75586413b5ab3a6785d7b2492d8d1b566a3a6d79941bb656131a4d";
const PAYLOAD_BYTES: usize = 5268;

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Info {
    pub width: usize,
    pub height: usize,
    pub palette_offset: usize,
    pub frame_offsets: [usize; 3],
}

impl Info {
    fn layout() -> Self {
        Self {
            width: 16,
            height: 16,
            palette_offset: 0x1268,
            frame_offsets: [0x1294, 0x1354, 0x1414],
        }
    }

    fn validate(&self, bytes: &[u8]) -> Result<()> {
        if self != &Self::layout() || bytes.len() != PAYLOAD_BYTES {
            return Err(Error::Invalid(
                "save-icon layout differs from its original source allocation".into(),
            ));
        }
        Ok(())
    }
}

/// Match a physical original source without merging identical archive entries.
pub fn qualify(source: &str, entry: usize, argument: u32, bytes: &[u8]) -> Result<Option<Info>> {
    if !matches!(
        (source, entry),
        ("BIN/ETC/SHOP.EMI", 8) | ("BIN/ETC/START.EMI", 9)
    ) {
        return Ok(None);
    }
    if argument != 0x801e_ec00 || sha256_hex(bytes) != PAYLOAD_SHA256 {
        return Err(Error::Invalid(format!(
            "{source}#{entry} differs from its qualified save-icon producer"
        )));
    }
    let info = Info::layout();
    info.validate(bytes)?;
    Ok(Some(info))
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Images {
    pub palette: Palette,
    pub frames: [Vec<u8>; 3],
}

/// Read original or edited image ranges. This does not qualify their provenance.
pub fn decode(bytes: &[u8], info: &Info) -> Result<Images> {
    info.validate(bytes)?;
    let palette = Palette::from_bytes(&bytes[info.palette_offset..info.palette_offset + 32])?;
    let mut frames = [Vec::new(), Vec::new(), Vec::new()];
    for (frame, offset) in frames.iter_mut().zip(info.frame_offsets) {
        let words: Vec<u16> = bytes[offset..offset + 128]
            .as_chunks::<2>()
            .0
            .iter()
            .map(|word| u16::from_le_bytes(*word))
            .collect();
        *frame = formats::indices(&words, 4)?;
    }
    Ok(Images { palette, frames })
}

/// Change only the palette and copied frame bytes; code and frame gaps stay exact.
pub fn encode(original: &[u8], info: &Info, images: &Images) -> Result<Vec<u8>> {
    info.validate(original)?;
    if sha256_hex(original) != PAYLOAD_SHA256 {
        return Err(Error::Invalid(
            "save-icon edits require the qualified original producer snapshot".into(),
        ));
    }
    if images.palette.words.len() != 16 {
        return Err(Error::Invalid(
            "save icons require exactly 16 original PSX palette words".into(),
        ));
    }
    let mut output = original.to_vec();
    output[info.palette_offset..info.palette_offset + 32]
        .copy_from_slice(&images.palette.encode()?);
    for (frame, offset) in images.frames.iter().zip(info.frame_offsets) {
        if frame.len() != 256 {
            return Err(Error::Invalid(
                "save-icon frames must keep their 16 by 16 index planes".into(),
            ));
        }
        let words = formats::words_from_indices(frame, 4)?;
        let bytes: Vec<u8> = words.iter().flat_map(|word| word.to_le_bytes()).collect();
        output[offset..offset + 128].copy_from_slice(&bytes);
    }
    Ok(output)
}

pub fn validate_edit(original: &[u8], edited: &[u8], info: &Info) -> Result<()> {
    if encode(original, info, &decode(edited, info)?)? != edited {
        return Err(Error::Invalid(
            "save-icon edits must preserve producer code, frame gaps and unrelated bytes".into(),
        ));
    }
    Ok(())
}
