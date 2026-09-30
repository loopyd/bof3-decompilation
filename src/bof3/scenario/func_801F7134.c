#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7134
 * @behavior Seeds the shared primitive cursor with one semi-transparent
 * texture sprite: the drawing-mode tpage comes from the LIBGPU graph type
 * (0x32D for graph types 1 and 2, 0xDD otherwise), SetDrawMode is issued on
 * the cursor with zero dfe/dtd and a null tw, a 0x0C-byte primitive is
 * appended, SetSprt lays the cursor out as an SPRT with x0/y0 = 32, u0 = 0,
 * v0 = 32, w = 256, h = 1768 and clut = 0x7A80, its r0/g0/b0 come from bytes
 * 0x5D/0x5E/0x5F of the scratchpad work object published at 0x1F800044, the
 * shade texture is cleared, SetSemiTrans receives the low byte of the
 * argument, and a 0x14-byte primitive is appended.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7134(s32 semi_transparent) {
  SPRT* sprt;
  s32 page;

  if (GetGraphType() == 1) {
    page = 0x32D;
  } else if (GetGraphType() == 2) {
    page = 0x32D;
  } else {
    page = 0xDD;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, page, 0);
  func_8014E5A0(D_1F800044[0x29], 0xC);

  sprt = (SPRT*)g_PrimCursor;
  SetSprt(sprt);
  sprt->x0 = 32;
  sprt->y0 = 32;
  sprt->u0 = 0;
  sprt->v0 = 32;
  sprt->w = 256;
  sprt->h = 1768;
  sprt->clut = 0x7A80;
  sprt->r0 = D_1F800044[0x5D];
  sprt->g0 = D_1F800044[0x5E];
  sprt->b0 = D_1F800044[0x5F];
  SetShadeTex(sprt, 0);
  SetSemiTrans(sprt, semi_transparent & 0xFF);
  func_8014E5A0(D_1F800044[0x29], 0x14);
}
