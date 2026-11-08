#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

extern u8 D_8014832E;   /* @source 0x8014832E @kind data */
extern u8 D_8014933B;   /* @source 0x8014933B @kind data */
extern u8 D_801F68B8[]; /* @source 0x801F68B8 @kind data */

/* @source 0x801F6390
 * @behavior Draws the overlay band of this target: one full width POLY_G4
 * anchored on the screen top, its height taken from the per-overlay-state byte
 * table 0x801F68B8 and its colour running from a blue violet top pair to a
 * white bottom pair. The body is gated twice before anything is written: the
 * shared overlay state byte at 0x8014933B must read 1 or 2, and the effect bank
 * byte at 0x8014832E must carry bit 0x04. Otherwise the function returns
 * without touching the GPU.
 * The texture page handed to SetDrawMode is chosen from the graph type.
 * GetGraphType is asked once and an answer of 1 selects 0x225; when that first
 * answer is not 1 the probe is repeated and an answer of 2 keeps 0x225 while any
 * other answer selects 0x95. SetDrawMode is then armed with the primitive
 * cursor word at 0x8014598C as the DR_MODE pointer, 0 as dfe, 1 as dtd, that
 * page, and a fifth argument of zero passed on the stack; func_8014E5A0 appends
 * the 0x0C-byte draw mode packet to ordering table slot 7.
 * The 0x24-byte POLY_G4 published at the cursor is stamped with SetPolyG4 and
 * has its semi-transparency bit cleared through SetSemiTrans with zero, then
 * the band is written: both left x stay 0 and both right x stay 0x140, the top
 * pair of vertices sits on the screen top at y 0 and the bottom pair sits at the
 * height the table 0x801F68B8 holds for the overlay state, read at that index
 * once for each bottom vertex. The index byte is reloaded after both appended
 * packets because the two calls may rewrite it, so it is not kept in a saved
 * register. Vertex colours are r 0x48, g 0x38 and b 0xFF on the top pair
 * and 0xFF on all three channels of the bottom pair, written as chained byte
 * assignments: red, green and blue for the top pair, then blue, green and red
 * for the bottom pair. func_8014E5A0 appends the finished quad to ordering table
 * slot 7 as a 0x24-byte packet.
 * It takes no arguments and returns nothing. Its 0x20-byte frame holds the
 * outgoing fifth argument of SetDrawMode at +0x10 and keeps $ra with $s0 across
 * the four calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: the draw-mode arming, the SetPolyG4 and SetSemiTrans pair, the
 * coordinate-then-colour store grouping and the two func_8014E5A0 appends are
 * the same statement order as the exact single-quad sibling
 * drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8
 * (0x801D19A8) of the scenario overlay, and the graph-type-conditional texture
 * page matches the kept double probe of the exact same-target sibling
 * emitMeterBarWithPaletteWorld03Area12114_801F595C (0x801F595C). The own bytes
 * fix the geometry literals 0x140 and the colour literals, and fix 0x225, 0x95
 * and the table 0x801F68B8 as this band's own texture pages and height source.
 * The measured texture page local is a plain int, and the four guarded stores
 * are ordered x0, y0, x1, y1, x2, y2, x3, y3: that pair of positions is where
 * the original issued the height load, so the store order there is not free.
 */
void drawBlueToWhiteGradientBandFromScreenTopSizedByOverlayStateWorld03Area12114_801F6390(
    void) {
  POLY_G4* prim;
  u32 state;
  s32 tpage;

  if (D_8014933B == 0u || D_8014933B > 2u || (D_8014832E & 4u) == 0u) {
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
  state = D_8014933B;
  prim->x0 = 0;
  prim->y0 = 0;
  prim->x1 = 0x140;
  prim->y1 = 0;
  prim->x2 = 0;
  prim->y2 = D_801F68B8[state];
  prim->x3 = 0x140;
  prim->y3 = D_801F68B8[state];
  prim->r0 = prim->r1 = 0x48;
  prim->g0 = prim->g1 = 0x38;
  prim->b0 = prim->b1 = 0xFF;
  prim->b2 = prim->b3 = 0xFF;
  prim->g2 = prim->g3 = 0xFF;
  prim->r2 = prim->r3 = 0xFF;
  func_8014E5A0(7, 0x24);
}
