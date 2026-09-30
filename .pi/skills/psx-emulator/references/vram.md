# VRAM and textures

## Purpose

Use [vram.lua](../scripts/vram.lua) for rectangle/palette export or native uploads.
Follow [Runtime](runtime.md#invocation); declare `support`, `snapshot`.

## Procedure

For `action=export` (default), choose an offline state or live target/EXE.
State plus target resumes execution. Set `x=0..1023` in **16-bit words**,
`y=0..511`, `width` in pixels, `height` in rows. x/y default to zero;
dimensions default to the remaining region.

Choose `bpp=16` (default), `8` or `4`. Indexed formats require `clut_x`
in words and `clut_y`; the full 16/256-entry palette must fit one VRAM row.
For a 4-bit texture, place these arguments on one command:

```text
--argument x=320 --argument y=0 --argument width=64 --argument height=64
--argument bpp=4 --argument clut_x=0 --argument clut_y=480
```

For `action=upload`, follow [BUS](bus.md), declare `bus`, select `scratch`,
`bpp=16` and a bounded rectangle. Stage `--input pixels=PATH` with exactly
width × height × 2 little-endian BGR555 bytes. Capture the original first if needed.
The action clears pending GPU FIFO via GP1, sends GP0 CPU-to-VRAM words through
guest stores, then exports the result.

Upload inherits the probe's RAM/capacity bounds; split oversized rectangles into
explicit missions. Indexed uploads/implicit palette changes reject.
`savestate=1` is valid for uploads/live exports, never offline export.

## Application

Verify `vram.json`, `vram.bin`, `vram.ppm`, `mask.bin` and receipt.
Binary rows are consecutive packed rectangle bytes without full-VRAM padding;
indexed rows round up to whole bytes. PPM is P6 RGB expanded from BGR555.
Mask output is colour bit 15 as one byte/pixel; zero texels/masks are not composited
as transparency. JSON retains source, geometry, palette, row size and GPU status.

These are VRAM contents, not displayed geometry, alpha blending or command history.
Native GPU mask settings govern upload writes; compare actual output rather than
assuming equality. Retain scratch/cycle reports and failed evidence.
