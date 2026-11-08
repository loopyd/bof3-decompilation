#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801D19A8
 * @behavior Draws the full-screen red gradient band of this overlay. It first
 * arms the draw mode the same way the work-table step of this target does:
 * GetTPage(0, 1, 0x380, 0x100) yields the texture page, that page is handed to
 * SetDrawMode together with the primitive published at g_PrimCursor, and
 * func_8014E5A0 queues the 0x0C-byte DR_MODE packet. It then stamps the
 * 0x24-byte POLY_G4 at the primitive cursor cell 0x8014598C with SetPolyG4,
 * clears its semi-transparency bit through SetSemiTrans with zero and fills the
 * top quad of the band: both left and right x stay at the screen edges 0 and
 * 0x140, the top pair of vertices sits on the screen top at y 0 and the bottom
 * pair on the horizontal midline at y 0x78, with the top pair black and the
 * bottom pair carrying the argument in red and green and blue cleared, so the
 * half fades from black at the top edge to the shade on the midline.
 * func_8014E5A0 then appends that quad under OT slot 7 as a 0x24-byte packet,
 * the cursor cell is reloaded and the mirrored bottom quad is written: its top
 * pair sits on the midline at y 0x78 and carries the shade in red, its bottom
 * pair sits on the screen bottom at y 0xF0 over black, and it is appended the
 * same way. The argument is an unsigned shade byte: the value is kept in a
 * saved register and stored with byte stores into the red bytes at +0x14 and
 * +0x1C of the first quad and +0x04 and +0x0C of the second, with green and
 * blue cleared, and both callers pass a shade that only the low byte carries.
 * It takes that one shade argument, returns nothing and its 0x30-byte frame
 * holds the outgoing fifth argument of SetDrawMode at +0x10 and keeps $ra with
 * $s0-$s3 across the five calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: the two quads, their primitive helper sequence and the
 * coordinate-before-colour store order are the same body as the exact sibling
 * drawRedGradientBandCenteredOnWorkEntryHalfwordWithFrameParityHalfHeightScenarioSce15ef200_801D1414
 * (0x801D1414) of this target, whose band is centred on a work entry instead of
 * on the screen midline and whose frame carries an extra local area. The draw
 * mode arming is the exact prologue of the work-table step
 * stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624
 * (0x801D1624), so the texture page is passed as the same u16 local and the
 * fifth SetDrawMode argument is the same literal zero. The own bytes fix the
 * geometry literals: 0x140 and 0x78 are materialised once in $s1 and $s0 and
 * reused by both quads, while 0xF0 is the only literal the second quad adds.
 */
void drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8(
    u8 shade) {
  POLY_G4* prim;
  u16 tpage;

  tpage = GetTPage(0, 1, 0x380, 0x100);
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
  prim->y2 = 0x78;
  prim->x3 = 0x140;
  prim->y3 = 0x78;
  prim->r0 = 0;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r1 = 0;
  prim->g1 = 0;
  prim->b1 = 0;
  prim->r2 = shade;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = shade;
  prim->g3 = 0;
  prim->b3 = 0;
  func_8014E5A0(7, 0x24);

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 0);
  prim->x0 = 0;
  prim->y0 = 0x78;
  prim->x1 = 0x140;
  prim->y1 = 0x78;
  prim->x2 = 0;
  prim->y2 = 0xF0;
  prim->x3 = 0x140;
  prim->y3 = 0xF0;
  prim->r0 = shade;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r1 = shade;
  prim->g1 = 0;
  prim->b1 = 0;
  prim->r2 = 0;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0;
  func_8014E5A0(7, 0x24);
}
