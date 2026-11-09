#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x801EF924
 * @behavior Draws up to eight glyph cells of this overlay's ASCII glyph bank at
 *   the pen position (x, y), advancing x by eight pixels per drawn code and
 *   stopping at the first zero code. The bank is addressed by the code byte
 *   less 0x30, so 0x30 is its first cell: the cell column is
 *   (code - 0x30) mod 31 and the cell row is (code - 0x30) / 31, a 31-column
 *   grid of 8-by-12 texel cells whose first row starts at texel y 0x48.
 *   The draw-mode word comes from GetGraphType, 0x25 for graph types 1 and 2 and
 *   0x15 for every other type, which is the arm order and the call sequence the
 *   exact sibling
 *   drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C
 *   records for the same source shape. The primitive is opened with
 *   SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0) and 0xC bytes are
 *   reserved with GpuAppendPrim(2, 0xC) before the string is tested, so an empty
 *   string still opens the draw mode and reserves that word.
 *   Each code stamps a POLY_FT4 taken from g_PrimCursor with SetPolyFT4 over the
 *   rectangle (x, y) to (x + 8, y + 0xC), fills the three colour channels with
 *   the literal grey 0x80, takes the texture page GetTPage(0, 1, 0x3C0, 0) and
 *   the clut GetClut(0, 0x1E0). The four uv corners take the code as their only
 *   source: u0 and u2 are ((code - 0x30) mod 31) * 8, u1 and u3 add 8 to that,
 *   v0 and v1 are ((code - 0x30) / 31) * 12 + 0x48 and v2 and v3 add 0xC, so one
 *   code selects one 8-by-12 cell of the bank. The byte is read once, before the
 *   division, and the quotient and the remainder are both kept in that one
 *   signed local. A code of 0xFF stamps its cell but skips
 *   GpuAppendPrim(2, 0x28), which keeps the pen advance for that cell without
 *   queueing a primitive.
 *   The run is bounded: the eighth cell ends it even when the string continues,
 *   and the per-iteration counter is compared before each cell. The loop is a
 *   while on both tests in the order (string first, counter second): the string
 *   test reloads the byte after every primitive store, because those stores may
 *   alias it, while the counter is a loop-carried local, so GCC keeps the string
 *   test at the back edge and the counter test at the head.
 *   Matching note: both uv locals are halfword. A byte field takes only the low
 *   byte of the value, so a halfword local costs no instruction at either store,
 *   and it also keeps the second texel row as an increment of the first row
 *   value instead of folding 0x48 and 0xC into one 0x54 offset from the row
 *   base. Two halfword locals measured 119 of 119 instructions; the same body
 *   with signed word locals folded the increment away and measured 114 of 119.
 *   The 0x40-byte frame keeps ra at 0x38, s7 down to s0 at 0x34 down to 0x18,
 *   and the repeated 0x80 colour literal sits in s7 across the loop. The caller
 *   func_801EF2DC passes a name field of the 0x118-byte local panel record
 *   table at 0x801EB6A4 with the pen position (0x70, 0x30). Nothing is returned
 *   and no state outside the primitive cursor's own storage is written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawUpToEightAsciiGlyphBankCellsAtPenPositionBmagicMagic06000_801EF924(
    u8 *text, s32 x, s32 y) {
  POLY_FT4 *prim;
  s32 mode;
  s32 x1;
  s32 y1;
  s32 i;
  s32 code;
  s16 u;
  s16 v;

  if (GetGraphType() == 1) {
    mode = 0x25;
  } else if (GetGraphType() == 2) {
    mode = 0x25;
  } else {
    mode = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
  GpuAppendPrim(2, 0xC);
  if (*text == 0) {
    return;
  }
  i = 0;
  y1 = y + 0xC;
  while (*text != 0 && i < 8) {
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
    code = *text - 0x30;
    u = (code % 31) * 8;
    v = (code / 31) * 12 + 0x48;
    prim->u0 = (u8)u;
    prim->v0 = (u8)v;
    prim->v1 = (u8)v;
    prim->u1 = (u8)(u + 8);
    prim->u2 = (u8)u;
    prim->v2 = (u8)(v + 0xC);
    prim->u3 = (u8)(u + 8);
    prim->v3 = (u8)(v + 0xC);
    if (*text != 0xFF) {
      GpuAppendPrim(2, 0x28);
    }
    i++;
    text++;
    x = x1;
  }
}
