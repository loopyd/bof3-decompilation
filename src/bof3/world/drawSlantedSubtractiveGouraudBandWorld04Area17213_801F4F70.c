#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x801F4F70
 * @behavior Arms the shared draw mode for the subtract-blended texture page
 *           GetTPage(0, 2, 0x3C0, 0) with dithering off and frame-buffer
 *           dithering on and queues the 0x0C-byte mode primitive at ordering
 *           table index 1, then fills the POLY_G4 at the shared primitive
 *           cursor g_PrimCursor with a slanted gouraud band: the top edge runs
 *           from x arg0 - 0x40 to x 0x140 at y arg1, the bottom edge runs from
 *           x arg0 to x 0x140 at y arg1 + 0x30, and the four corners are tinted
 *           0x00, 0xFF, 0x40 and 0xFF grey. The primitive is marked
 *           semi-transparent with shading disabled and queued as a 0x24-byte
 *           primitive at the same ordering table index. Takes the moving
 *           left-edge reference and the band top edge and returns nothing; the
 *           only in-image caller passes the work record halfword at offset 0x2E
 *           and 0xC8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSlantedSubtractiveGouraudBandWorld04Area17213_801F4F70(s32 arg0,
    s32 arg1) {
  POLY_G4* prim;
  s16 bottom;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 2, 0x3C0, 0), 0);
  GpuAppendPrim(1, 0xc);
  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  prim->x0 = arg0 - 0x40;
  bottom = arg1 + 0x30;
  prim->y3 = bottom;
  prim->y2 = bottom;
  prim->x3 = 0x140;
  prim->x1 = 0x140;
  prim->x2 = arg0;
  prim->y1 = arg1;
  prim->y0 = arg1;
  prim->r0 = 0x00;
  prim->g0 = 0x00;
  prim->b0 = 0x00;
  prim->r1 = 0xFF;
  prim->g1 = 0xFF;
  prim->b1 = 0xFF;
  prim->r2 = 0x40;
  prim->g2 = 0x40;
  prim->b2 = 0x40;
  prim->r3 = 0xFF;
  prim->g3 = 0xFF;
  prim->b3 = 0xFF;
  SetSemiTrans(prim, 1);
  SetShadeTex(prim, 0);
  GpuAppendPrim(1, 0x24);
}
