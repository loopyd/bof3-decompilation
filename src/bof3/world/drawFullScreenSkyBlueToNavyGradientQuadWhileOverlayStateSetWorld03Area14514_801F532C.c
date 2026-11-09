#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* @source 0x801F532C
 * @behavior Full-screen gradient curtain of this overlay. While the shared
 * overlay state byte at 0x8014933B reads zero the body returns without touching
 * the GPU. Otherwise it chooses the draw-mode texture page from the graph type:
 * an answer of 1 selects 0x225, a repeated probe answering 2 keeps 0x225 and
 * every other answer selects 0x95. SetDrawMode is armed with the shared
 * primitive cursor word at 0x8014598C as the DR_MODE pointer, 0 as dfe, 1 as
 * dtd, that texture page and a null fifth argument passed on the stack, and
 * func_8014E5A0 appends the 0x0C-byte draw mode packet to ordering table slot 7.
 * The 0x24-byte POLY_G4 reloaded from the same cursor is stamped by SetPolyG4
 * with its semi-transparency bit cleared through SetSemiTrans with zero, then
 * the quad is written across the whole screen: x 0 on both left vertices and
 * 0x140 on both right ones, y 0 on the top pair and 0xF0 on the bottom pair, the
 * top pair coloured r 0, g 0xC8 and b 0xFF and the bottom pair r 0, g 0 and
 * b 0x20. func_8014E5A0 appends the finished quad to ordering table slot 7 as a
 * 0x24-byte packet. It takes no arguments and returns nothing. Its 0x20-byte
 * frame holds the outgoing fifth argument of SetDrawMode at +0x10 and keeps $ra
 * with $s0 across the four calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: the draw-mode arming, the SetPolyG4 and SetSemiTrans pair, the
 * coordinate-then-colour store grouping and the two func_8014E5A0 appends follow
 * the exact same-family sibling
 * drawBlueToWhiteGradientBandFromScreenTopSizedByOverlayStateWorld03Area12114_801F6390
 * (0x801F6390), whose graph-type block issues the same double probe with the same
 * 0x225 and 0x95 texture pages in the same beq and bne delay slots. This span
 * owns its gate (the single zero test on 0x8014933B), the full-screen geometry
 * 0x140 and 0xF0 and the three colour constants, all read from its own bytes.
 */
void drawFullScreenSkyBlueToNavyGradientQuadWhileOverlayStateSetWorld03Area14514_801F532C(
    void) {
  POLY_G4* prim;
  s32 tpage;

  if (D_8014933B == 0u) {
    return;
  }
  if (GetGraphType() == 1) {
    tpage = 0x225;
  } else if (GetGraphType() == 2) {
    tpage = 0x225;
  } else {
    tpage = 0x95;
  }
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(7, 0xC);

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 0);
  prim->x0 = 0;
  prim->y0 = 0;
  prim->x1 = 0x140;
  prim->y1 = 0;
  prim->x2 = 0;
  prim->y2 = 0xF0;
  prim->x3 = 0x140;
  prim->y3 = 0xF0;
  prim->r0 = prim->r1 = 0;
  prim->g0 = prim->g1 = 0xC8;
  prim->b0 = prim->b1 = 0xFF;
  prim->r2 = prim->r3 = 0;
  prim->g2 = prim->g3 = 0;
  prim->b2 = prim->b3 = 0x20;
  func_8014E5A0(7, 0x24);
}
