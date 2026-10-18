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
same consumer observation. The library does not choose a source image, identify
a resident bank, resolve palette uploads, apply GPU drawing state or rasterize
vertices. An exact source-bank match in RAM establishes byte identity only.
Assembled previews and runtime appearance still require those bindings.
