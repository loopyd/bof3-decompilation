//! Unscaled packet geometry from SLUS_004.22@0x8014C6F8 and 0x8014CF9C.
//! The caller supplies the entity, graph type and resolved CLUT. These values
//! do not establish source residency, packet submission or rendered appearance.
use crate::{Error, Result, cell::Cell};
use serde::Serialize;

fn invalid(reason: &str) -> Error {
    Error::Codec(format!("sprite composition: {reason}"))
}

/// Qualified unscaled entity fields. Construct from the same entity that the
/// original consumer references through scratchpad address 0x1F800044.
#[derive(Clone, Copy, Debug)]
pub struct DrawContext {
    anchor_x: u16,
    anchor_y: u16,
    page: u8,
    v_base: u8,
    indexed_8bit: bool,
    mirrored: bool,
    blend: u8,
    clut: u16,
}

impl DrawContext {
    /// Read the original fields through byte 0x5F. `clut` is the original CLUT
    /// helper's result, not a palette ordinal. Graph types 1/2 and scaled
    /// entities refuse because their complete geometry is not qualified here.
    pub fn from_entity(entity: &[u8], graph_type: u8, clut: u16) -> Result<Self> {
        if entity.len() < 0x60 {
            return Err(invalid("entity is shorter than 0x60 bytes"));
        }
        if matches!(graph_type, 1 | 2) {
            return Err(invalid("graph type 1/2 uses an unsupported page encoding"));
        }
        if entity[0x48] != 0 {
            return Err(invalid("scaled entity geometry is unsupported"));
        }
        Ok(Self {
            anchor_x: u16::from_le_bytes([entity[0x2e], entity[0x2f]]),
            anchor_y: u16::from_le_bytes([entity[0x30], entity[0x31]]),
            page: entity[0x25],
            v_base: entity[0x26],
            indexed_8bit: entity[0x28] != 0,
            mirrored: entity[0x2a] != 0,
            blend: entity[0x5c] & 3,
            clut,
        })
    }

    fn tpage(self, correction: i16) -> u16 {
        let page = i16::from(self.page) + correction;
        // The original signed division truncates toward zero, including page -1.
        let quotient = page / 16;
        let remainder = page - quotient * 16;
        let row = quotient << 8;
        ((remainder & 15) | ((row & 0x100) >> 4) | ((row & 0x200) << 2)) as u16
            | (u16::from(self.blend) << 5)
            | if self.indexed_8bit { 0x80 } else { 0 }
    }
}

/// Stored packet coordinate bits. XY halfwords and UV bytes wrap independently.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct Vertex {
    pub x: u16,
    pub y: u16,
    pub u: u8,
    pub v: u8,
}

/// Geometry in original part order. Quad vertices use original packet order.
/// Rectangle `tpage` is the preceding draw-mode selection. This is geometry,
/// not a GPU packet, rasterizer, palette binding or composited image.
#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
#[serde(tag = "primitive", rename_all = "kebab-case")]
pub enum Geometry {
    Rectangle {
        origin: Vertex,
        width: u8,
        height: u8,
        tpage: u16,
        clut: u16,
    },
    Quad {
        vertices: [Vertex; 4],
        tpage: u16,
        clut: u16,
    },
}

/// Compose qualified unscaled geometry without selecting a bank or image.
/// Unlike a bitmap mirror, original flip dispatch changes primitive type,
/// placement, UV endpoints and sometimes the page. No raster sampling is implied.
pub fn unscaled(cell: &Cell, context: DrawContext) -> Result<Vec<Geometry>> {
    if cell.parts.len() > 255 {
        return Err(invalid("part count exceeds the original byte count"));
    }
    cell.parts
        .iter()
        .map(|part| {
            let (width, height) = part.dimensions()?;
            let x = context.anchor_x;
            let y = context.anchor_y.wrapping_add_signed(i16::from(part.y));
            let v = context.v_base.wrapping_add(part.v);
            let state = u8::from(part.horizontal_flip) | (u8::from(context.mirrored) << 1);
            if matches!(state, 0 | 3) {
                let x = if state == 0 {
                    x.wrapping_add_signed(i16::from(part.x))
                } else {
                    x.wrapping_sub_signed(i16::from(part.x))
                        .wrapping_sub(u16::from(width))
                };
                return Ok(Geometry::Rectangle {
                    origin: Vertex { x, y, u: part.u, v },
                    width,
                    height,
                    tpage: context.tpage(0),
                    clut: context.clut,
                });
            }
            let correction = if !context.indexed_8bit {
                0
            } else if part.u < 40 {
                -1
            } else if part.u <= 160 {
                0
            } else {
                1
            };
            let u = (i16::from(part.u) - 128 * correction - 1) as u8;
            let (x0, x1, u0, u1) = if state == 1 {
                let x0 = x.wrapping_add_signed(i16::from(part.x));
                (
                    x0,
                    x0.wrapping_add(u16::from(width)),
                    u.wrapping_add(width),
                    u,
                )
            } else {
                let x0 = x.wrapping_sub_signed(i16::from(part.x));
                (
                    x0,
                    x0.wrapping_sub(u16::from(width)),
                    u,
                    u.wrapping_add(width),
                )
            };
            let y1 = y.wrapping_add(u16::from(height));
            let v1 = v.wrapping_add(height);
            Ok(Geometry::Quad {
                vertices: [
                    Vertex { x: x0, y, u: u0, v },
                    Vertex { x: x1, y, u: u1, v },
                    Vertex {
                        x: x0,
                        y: y1,
                        u: u0,
                        v: v1,
                    },
                    Vertex {
                        x: x1,
                        y: y1,
                        u: u1,
                        v: v1,
                    },
                ],
                tpage: context.tpage(correction),
                clut: context.clut,
            })
        })
        .collect()
}
