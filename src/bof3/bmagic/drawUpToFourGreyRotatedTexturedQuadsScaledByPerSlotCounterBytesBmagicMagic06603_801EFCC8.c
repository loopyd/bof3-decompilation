#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad quad parameters of this overlay at 0x1F800000. The body stores a
 * signed half-extent into 0x1F800000, an unsigned grey shade into 0x1F800002
 * and an unsigned depth into 0x1F800004, and re-reads each of the three through
 * one base pointer, so the cells are one struct behind one pointer local. */
typedef struct {
  s16 width;
  u16 shade;
  u16 depth;
} QuadParams;

extern QuadParams D_1F800000; /* @source 0x1F800000 @kind data */

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects. The
 * original keeps their base in a saved register for the four vector arguments
 * and writes the halfword lanes through that base. */
extern SVECTOR D_1F800014[]; /* @source 0x1F800014 @kind data */

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The loop re-reads the cell for every step, so byte 0x9 is a fresh load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Per-slot counter bytes of this overlay at 0x801F00B0, runtime-filled (the
 * payload holds zeros) and cleared by the slot routine at 0x801EF878. The
 * original walks them with a pointer that advances once per step. */
extern u8 D_801F00B0[]; /* @source 0x801F00B0 @kind data */

/* Shared phase samplers of the main executable: both return the sample of a
 * 12-bit phase in the same fixed-point scale as the shift-right-twelve products
 * below. This body takes the 0x801783C8 sample for x and the 0x801782FC sample
 * for y. */
extern s32 func_801782FC(s32 arg0); /* @source 0x801782FC */
extern s32 func_801783C8(s32 arg0); /* @source 0x801783C8 */

/* @source 0x801EFCC8
 * @behavior Draws up to four semi-transparent grey textured quads of the
 *           transform the caller installed through 0x801EFFB0. Each of the four
 *           steps re-reads the scratchpad work-object pointer cell 0x1F800044
 *           and draws only while three times the step index stays below that
 *           object byte 0x9 and the step counter byte at 0x801F00B0 stays below
 *           0x10; a failing step is skipped and the loop advances. A drawn step
 *           stamps POLY_FT4 at the primitive cursor, sets semi-transparency with
 *           argument 1, writes counter times four plus 0x32 as the half-extent
 *           and counter times fifteen as the depth to the scratchpad cells
 *           0x1F800000 and 0x1F800004, and fills the four scratchpad vectors at
 *           0x1F800014 with the phase corners 0x200, 0x600, 0xE00 and 0xA00 of
 *           the 4096-unit turn: each corner takes the 0x801783C8 sample scaled
 *           by the half-extent shifted right twelve as x and the 0x801782FC
 *           sample scaled the same way as y, and the negated depth as every z.
 *           It then selects texture page 0x340 with semi-transparency, clut
 *           0x1E2 and a 0x20-square texture whose corners are (0x0,0x0),
 *           (0x20,0x0), (0x0,0x20) and (0x20,0x20), writes the grey shade 0x41
 *           minus four times half the counter byte to the red, green and blue
 *           lanes, projects the four vectors into the primitive x0 to x3 slots
 *           through RotTransPers4, and links the 0x28-byte primitive at output
 *           list 3 through the shared append routine. It takes no arguments and
 *           returns nothing; the roles of overlay counter byte 0x801F00B0 and
 *           work-object byte 0x9 stay offset-anchored.
 * @status partial
 * @match 95.16
 * @residual Single register exchange in the texture-coordinate and shade block:
 *           every store site, value, call and memory event is in place and the
 *           body is the original size, but the shared value local takes $v1
 *           where the original keeps $v0, so the 0x20 literal is issued before
 *           the clut store, $a0 and $a1 of RotTransPers4 follow it, and the
 *           shade subtraction writes $v1 instead of $v0.
 */
void drawUpToFourGreyRotatedTexturedQuadsScaledByPerSlotCounterBytesBmagicMagic06603_801EFCC8(
    void) {
  s32 i = 0;
  QuadParams *params = &D_1F800000;
  SVECTOR *vec = D_1F800014;
  u8 *counter = D_801F00B0;
  s32 k = 0;
  s32 v;
  POLY_FT4 *prim;
  long p;
  long flag;

  for (; i < 4; i++, k += 3) {
    if (k < g_battle_work[9] && *counter < 0x10) {
      prim = (POLY_FT4 *)g_PrimCursor;
      SetPolyFT4(prim);
      SetSemiTrans(prim, 1);

      params->width = *counter * 4 + 0x32;
      params->depth = *counter * 15;

      vec[0].vx = (func_801783C8(0x200) * params->width) >> 12;
      vec[0].vy = (func_801782FC(0x200) * params->width) >> 12;
      vec[0].vz = -params->depth;
      vec[1].vx = (func_801783C8(0x600) * params->width) >> 12;
      vec[1].vy = (func_801782FC(0x600) * params->width) >> 12;
      vec[1].vz = -params->depth;
      vec[2].vx = (func_801783C8(0xE00) * params->width) >> 12;
      vec[2].vy = (func_801782FC(0xE00) * params->width) >> 12;
      vec[2].vz = -params->depth;
      vec[3].vx = (func_801783C8(0xA00) * params->width) >> 12;
      vec[3].vy = (func_801782FC(0xA00) * params->width) >> 12;
      vec[3].vz = -params->depth;

      prim->tpage = GetTPage(0, 1, 0x340, 0x100);
      prim->clut = GetClut(0, 0x1E2);

      v = 0x20;
      prim->u0 = 0;
      prim->v0 = 0;
      prim->u1 = v;
      prim->v1 = 0;
      prim->u2 = 0;
      prim->v2 = v;
      prim->u3 = v;
      prim->v3 = v;

      v = 0x41 - ((*counter >> 1) << 2);
      params->shade = v;
      prim->r0 = v;
      prim->g0 = params->shade;
      prim->b0 = params->shade;

      RotTransPers4(vec, vec + 1, vec + 2, vec + 3, (long *)&prim->x0,
                    (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                    &p, &flag);

      GpuAppendPrim(3, 0x28);
    }
    counter++;
  }
}
