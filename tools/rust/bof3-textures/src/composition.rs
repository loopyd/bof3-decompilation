//! Packet geometry from SLUS_004.22@0x8014C6F8 and 0x8014CF9C.
//! The caller supplies the entity and graph type. These values
//! do not establish source residency, packet submission or rendered appearance.
use crate::{Error, Result, cell::Cell};
use serde::Serialize;

fn invalid(reason: &str) -> Error {
    Error::Codec(format!("sprite composition: {reason}"))
}

/// Qualified entity fields. Construct from the same entity that the
/// original consumer references through scratchpad address 0x1F800044.
#[derive(Clone, Copy, Debug)]
pub struct DrawContext {
    anchor_x: u16,
    anchor_y: u16,
    page: u8,
    v_base: u8,
    indexed_8bit: bool,
    mirrored: bool,
    scale: Option<[i32; 2]>,
    blend: u8,
    clut: u16,
}

impl DrawContext {
    /// Read the original fields through byte 0x5F, including the CLUT helper's
    /// raw encoded result. Graph types 1/2 use an unsupported page encoding.
    pub fn from_entity(entity: &[u8], graph_type: u8) -> Result<Self> {
        if entity.len() < 0x60 {
            return Err(invalid("entity is shorter than 0x60 bytes"));
        }
        if matches!(graph_type, 1 | 2) {
            return Err(invalid("graph type 1/2 uses an unsupported page encoding"));
        }
        let scale = (entity[0x48] != 0).then(|| {
            [
                i32::from_le_bytes(entity[0x40..0x44].try_into().unwrap()),
                i32::from_le_bytes(entity[0x44..0x48].try_into().unwrap()),
            ]
        });
        Ok(Self {
            anchor_x: u16::from_le_bytes([entity[0x2e], entity[0x2f]]),
            anchor_y: u16::from_le_bytes([entity[0x30], entity[0x31]]),
            page: entity[0x25],
            v_base: entity[0x26],
            indexed_8bit: entity[0x28] != 0,
            mirrored: entity[0x2a] != 0,
            scale,
            blend: entity[0x5c] & 3,
            clut: clut(entity[0x24], entity[0x27], entity[0x28]),
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

/// Original helper at 0x8014CE78, including its mode>=4 shared path.
fn clut(flags: u8, selector: u8, mode: u8) -> u16 {
    let base = if flags & 4 == 0 { 480 } else { 496 };
    let selector = u16::from(selector);
    let (row, column) = match mode {
        0 => (selector >> 4, selector & 15),
        1 => (selector, 0),
        2 => (selector >> 3, (selector & 7) << 1),
        3 => (selector >> 2, (selector & 3) << 2),
        _ => (selector >> 1, (selector & 1) << 3),
    };
    ((base + row) << 6) | column
}

/// Original 0x8014CE48 keeps the low product and rounds by the scale's sign.
fn scaled(value: i32, scale: i32) -> i16 {
    let product = value.wrapping_mul(scale);
    let high = (product >> 16) as i16;
    if scale >= 0 && product & 0xffff != 0 {
        return high.wrapping_add(1);
    }
    high
}

fn shrink(scale: i32) -> u8 {
    u8::from((scale as u32).wrapping_add(0x1e000) > 0x3c000)
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

/// Compose qualified geometry without selecting a bank or image.
/// Unlike a bitmap mirror, original flip dispatch changes primitive type,
/// placement, UV endpoints and sometimes the page. No raster sampling is implied.
pub fn compose(cell: &Cell, context: DrawContext) -> Result<Vec<Geometry>> {
    if cell.parts.len() > 255 {
        return Err(invalid("part count exceeds the original byte count"));
    }
    cell.parts
        .iter()
        .map(|part| {
            let (width, height) = part.dimensions()?;
            let x = context.anchor_x;
            let y = context.anchor_y;
            let v = context.v_base.wrapping_add(part.v);
            let state = u8::from(part.horizontal_flip)
                | (u8::from(context.mirrored) << 1)
                | (u8::from(context.scale.is_some()) << 2);
            if matches!(state, 0 | 3) {
                let x = if state == 0 {
                    x.wrapping_add_signed(i16::from(part.x))
                } else {
                    x.wrapping_sub_signed(i16::from(part.x))
                        .wrapping_sub(u16::from(width))
                };
                return Ok(Geometry::Rectangle {
                    origin: Vertex {
                        x,
                        y: y.wrapping_add_signed(i16::from(part.y)),
                        u: part.u,
                        v,
                    },
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
            let u =
                (i16::from(part.u) - 128 * correction - i16::from(!matches!(state, 4 | 7))) as u8;
            let (x0, x1, y0, y1, u0, u1, v1) = if let Some([scale_x, scale_y]) = context.scale {
                let offset_x = scaled(i32::from(part.x), scale_x);
                let scaled_width = scaled(i32::from(width), scale_x);
                let (x0, x1) = if state & 2 == 0 {
                    let x0 = x.wrapping_add_signed(offset_x);
                    (x0, x0.wrapping_add_signed(scaled_width))
                } else {
                    let x1 = x.wrapping_sub_signed(offset_x);
                    (x1.wrapping_sub_signed(scaled_width), x1)
                };
                let y0 = y.wrapping_add_signed(scaled(i32::from(part.y), scale_y));
                let y1 = y0.wrapping_add_signed(scaled(i32::from(height), scale_y));
                let (u0, u1) = if matches!(state, 4 | 7) {
                    (u, u.wrapping_add(width).wrapping_sub(shrink(scale_x)))
                } else {
                    (u.wrapping_add(width), u.wrapping_add(shrink(scale_x)))
                };
                (
                    x0,
                    x1,
                    y0,
                    y1,
                    u0,
                    u1,
                    v.wrapping_add(height).wrapping_sub(shrink(scale_y)),
                )
            } else if state == 1 {
                let x0 = x.wrapping_add_signed(i16::from(part.x));
                let y0 = y.wrapping_add_signed(i16::from(part.y));
                (
                    x0,
                    x0.wrapping_add(u16::from(width)),
                    y0,
                    y0.wrapping_add(u16::from(height)),
                    u.wrapping_add(width),
                    u,
                    v.wrapping_add(height),
                )
            } else {
                let x0 = x.wrapping_sub_signed(i16::from(part.x));
                let y0 = y.wrapping_add_signed(i16::from(part.y));
                (
                    x0,
                    x0.wrapping_sub(u16::from(width)),
                    y0,
                    y0.wrapping_add(u16::from(height)),
                    u,
                    u.wrapping_add(width),
                    v.wrapping_add(height),
                )
            };
            Ok(Geometry::Quad {
                vertices: [
                    Vertex {
                        x: x0,
                        y: y0,
                        u: u0,
                        v,
                    },
                    Vertex {
                        x: x1,
                        y: y0,
                        u: u1,
                        v,
                    },
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
