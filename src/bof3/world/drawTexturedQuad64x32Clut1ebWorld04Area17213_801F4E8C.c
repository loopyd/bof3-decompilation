#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x801F4E8C
 * @behavior Fills the POLY_FT4 that sits at the shared primitive cursor
 *           g_PrimCursor with the 0x40 by 0x20 quad whose top-left corner is
 *           (arg0, arg1), sampling the same 0x40 by 0x20 texel window from the
 *           top-left of its texture page. The CLUT comes from GetClut(0, 0x1EB)
 *           and the 16-bit semi-transparent texture page from
 *           GetTPage(1, 1, 0x300, 0x100); the primitive is tinted 0x80 grey in
 *           all three colour channels and then queued at ordering-table index 1
 *           with primitive size 0x28 through func_8014E5A0. Takes the quad
 *           position and returns nothing; both in-image callers pass the fixed
 *           pair (0xDC, 0xC8).
 * @status partial
 * @match 94.74
 * @residual 54/57 instructions, 228 -> 228 bytes. The halfword coordinate
 *           snapshots retain both copies, but the X snapshot uses $v0 and is
 *           stored before the Y snapshot and offset additions. The original
 *           keeps X in $v1 and copies both coordinates before those additions.
 *           The frame, calls and remaining primitive fields agree.
 */
void drawTexturedQuad64x32Clut1ebWorld04Area17213_801F4E8C(s32 arg0, s32 arg1) {
  POLY_FT4* prim;
  s16 left;
  s16 top;

  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);
  left = arg0;
  top = arg1;
  prim->x0 = left;
  prim->x2 = left;
  prim->y0 = top;
  prim->y1 = top;
  prim->x1 = arg0 + 0x40;
  prim->y2 = arg1 + 0x20;
  prim->x3 = arg0 + 0x40;
  prim->y3 = arg1 + 0x20;
  setUVWH(prim, 0, 0, 0x40, 0x20);
  prim->clut = GetClut(0, 0x1EB);
  prim->tpage = GetTPage(1, 1, 0x300, 0x100);
  prim->r0 = 0x80;
  prim->g0 = 0x80;
  prim->b0 = 0x80;
  GpuAppendPrim(1, 0x28);
}
