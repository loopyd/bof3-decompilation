# Sprite packet geometry

The `composition` library computes the geometry written by the original US
consumers at `SLUS_004.22@0x8014c6f8` and `0x8014cf9c`. It accepts an existing
`cell::Cell` and explicit draw context. It returns rectangles or textured quad
vertices in the original part order.

Construct `DrawContext` with `DrawContext::from_entity(entity, graph_type)`.
Supply at least `0x60` bytes from the entity referenced by the consumer through
scratchpad address `0x1f800044`, and the original graph-type value. The function
rejects short entities and graph types 1 and 2. It derives the original encoded
CLUT field from entity bytes `0x24`, `0x27` and `0x28`. This preserves raw field
bits, including values outside a physical palette row range. It does not select
a source palette or normalize those values.

Call `composition::compose(&cell, context)` to obtain the geometry. It preserves
the packet's 16-bit XY and 8-bit UV wrapping. Coordinates are stored bits, not
clipped screen positions. The result includes the texture-page and CLUT fields.
For a rectangle, the texture page describes its preceding draw-mode selection.
Size codes use the original 16-entry table at `0x8017f53c`.

Without scaling, both part and entity flip flags clear or both set select
rectangles with different X placement. Exactly one set selects a quad. A nonzero
entity byte `0x48` selects scaled quads. The signed factors at bytes `0x40` and
`0x44` control X and Y independently. The original multiply retains its low
32 bits and rounds according to the factor's sign. Negative factors and product
overflow preserve that original behavior.

The quad path changes UV endpoints and can adjust the texture page when entity
byte `0x28` is nonzero. Scaled UV endpoints also change when a factor lies outside
`-122880..=122880`. Reversing or resizing an image alone does not reproduce these
rules. Zero-area or wrapped coordinates remain encoded geometry; the library
does not claim that they rasterize a visible primitive.

The caller must establish that the entity, cell and graph type belong to the
same consumer observation. Geometry alone does not choose an image, identify a
resident bank or resolve palette uploads. An exact source-bank match in RAM
establishes byte identity only.

## Rectangle source texels

`Geometry::sample_rectangle` selects exact palette color words for one indexed
rectangle. Supply `RectangleState::from_commands(draw_mode, texture_window)`
with explicit GP0(E1h) and GP0(E2h) commands. A missing texture-window observation
cannot be replaced with an assumed zero command.

Each source is a `WordRegion::new(x_words, y, &image)`. The image is a
`png::WordImage` whose width counts VRAM words. Regions must fit within 1024 words
by 512 rows. Supply the decoded texture and palette at their qualified upload
coordinates. Region construction checks bounds, not physical identity, upload
execution or residency. Every selected texture and palette word must exist in
the supplied regions. A partial palette is sufficient only when it contains
every selected color word. Missing colors cause an error.

The sampler supports 4-bit and 8-bit indices. It checks the rectangle's page
against E1, wraps UV bytes, applies E2 masks and offsets, and resolves the encoded
CLUT coordinates. Color words retain transparent zero and STP bits. The result
can pass directly to `png::encode_words`. It remains a source derivative and
does not replace editable image, palette or cell records.

The sampler refuses quads, empty rectangles, odd UV coordinates or width,
extended VRAM, hardware rectangle flips, direct color and mismatched commands.
Its hardware rules follow the [PSX GPU specification](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/).
It does not clip, shade, blend, apply framebuffer masks or determine layer order.
Per-part source selection does not establish assembled-frame or runtime accuracy.

## Rectangle source frames

`composition::assemble_rectangles(&parts)` places sampled rectangles on one
source-color canvas. Each `RectanglePart` supplies its `geometry`, `state`,
`texture` and `palette`. Supply parts in explicit back-to-front order. The
function does not derive submission order from cell records or ordering tables.

The returned `SourceFrame` contains the top-left `origin_x` and `origin_y` and a
`png::WordImage`. Placement uses the signed low 11 bits of each packet XY field,
before drawing offsets or clipping. Canvas bounds include every rectangle's
declared extent, even transparent edges. The canvas starts with zero words.
A later zero color word leaves an earlier part intact. Every nonzero word
replaces the earlier word, including STP black `0x8000`.

Frames require 1 through 255 parts and at most `png::MAX_PIXELS` canvas pixels.
Every part must satisfy `sample_rectangle` and its source bounds. Quads and empty
rectangles fail. The output can pass to `png::encode_words` as a derivative;
editable cells, image indices and palettes remain separate.

This source preview does not apply color modulation, semitransparency, draw
offsets, clipping, framebuffer masks or other scene objects. Its part order and
source bindings require caller evidence. Matching the preview to captured
commands does not establish rendered appearance or edited-disc loading.

## Qualified source associations

The catalog retains one conditional association for the original US
`BIN/WORLD00/AREA024.EMI#4` cell at payload offset `0x0c51`. Query its logical
record with `--records` and use `--palette` or `--dependency` to follow its known
sources. Entry 5 and entry 6 list that cell in their `uses` fields.

This observation uses two quads with 8-bit texture page `0x0089`, CLUT `0x7c02`
and a preceding zero texture-window command. Entry 5 chunks 6 and 22 match the
VRAM backing of the inclusive UV envelope `U=128..176, V=16..32` at both capture
boundaries. The envelope contains indices `0..31`, selecting entry 6 palette
words `32..63`. Index 17 selects source word 49.

The edges have status `conditional`. The cell retains its unresolved general
palette and resident dependencies. The association does not establish other draw
contexts, which UVs rasterization samples, whether the `V=32` endpoint is sampled,
or unchanged VRAM between capture boundaries. It does not assign a character
identity or make a flattened preview editable.
