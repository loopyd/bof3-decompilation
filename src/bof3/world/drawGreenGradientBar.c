#include "bof3/world/area03004_internal.h"

/* @source 0x801D1B88
 * @behavior Builds the AREA030 two-strip green gradient bar in place at the
 *           shared primitive cursor g_PrimCursor and appends each 0x24-byte
 *           Gouraud quad through GpuAppendPrim under the caller graphics-mode
 *           byte arg3. The upper strip spans (arg0, arg1) to (arg0 + arg2, arg1)
 *           with black 0x00/0x00/0x00 on its top edge and 0x00/0x80/0x00 four
 *           rows below; the lower strip repeats that x pair one 4-row step down
 *           with the two edge colours exchanged, so the 8-row band runs
 *           black-green-black. Each strip reads its x and y values back out of
 *           the quad and re-reads the cursor cell before the second SetPolyG4,
 *           the same shape as the sibling quad builder func_801D1744. Both
 *           callers pass a width already clamped to 0x60 (the ramp maximum in
 *           func_801D1CF0 and func_801D5D88) and an append mode of 1 or 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The lower-row y value goes through a named s32 temporary so both lower vertex
 * rows are stored from one computed value: `y3 = (y_lower = (y2 = y0 + 0x04))`.
 * That is the retained form of the exact sibling func_801D1744, whose build
 * needed the named lifetime to keep the append-argument mask hoisted and the
 * x1 store/read-back pair adjacent. */
void drawGreenGradientBar(s16 arg0, s16 arg1, s16 arg2, u8 arg3) {
  s32 y_lower;
  POLY_G4* primitive;

  primitive = (POLY_G4*)g_PrimCursor;
  SetPolyG4(primitive);
  primitive->x0 = arg0;
  primitive->x1 = arg0 + arg2;
  primitive->y0 = arg1;
  primitive->y1 = arg1;
  primitive->r0 = 0x00;
  primitive->g0 = 0x00;
  primitive->b0 = 0x00;
  primitive->r1 = 0x00;
  primitive->g1 = 0x00;
  primitive->b1 = 0x00;
  primitive->r2 = 0x00;
  primitive->g2 = 0x80;
  primitive->b2 = 0x00;
  primitive->r3 = 0x00;
  primitive->g3 = 0x80;
  primitive->b3 = 0x00;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y3 = (y_lower = (primitive->y2 = primitive->y0 + 0x04));
  GpuAppendPrim(arg3, 0x24);

  primitive = (POLY_G4*)g_PrimCursor;
  SetPolyG4(primitive);
  primitive->x0 = arg0;
  primitive->x1 = arg0 + arg2;
  primitive->y0 = arg1 + 0x04;
  primitive->y1 = arg1 + 0x04;
  primitive->r0 = 0x00;
  primitive->g0 = 0x80;
  primitive->b0 = 0x00;
  primitive->r1 = 0x00;
  primitive->g1 = 0x80;
  primitive->b1 = 0x00;
  primitive->r2 = 0x00;
  primitive->g2 = 0x00;
  primitive->b2 = 0x00;
  primitive->r3 = 0x00;
  primitive->g3 = 0x00;
  primitive->b3 = 0x00;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y3 = (y_lower = (primitive->y2 = primitive->y0 + 0x04));
  GpuAppendPrim(arg3, 0x24);
}
