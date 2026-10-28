//! Packet geometry from SLUS_004.22@0x8014C6F8 and 0x8014CF9C.
//! The caller supplies the entity and graph type. These values
//! do not establish source residency, packet submission or rendered appearance.
use crate::{Error, Result, cell::Cell, png::WordImage};
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

/// A caller-qualified rectangular source in 1024-word by 512-row VRAM coordinates.
/// Construction validates bounds, not source identity or upload residency.
#[derive(Clone, Copy, Debug)]
pub struct WordRegion<'a> {
    x_words: usize,
    y: usize,
    image: &'a WordImage,
}

impl<'a> WordRegion<'a> {
    pub fn new(x_words: usize, y: usize, image: &'a WordImage) -> Result<Self> {
        image.validate()?;
        if x_words
            .checked_add(image.width)
            .is_none_or(|end| end > 1024)
            || y.checked_add(image.height).is_none_or(|end| end > 512)
        {
            return Err(invalid("source region exceeds retail VRAM"));
        }
        Ok(Self { x_words, y, image })
    }

    fn word(self, x: usize, y: usize) -> Option<u16> {
        let x = x.checked_sub(self.x_words)?;
        let y = y.checked_sub(self.y)?;
        if x >= self.image.width || y >= self.image.height {
            return None;
        }
        self.image.words.get(y * self.image.width + x).copied()
    }
}

/// Explicit GP0(E1h/E2h) state for indexed source sampling.
/// Extended VRAM, hardware rectangle flips and direct color remain unsupported.
#[derive(Clone, Copy, Debug)]
pub struct SampleState {
    draw_mode: u32,
    texture_window: u32,
}

impl SampleState {
    pub fn from_commands(draw_mode: u32, texture_window: u32) -> Result<Self> {
        if draw_mode >> 24 != 0xe1 || draw_mode & 0x00ff_f800 != 0 {
            return Err(invalid(
                "source sampling requires a retail E1 command without hardware flips",
            ));
        }
        if (draw_mode >> 7) & 3 > 1 {
            return Err(invalid("source sampling requires 4-bit or 8-bit indices"));
        }
        if texture_window & 0xfff0_0000 != 0xe200_0000 {
            return Err(invalid("source sampling requires an explicit E2 command"));
        }
        Ok(Self {
            draw_mode,
            texture_window,
        })
    }

    fn window(self, coordinate: u8, shift: u32) -> u8 {
        let mask = ((self.texture_window >> shift) & 31) as u8;
        let offset = ((self.texture_window >> (shift + 10)) & 31) as u8;
        (coordinate & !(mask << 3)) | ((offset & mask) << 3)
    }
}

/// Indexed source selection, independent of framebuffer writes.
struct Texels<'a> {
    state: SampleState,
    texture: &'a [WordRegion<'a>],
    palette: &'a [WordRegion<'a>],
    tpage: u16,
    clut: u16,
}

fn source_word(regions: &[WordRegion<'_>], x: usize, y: usize, kind: &str) -> Result<u16> {
    let mut selected = None;
    for region in regions {
        if let Some(word) = region.word(x, y) {
            if selected.is_some_and(|previous| previous != word) {
                return Err(invalid(&format!(
                    "overlapping {kind} regions disagree at ({x}, {y})"
                )));
            }
            selected = Some(word);
        }
    }
    selected.ok_or_else(|| {
        invalid(&format!(
            "selected {kind} word ({x}, {y}) is outside its supplied source regions"
        ))
    })
}

impl Texels<'_> {
    fn color(&self, u: u8, v: u8) -> Result<u16> {
        let bits = if self.tpage & 0x80 == 0 { 4 } else { 8 };
        let per_word = 16 / bits;
        let u = usize::from(self.state.window(u, 0));
        let v = usize::from(self.state.window(v, 5));
        let page_x = usize::from(self.tpage & 15) * 64;
        let page_y = usize::from((self.tpage >> 4) & 1) * 256;
        let word = source_word(self.texture, page_x + u / per_word, page_y + v, "texture")?;
        let index = usize::from((word >> ((u % per_word) * bits)) & ((1 << bits) - 1));
        source_word(
            self.palette,
            usize::from(self.clut & 63) * 16 + index,
            usize::from(self.clut >> 6),
            "palette",
        )
    }
}

fn packet_coordinate(value: u16) -> i32 {
    i32::from(((value << 5) as i16) >> 5)
}

impl Geometry {
    fn bounds(&self) -> Result<[i32; 4]> {
        match *self {
            Self::Rectangle {
                origin,
                width,
                height,
                ..
            } => {
                if width == 0 || height == 0 {
                    return Err(invalid("source frame contains an empty rectangle"));
                }
                let x = packet_coordinate(origin.x);
                let y = packet_coordinate(origin.y);
                Ok([x, y, x + i32::from(width), y + i32::from(height)])
            }
            Self::Quad {
                vertices: [a, b, c, d],
                ..
            } => {
                let [x0, x1, y0, y1] = [
                    packet_coordinate(a.x),
                    packet_coordinate(b.x),
                    packet_coordinate(a.y),
                    packet_coordinate(c.y),
                ];
                if a.x != c.x
                    || b.x != d.x
                    || a.y != b.y
                    || c.y != d.y
                    || a.u != c.u
                    || b.u != d.u
                    || a.v != b.v
                    || c.v != d.v
                {
                    return Err(invalid(
                        "quad does not have the qualified axis-aligned XY/UV layout",
                    ));
                }
                if x0 == x1 || y0 == y1 {
                    return Err(invalid("source frame contains a zero-area quad"));
                }
                if (x1 - x0).abs() >= 1024 || (y1 - y0).abs() >= 512 {
                    return Err(invalid("quad exceeds the GPU primitive extent limit"));
                }
                Ok([x0.min(x1), y0.min(y1), x0.max(x1), y0.max(y1)])
            }
        }
    }

    /// Select indexed source color words before framebuffer effects. Quads use
    /// their packet page, affine triangle interpolation and half-open edges.
    pub fn sample(
        &self,
        state: SampleState,
        texture: &[WordRegion<'_>],
        palette: &[WordRegion<'_>],
    ) -> Result<SourceFrame> {
        let [left, top, right, bottom] = self.bounds()?;
        let (tpage, clut) = match *self {
            Self::Rectangle { tpage, clut, .. } | Self::Quad { tpage, clut, .. } => (tpage, clut),
        };
        if tpage & !0x1ff != 0 || (tpage >> 7) & 3 > 1 || clut & 0x8000 != 0 {
            return Err(invalid(
                "source sampling requires retail indexed page and CLUT fields",
            ));
        }
        let texels = Texels {
            state,
            texture,
            palette,
            tpage,
            clut,
        };
        let width = (right - left) as usize;
        let height = (bottom - top) as usize;
        let mut frame = SourceFrame {
            origin_x: left as i16,
            origin_y: top as i16,
            image: WordImage {
                width,
                height,
                words: vec![0; width * height],
            },
        };
        match *self {
            Self::Rectangle {
                origin,
                width,
                height,
                ..
            } => {
                if u32::from(tpage) != state.draw_mode & 0x1ff {
                    return Err(invalid("rectangle page differs from its E1 command"));
                }
                if origin.u & 1 != 0 || origin.v & 1 != 0 || width & 1 != 0 {
                    return Err(invalid("rectangle sampling requires even UV and width"));
                }
                for y in 0..height {
                    for x in 0..width {
                        frame.image.words[usize::from(y) * usize::from(width) + usize::from(x)] =
                            texels.color(origin.u.wrapping_add(x), origin.v.wrapping_add(y))?;
                    }
                }
            }
            Self::Quad { vertices, .. } => {
                for indices in [[0, 1, 2], [1, 2, 3]] {
                    sample_triangle(indices.map(|i| vertices[i]), &texels, &mut frame)?;
                }
            }
        }
        Ok(frame)
    }
}

// Every qualified quad is an axis-aligned XY/UV grid. Its two triangles can
// still select different texels from an ideal image resize: UV edge increments
// truncate in 16.16 before row accumulation. X intersections retain the full
// ratio. Each span starts at its first integer X with a half-texel bias.
fn sample_triangle(
    vertices: [Vertex; 3],
    texels: &Texels<'_>,
    frame: &mut SourceFrame,
) -> Result<()> {
    let points = vertices.map(|p| {
        [
            i64::from(packet_coordinate(p.x)),
            i64::from(packet_coordinate(p.y)),
            i64::from(p.u),
            i64::from(p.v),
        ]
    });
    let [a, b, c] = points;
    let area = (b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1]);
    if area == 0 {
        return Ok(());
    }
    let gradients = [2, 3].map(|uv| {
        (((b[uv] - a[uv]) * (c[1] - a[1]) - (c[uv] - a[uv]) * (b[1] - a[1])) * 65536) / area
    });
    let first_y = points.iter().map(|p| p[1]).min().unwrap();
    let last_y = points.iter().map(|p| p[1]).max().unwrap();
    for y in first_y..last_y {
        let mut edges = Vec::with_capacity(2);
        for [i, j] in [[0, 1], [1, 2], [2, 0]] {
            let (mut start, mut end) = (points[i], points[j]);
            if start[1] > end[1] {
                std::mem::swap(&mut start, &mut end);
            }
            if y < start[1] || y >= end[1] {
                continue;
            }
            let row = y - start[1];
            let height = end[1] - start[1];
            edges.push([
                start[0] * 65536 + (end[0] - start[0]) * 65536 * row / height,
                start[2] * 65536 + row * ((end[2] - start[2]) * 65536 / height),
                start[3] * 65536 + row * ((end[3] - start[3]) * 65536 / height),
            ]);
        }
        if edges.len() != 2 {
            return Err(invalid("quad triangle has an invalid scanline"));
        }
        edges.sort_unstable_by_key(|edge| edge[0]);
        let first_x = (edges[0][0] + 65535) >> 16;
        let last_x = (edges[1][0] + 65535) >> 16;
        for x in first_x..last_x {
            let distance = x * 65536 - edges[0][0];
            let [u, v] = [0, 1].map(|axis| {
                ((edges[0][axis + 1] + ((distance * gradients[axis]) >> 16) + 32768) >> 16) as u8
            });
            let color = texels.color(u, v)?;
            if color != 0 {
                let row = (y - i64::from(frame.origin_y)) as usize;
                let column = (x - i64::from(frame.origin_x)) as usize;
                frame.image.words[row * frame.image.width + column] = color;
            }
        }
    }
    Ok(())
}

/// One source-qualified part in the caller's explicit preview order.
#[derive(Clone, Copy, Debug)]
pub struct SourcePart<'a> {
    pub geometry: &'a Geometry,
    pub state: SampleState,
    pub texture: &'a [WordRegion<'a>],
    pub palette: &'a [WordRegion<'a>],
}

/// A source-color derivative before draw offsets, clipping or framebuffer effects.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SourceFrame {
    pub origin_x: i16,
    pub origin_y: i16,
    pub image: WordImage,
}

/// Assemble sampled parts in explicit back-to-front order. Zero color words
/// leave earlier parts intact. Every other word replaces them, including STP black.
pub fn assemble(parts: &[SourcePart<'_>]) -> Result<SourceFrame> {
    if parts.is_empty() || parts.len() > 255 {
        return Err(invalid("source frame requires 1..=255 parts"));
    }
    let mut bounds = [i32::MAX, i32::MAX, i32::MIN, i32::MIN];
    for part in parts {
        let [left, top, right, bottom] = part.geometry.bounds()?;
        bounds = [
            bounds[0].min(left),
            bounds[1].min(top),
            bounds[2].max(right),
            bounds[3].max(bottom),
        ];
    }
    let [left, top, right, bottom] = bounds;
    let width = (right - left) as usize;
    let height = (bottom - top) as usize;
    let count = width
        .checked_mul(height)
        .filter(|count| *count <= crate::png::MAX_PIXELS)
        .ok_or_else(|| invalid("source frame exceeds the PNG pixel bound"))?;
    let mut image = WordImage {
        width,
        height,
        words: vec![0; count],
    };
    for part in parts {
        let sampled = part
            .geometry
            .sample(part.state, part.texture, part.palette)?;
        let x = (i32::from(sampled.origin_x) - left) as usize;
        let y = (i32::from(sampled.origin_y) - top) as usize;
        for (row, colors) in sampled
            .image
            .words
            .chunks_exact(sampled.image.width)
            .enumerate()
        {
            let start = (y + row) * width + x;
            for (target, color) in image.words[start..start + sampled.image.width]
                .iter_mut()
                .zip(colors)
            {
                if *color != 0 {
                    *target = *color;
                }
            }
        }
    }
    Ok(SourceFrame {
        origin_x: left as i16,
        origin_y: top as i16,
        image,
    })
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
