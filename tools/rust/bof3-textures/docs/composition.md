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

## Indexed source texels

`Geometry::sample` selects palette color words for one indexed rectangle or
qualified quad. Supply `SampleState::from_commands(draw_mode, texture_window)`
with explicit GP0(E1h) and GP0(E2h) commands. A missing texture-window observation
cannot be replaced with an assumed zero command.

Supply texture and palette slices of `WordRegion::new(x_words, y, &image)`. The image is a
`png::WordImage` whose width counts VRAM words. Regions must fit within 1024 words
by 512 rows. Supply the decoded texture and palette at their qualified upload
coordinates. Region construction checks bounds, not physical identity, upload
execution or residency. Every selected texture and palette word must exist in
the supplied regions. A partial palette is sufficient only when it contains
every selected color word. Missing words cause an error. Separate regions retain
holes between uploads. If regions overlap at a selected word, their values must
agree; the sampler does not infer transfer order.

The sampler supports 4-bit and 8-bit indices. A rectangle's page must match E1.
A quad selects its own packet page, which can differ from E1. Both paths wrap
sampled UV bytes, apply E2 masks and offsets, and resolve the encoded CLUT.
Color words retain transparent zero and STP bits. The returned `SourceFrame`
contains its signed origin and a `png::WordImage` suitable for `png::encode_words`.
It remains a source derivative and does not replace editable records.

Quads require the axis-aligned XY and UV grid emitted by `compose`. The sampler
splits them into triangles in packet order, excludes bottom and right edges,
and handles reversed coordinates and UV endpoints. UV edge increments truncate
in 16.16 before row accumulation. Each span includes the first integer pixel's
edge correction and a half-texel sampling bias. Ideal image resizing can select
different texels. The model follows the [PSX GPU specification](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/)
and retained [hardware interpolation probes](../../../../third_party/pcsx-redux-agentic/src/mips/tests/gpu-raster-phase20/raster-expected-phase20.h).

The sampler refuses empty rectangles, odd rectangle UV coordinates or width,
zero-area quads, quads spanning at least 1024 X pixels or 512 Y pixels,
other quad layouts, extended VRAM, hardware rectangle flips and direct color.
It does not clip, shade, blend, apply framebuffer masks or determine layer order.
Per-part source selection does not establish assembled-frame or runtime accuracy.

## Source frames

`composition::assemble(&parts)` places sampled parts on one source-color canvas.
Each `SourcePart` supplies its `geometry`, `state`,
`texture` and `palette`. Supply parts in explicit back-to-front order. The
function does not derive submission order from cell records or ordering tables.

The returned `SourceFrame` contains the top-left `origin_x` and `origin_y` and a
`png::WordImage`. Placement uses the signed low 11 bits of each packet XY field,
before drawing offsets or clipping. Canvas bounds include every rectangle's
declared extent, even transparent edges. A quad's bounds exclude its maximum
X and Y coordinates. The canvas starts with zero words.
A later zero color word leaves an earlier part intact. Every nonzero word
replaces the earlier word, including STP black `0x8000`.

Frames require 1 through 255 parts and at most `png::MAX_PIXELS` canvas pixels.
Every part must satisfy `sample` and its source bounds. The output can pass to
`png::encode_words` as a derivative;
editable cells, image indices and palettes remain separate.

This source preview does not apply color modulation, semitransparency, draw
offsets, clipping, framebuffer masks or other scene objects. Its part order and
source bindings require caller evidence. Matching the preview to captured
commands does not establish rendered appearance or edited-disc loading.

## Frame command

The `frame` command reconstructs the selected archive from a current edit package,
then samples one declared cell from a qualified three-root or resident bank. It uses
the same edit validation and allocation limits as `pack`. The package contains
its original snapshots, so the command does not reopen former source paths.

```sh
bin/harness textures frame EDIT_PACKAGE --asset RESOURCE_ID --context CONTEXT.json --output NEW_DIRECTORY
```

Choose the bank's physical resource ID from the package's `index.resources`.
For a three-root bank, its `sprite_bank` field identifies support. The context is
a JSON object with all the following fields. Unknown fields are refused.

| Field | Value |
| --- | --- |
| `schema` | `bof3.sprite-frame-context/v1` |
| `root`, `image`, `palette` | Zero-based indices in the selected bank's `sprite.json` |
| `cell_offset` | A declared cell's decoded-bank byte offset in that root |
| `entity` | An array of 96 through 152 unsigned bytes from the selected draw context |
| `graph_type` | The consumer's unsigned-byte graph type |
| `texture_origin` | `[x_words, y]` for the selected decoded image in VRAM |
| `palette_origin` | `[x_words, y]` for the selected 32-word palette in VRAM |
| `draw_mode`, `texture_window` | Unsigned 32-bit GP0(E1h) and GP0(E2h) command words |
| `part_order` | Every cell part index exactly once, in back-to-front order |

JSON numbers use decimal notation. Context files are bounded at 64 KiB. Every
selected texel and color must lie in the supplied image and palette regions.
The command refuses unsupported draw states and geometry, invalid record indices,
missing colors and every other sampler failure described above.
It does not derive runtime selectors or replace missing draw-state evidence.

For a resident bank, choose an asset with `resident_animation` metadata and use
schema `bof3.resident-frame-context/v1`. Keep `cell_offset`, `entity`, `graph_type`,
`draw_mode`, `texture_window` and `part_order` as above. Replace the root, image,
palette and origin fields with `texture_resource` and `palette_resource`, the
exact physical resource IDs from the same package. These resources may belong to
different archives. The selected cell must be declared in the resident bank.

The texture must have a qualified raw upload descriptor and the palette a
qualified staging descriptor. Texture chunks use their declared 32-word by
32-row positions. Skipped chunks and final-row holes provide no words. Palette
staging advances across 256-word rows. Coordinates come from the descriptors;
the context cannot override them. All sampled texels and palette words must
exist in those sources. Source selection remains the caller's assertion,
including when an edited cell samples outside an earlier observed UV envelope.

Create the output parent first and choose a new directory outside the package.
The command publishes `frame.png` and `frame.json` together. The report retains
schema `bof3.sprite-source-frame/v2`, the complete context, and original and
reconstructed hashes for each resource and unique archive. Resource entries
identify their role, source, EMI entry and byte extent. The decoded-bank hash is
present for three-root banks and null for resident sources. The report also
retains the source-color word hash, PNG hash, canvas origin and dimensions.
Its source bindings are caller assertions. The report does not certify upload
execution, runtime residency, scene rendering or edited-disc loading.

To change a frame, edit the package's supported image, palette or cell views and
run `frame` with a new output directory. Frame outputs are separate derivatives,
not package members or editable inverse representations. Regenerate old packages
with the current executable when their interpretation hash is stale.

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
