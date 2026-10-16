use serde::{Deserialize, Serialize};

use crate::{Error, Result, formats::color_rgba, png::IndexedImage};

/// Type-0 words copied through the visible palette staging buffer.
/// CLUT selection and asset uses require separate consumer evidence.
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct PaletteUpload {
    pub ram_address: u32,
    pub vram_x_words: usize,
    pub vram_y: usize,
    pub word_count: usize,
}

impl PaletteUpload {
    pub fn from_argument(address: u32, byte_count: usize) -> Option<Self> {
        const BASE: u32 = 0x8003_3800;
        const CLEARED_ROWS: u32 = 0x8003_7000;
        let end = address.checked_add(u32::try_from(byte_count).ok()?)?;
        if address < BASE
            || end > CLEARED_ROWS
            || !address.is_multiple_of(32)
            || byte_count == 0
            || !byte_count.is_multiple_of(32)
        {
            return None;
        }
        let offset = (address - BASE) as usize;
        Some(Self {
            ram_address: address,
            vram_x_words: offset % 512 / 2,
            vram_y: 480 + offset / 512,
            word_count: byte_count / 2,
        })
    }

    pub fn decode_words(&self, bytes: &[u8]) -> Result<Vec<u16>> {
        self.validate(bytes)?;
        Ok(bytes
            .as_chunks::<2>()
            .0
            .iter()
            .map(|word| u16::from_le_bytes(*word))
            .collect())
    }

    pub fn encode_words(&self, words: &[u16], original: &[u8]) -> Result<Vec<u8>> {
        self.validate(original)?;
        if words.len() != self.word_count {
            return Err(Error::Invalid(
                "edited palette word count differs from its source allocation".into(),
            ));
        }
        Ok(words.iter().flat_map(|word| word.to_le_bytes()).collect())
    }

    fn validate(&self, bytes: &[u8]) -> Result<()> {
        if Self::from_argument(self.ram_address, bytes.len()).as_ref() != Some(self) {
            return Err(Error::Invalid(
                "palette span differs from the visible source buffer".into(),
            ));
        }
        Ok(())
    }
}

/// Original PSX color words. Duplicate colors, palette order, and STP are retained.
#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(deny_unknown_fields)]
pub struct Palette {
    pub words: Vec<u16>,
}

impl Palette {
    pub fn validate(&self) -> Result<()> {
        if ![16, 256].contains(&self.words.len()) {
            return Err(Error::Invalid(
                "PSX indexed palettes require exactly 16 or 256 color words".into(),
            ));
        }
        Ok(())
    }

    pub fn from_bytes(bytes: &[u8]) -> Result<Self> {
        if ![32, 512].contains(&bytes.len()) {
            return Err(Error::Invalid(
                "palette extent must contain 32 or 512 bytes".into(),
            ));
        }
        let palette = Self {
            words: bytes
                .as_chunks::<2>()
                .0
                .iter()
                .map(|word| u16::from_le_bytes(*word))
                .collect(),
        };
        palette.validate()?;
        Ok(palette)
    }

    pub fn encode(&self) -> Result<Vec<u8>> {
        self.validate()?;
        Ok(self
            .words
            .iter()
            .flat_map(|word| word.to_le_bytes())
            .collect())
    }

    pub fn image(&self, width: usize, height: usize, indices: Vec<u8>) -> Result<IndexedImage> {
        self.validate()?;
        if indices
            .iter()
            .any(|index| usize::from(*index) >= self.words.len())
        {
            return Err(Error::Invalid(
                "image uses indices outside its declared palette".into(),
            ));
        }
        let mut palette_bgra = vec![0; 1024];
        for (word, bgra) in self.words.iter().zip(palette_bgra.as_chunks_mut::<4>().0) {
            let [r, g, b, a] = color_rgba(*word);
            *bgra = [b, g, r, a];
        }
        let image = IndexedImage {
            width,
            height,
            indices,
            palette_bgra,
        };
        image.validate()?;
        Ok(image)
    }

    pub fn import_indices(&self, image: &IndexedImage) -> Result<Vec<u8>> {
        let expected = self.image(image.width, image.height, image.indices.clone())?;
        if expected.palette_bgra != image.palette_bgra {
            return Err(Error::Invalid("PNG colors differ from palette words; edit the exact palette-word document explicitly".into()));
        }
        Ok(image.indices.clone())
    }
}
