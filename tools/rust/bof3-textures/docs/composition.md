# Unscaled sprite geometry

The `composition` library computes the geometry written by the original US
consumers at `SLUS_004.22@0x8014c6f8` and `0x8014cf9c`. It accepts an existing
`cell::Cell` and explicit draw context. It returns rectangles or textured quad
vertices in the original part order.

Construct `DrawContext` with `DrawContext::from_entity(entity, graph_type, clut)`.
Supply at least `0x60` bytes from the entity referenced by the consumer through
scratchpad address `0x1f800044`. Supply the original graph-type value and the
resolved CLUT helper result. A CLUT result is a packet field, not a palette index.
The function rejects short entities, graph types 1 and 2, and entities with a
nonzero scaling flag at byte `0x48`.

Call `composition::unscaled(&cell, context)` to obtain the geometry. It preserves
the packet's 16-bit XY and 8-bit UV wrapping. Coordinates are stored bits, not
clipped screen positions. The result includes the texture-page and CLUT fields.
For a rectangle, the texture page describes its preceding draw-mode selection.

The part's horizontal flag and the entity's flip flag choose the original path.
Both clear or both set select rectangles with different X placement. Exactly one
set selects a quad. The quad path changes UV endpoints and can adjust the texture
page when entity byte `0x28` is nonzero. Reversing an image alone does not reproduce
these rules. Size codes use the original 16-entry table at `0x8017f53c`.

The caller must establish that the entity, cell, graph type and CLUT belong to
the same consumer observation. The library does not choose a source image,
identify a resident bank, resolve palette uploads, apply GPU drawing state or
rasterize vertices. An exact source-bank match in RAM establishes byte identity
only. Assembled previews and runtime appearance still require those bindings.
