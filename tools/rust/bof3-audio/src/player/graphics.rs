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

fn copy_font_cells(
    font: &mut [u8],
    original: &[u8],
    column: usize,
    row: usize,
    destination: usize,
    cells: usize,
) {
    for y in 0..8 {
        let source = (row + y) * 128 + column * 4;
        let target = (destination / 32 * 8 + y) * 128 + destination % 32 * 4;
        font[target..target + cells * 4].copy_from_slice(&original[source..source + cells * 4]);
    }
}

fn add_button_glyphs(font: &mut [u8], original: &[u8], palette: &mut Vec<u8>) {
    // The native renderer maps 0xe0 to the first cell of row 48.
    const FIRST: usize = 0xe0 - 0x20;
    const ROW: usize = FIRST / 32 * 8;
    let neutral = palette[..16].to_vec();
    palette.extend_from_slice(&neutral);
    for (index, (column, bright_ink, rgb)) in [
        (29, 14, [112u16, 160, 248]), // Cross
        (27, 9, [248, 104, 120]),     // Circle
        (30, 11, [240, 144, 216]),    // Square
        (28, 12, [112, 224, 176]),    // Triangle
    ]
    .into_iter()
    .enumerate()
    {
        let bright = 8 + index as u8 * 2;
        for light in [4, 3] {
            let [r, g, b] = rgb.map(|component| (component * light / 4) >> 3);
            palette.extend_from_slice(&(r | (g << 5) | (b << 10)).to_le_bytes());
        }
        copy_font_cells(font, original, column, 0, FIRST + index, 1);
        let tint = |ink| {
            if ink < 9 {
                ink
            } else {
                bright + u8::from(ink != bright_ink)
            }
        };
        for y in ROW..ROW + 8 {
            for byte in &mut font[y * 128 + index * 4..y * 128 + index * 4 + 4] {
                *byte = tint(*byte & 15) | (tint(*byte >> 4) << 4);
            }
        }
    }

    const UP: [u8; 7] = [0x08, 0x1c, 0x3e, 0x7f, 0x1c, 0x1c, 0x1c];
    for direction in 0..4 {
        for y in 0..8 {
            let row = (ROW + y) * 128 + (4 + direction) * 4;
            font[row..row + 4].fill(0);
            if y == 7 {
                continue;
            }
            for x in 0..7 {
                let (sx, sy) = match direction {
                    0 => (x, y),
                    1 => (x, 6 - y),
                    2 => (6 - y, x),
                    _ => (y, 6 - x),
                };
                if UP[sy] & (0x40 >> sx) != 0 {
                    font[row + x / 2] |= 1 << ((x % 2) * 4);
                }
            }
        }
    }

    copy_font_cells(font, original, 0, 16, FIRST + 8, 4);
    copy_font_cells(font, original, 4, 16, FIRST + 12, 2);
    copy_font_cells(font, original, 6, 16, FIRST + 14, 3);
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
    let mut palette = entry(
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
    // in a second alphabet. Copy those exact glyphs into the ASCII slots.
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
        copy_font_cells(&mut ascii_font, &original_font, column, row, destination, 1);
    }
    add_button_glyphs(&mut ascii_font, &original_font, &mut palette);
    Ok(Graphics {
        font: ascii_font,
        palette,
        logo,
        identity: serde_json::json!({
            "font": "BIN/ETC/FIRST.EMI#3", "font_palette": "BIN/ETC/FIRST.EMI#8",
            "logo": ["BIN/ETC/DEMO.EMI#3", "BIN/ETC/DEMO.EMI#4", "BIN/ETC/DEMO.EMI#7"],
            "font_renderer": "SLUS_004.22@0x80150098", "logo_size": [160, 80],
            "font_mapping": "original lowercase and underscore glyphs copied to ASCII slots",
            "button_glyphs": "inline PSX face, directional, shoulder, Start and Select glyphs",
        }),
    })
}
