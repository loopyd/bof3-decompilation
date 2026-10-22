//! Original US FIRST font and DEMO logo panels, tiled as the game's type-3 uploads.
use crate::{digest::sha256_hex, Result};
use emi_ex_v2::image::ArchiveImage;
use std::{fs, path::Path};

pub struct Graphics {
    pub font: Vec<u8>,
    pub palette: Vec<u8>,
    pub logo: Vec<u8>,
    pub identity: serde_json::Value,
}

fn entry(image: &ArchiveImage, index: usize, kind: u16, hash: &str) -> Result<Vec<u8>> {
    let bytes = image.entry(index)?;
    if image.entries()[index].file_type != kind || sha256_hex(bytes) != hash {
        return Err(
            format!("player graphics: entry {index} differs from the original US asset").into(),
        );
    }
    Ok(bytes.to_vec())
}

fn linear(bytes: &[u8], columns: usize) -> Vec<u8> {
    let width = columns * 64;
    let mut pixels = vec![0; bytes.len()];
    for (index, chunk) in bytes.as_chunks::<2048>().0.iter().enumerate() {
        for row in 0..32 {
            let start = (index / columns * 32 + row) * width + index % columns * 64;
            pixels[start..start + 64].copy_from_slice(&chunk[row * 64..row * 64 + 64]);
        }
    }
    pixels
}

pub fn prepare(root: &Path) -> Result<Graphics> {
    let first = ArchiveImage::from_bytes(fs::read(root.join("BIN/ETC/FIRST.EMI"))?)?;
    let demo = ArchiveImage::from_bytes(fs::read(root.join("BIN/ETC/DEMO.EMI"))?)?;
    let font = entry(
        &first,
        3,
        3,
        "d72c7f620b52d93f75a979fb9ae9892aa91d687a4f71eac7605b00795c58493d",
    )?;
    let palette = entry(
        &first,
        8,
        0,
        "166ead38de405cf28c1cfc35a7c4a2b9b4a69ae95a128b970cf4b9df8b4567c7",
    )?;
    let title = entry(
        &demo,
        3,
        3,
        "17e0b3058b76fbdd0d0527293464d3148c8b24dbbf77b41922019b7f92bb410a",
    )?;
    let numeral = entry(
        &demo,
        4,
        3,
        "5587f56d745abffaf77494349b0b902120eaf5cd3dbf521b6999f2e02c3b471d",
    )?;
    let colors = entry(
        &demo,
        7,
        0,
        "25389de257006aa7e940702803b621bae0bb0d6c4d46921eb80c4e8d765e1e06",
    )?;
    // GAME#1@0x801D1C6C records 2..5 and updateWindows' original screen positions.
    let mut canvas = vec![0u16; 320 * 184];
    for (atlas, palette_start, panels) in [
        (
            linear(&numeral, 4),
            512,
            vec![(26, 24, 0, 0, 240, 160), (266, 136, 0, 160, 48, 48)],
        ),
        (
            linear(&title, 4),
            0,
            vec![(-6, 28, 0, 0, 224, 128), (218, 28, 0, 128, 96, 128)],
        ),
    ] {
        for (x, y, u, v, width, height) in panels {
            for dy in 0..height {
                for dx in 0..width {
                    let screen_x = x + dx;
                    let screen_y = y + dy;
                    if !(0..320).contains(&screen_x) || !(0..184).contains(&screen_y) {
                        continue;
                    }
                    let index = usize::from(atlas[((v + dy) * 256 + u + dx) as usize]);
                    let at = palette_start + index * 2;
                    let color = u16::from_le_bytes([colors[at], colors[at + 1]]);
                    if color & 0x7fff != 0 {
                        canvas[(screen_y * 320 + screen_x) as usize] = color & 0x7fff;
                    }
                }
            }
        }
    }
    let mut logo = Vec::with_capacity(160 * 80 * 2);
    for y in 0..80 {
        for x in 0..160 {
            logo.extend_from_slice(&canvas[(y * 2 + 24) * 320 + x * 2].to_le_bytes());
        }
    }
    let mut ascii_font = linear(&font, 2);
    let original_font = ascii_font.clone();
    // The original 8x8 renderer uses ASCII positions; FIRST keeps lowercase
    // in a second alphabet. Copy those exact glyphs over unused controller icons.
    for letter in 0..27 {
        let (column, row) = if letter < 13 {
            (18 + letter, 128)
        } else if letter < 26 {
            (letter - 13, 136)
        } else {
            (13, 72)
        };
        let destination = if letter < 26 {
            usize::from(b'a' - b' ') + letter
        } else {
            usize::from(b'_' - b' ')
        };
        for y in 0..8 {
            let source = (row + y) * 128 + column * 4;
            let target = (destination / 32 * 8 + y) * 128 + destination % 32 * 4;
            ascii_font[target..target + 4].copy_from_slice(&original_font[source..source + 4]);
        }
    }
    Ok(Graphics {
        font: ascii_font,
        palette,
        logo,
        identity: serde_json::json!({
            "font": "BIN/ETC/FIRST.EMI#3", "font_palette": "BIN/ETC/FIRST.EMI#8",
            "logo": ["BIN/ETC/DEMO.EMI#3", "BIN/ETC/DEMO.EMI#4", "BIN/ETC/DEMO.EMI#7"],
            "font_renderer": "SLUS_004.22@0x80150098", "logo_size": [160, 80],
            "font_mapping": "original lowercase and underscore glyphs copied to ASCII slots",
        }),
    })
}
