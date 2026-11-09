#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x801EF76C
 * @behavior Draws one eight-pixel textured cell per byte of the NUL-terminated
 *   glyph-code string the first argument points at, along the pen position
 *   (x, y), advancing x by eight pixels per code. It first selects the draw-mode
 *   word from GetGraphType, 0x25 for graph types 1 and 2 and 0x15 for every
 *   other type, which is the arm order and the call sequence the exact sibling
 *   dispatchWorkByte1HandlerThenUpdateAndDrawWhileWorkStepRunningBmagicMagic09403
 *   _801EEE4C records for the same source shape. It then opens the primitive
 *   with SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0) and reserves 0xC
 *   bytes with GpuAppendPrim(2, 0xC). The body returns as soon as the first code
 *   is zero, so the loop runs once per non-zero code.
 *   Each code stamps a POLY_FT4 taken from g_PrimCursor with SetPolyFT4 at the
 *   rectangle (x, y) to (x + 8, y + 8) and fills all three colour channels with
 *   the literal grey 0x80. The texture page is GetTPage(0, 1, 0x3C0, 0) and the
 *   clut GetClut(0, 0x1E0).
 *   The four uv corners take the code as their only source, and every corner
 *   re-reads that byte because the primitive stores may alias the string: u0 and
 *   u2 are (code - 0x40) * 8, u1 and u3 are code * 8 - 0x1F8, v0 and v1 are 8
 *   and v2 and v3 are 0x10, so one code selects an eight-texel column of the
 *   first glyph bank, in its row at texel y 8 to 0x10.
 *   A code of 0xFF stamps the cell but skips GpuAppendPrim(2, 0x28), which keeps
 *   the pen advance for that cell without queueing a primitive. The loop ends at
 *   the next zero code. Nothing is returned and no state outside the primitive
 *   cursor's own storage is written.
 *   The 0x40-byte frame keeps ra at 0x3C, fp at 0x38, s7 down to s0 at 0x34 down
 *   to 0x18; the three literals the body repeats, 0x80, 8 and 0x10, are hoisted
 *   out of the loop into s6, fp and s7 ahead of it, and the caller-side
 *   y + 8 corner is hoisted into s4 the same way.
 *   Matching note: the u1 and u3 texel offsets go through one named s16 local.
 *   Storing the truncated expression straight into the byte field lets the
 *   compiler fold the -0x1F8 constant modulo 0x100 into an addiu of 8, while the
 *   halfword local keeps the negative immediate, which is the addiu of -0x1F8
 *   the original holds in its own two corners. The four corners still reload the
 *   code byte, because the primitive stores may alias the string.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C(
    u8 *text, s32 x, s32 y) {
  POLY_FT4 *prim;
  s32 mode;
  s32 x1;
  s32 y1;
  s16 u;

  if (GetGraphType() == 1) {
    mode = 0x25;
  } else if (GetGraphType() == 2) {
    mode = 0x25;
  } else {
    mode = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
  GpuAppendPrim(2, 0xC);
  y1 = y + 8;
  if (*text == 0) {
    return;
  }
  do {
    prim = (POLY_FT4 *)g_PrimCursor;
    SetPolyFT4(prim);
    x1 = x + 8;
    prim->x0 = x;
    prim->y0 = y;
    prim->x1 = x1;
    prim->y1 = y;
    prim->x2 = x;
    prim->y2 = y1;
    prim->x3 = x1;
    prim->y3 = y1;
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    prim->tpage = GetTPage(0, 1, 0x3C0, 0);
    prim->clut = GetClut(0, 0x1E0);
    prim->u0 = (u8)((*text - 0x40) << 3);
    prim->v0 = 8;
    u = (*text << 3) - 0x1F8;
    prim->u1 = (u8)u;
    prim->v1 = 8;
    prim->u2 = (u8)((*text - 0x40) << 3);
    prim->v2 = 0x10;
    u = (*text << 3) - 0x1F8;
    prim->u3 = (u8)u;
    prim->v3 = 0x10;
    if (*text != 0xFF) {
      GpuAppendPrim(2, 0x28);
    }
    text++;
    x = x1;
  } while (*text != 0);
}
