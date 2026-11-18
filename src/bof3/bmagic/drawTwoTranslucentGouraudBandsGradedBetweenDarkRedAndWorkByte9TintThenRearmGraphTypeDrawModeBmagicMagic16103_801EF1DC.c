#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell this overlay steps, spelled through the
 * same address-derived symbol the target's other lifts use. The body reads the
 * cell once and takes unsigned byte 9 through it as the tint source. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable graph-type query bound by the PsyQ map at 0x8017B2B4.
 * It takes no arguments and returns the current integer graph type. */
s32 GetGraphType(void);

/* @source 0x801EF1DC
 * @behavior Full-screen gradation drawer of this overlay's battle-magic effect
 *           sequence, the draw step that the work-byte-9 handler at 0x801EF0E8
 *           reaches by jal after its own state update. It takes no arguments
 *           and returns nothing. It first picks a draw-mode word by graph
 *           type: 0xA5 when GetGraphType returns 1, 0xA5 again when the second
 *           call returns 2, and 0x35 for every other type; the arms share one
 *           store, so the first comparison hoists its constant into the branch
 *           delay slot and the later arms overwrite it. It seeds SetDrawMode at
 *           g_PrimCursor with dfe 0, dtd 1 and that word, and appends the
 *           0xC-byte draw-mode packet with GpuAppendPrim(1, 0xC).
 *           It then reads the work object through the pointer cell at
 *           0x1F800044 once, takes a POLY_G4 at g_PrimCursor, seeds it with
 *           SetPolyG4, marks it semi-transparent with the argument 1, and
 *           fills the upper screen band: x0 0, y0 0, x1 0x13F, y1 0, x2 0,
 *           y2 0x78, x3 0x13F, y3 0x78. Vertices 0 and 1 take the fixed colour
 *           0xC0, 0x60, 0x60; vertices 2 and 3 take red twelve times and green
 *           and blue six times the unsigned work byte 9 read at entry. The
 *           0x24-byte packet is closed with GpuAppendPrim(1, 0x24).
 *           A second POLY_G4 re-read from g_PrimCursor repeats the same
 *           treatment for the lower band: y0 and y1 0x78, y2 and y3 0x167,
 *           with the work-byte-9 tint on vertices 0 and 1 and the fixed colour
 *           on vertices 2 and 3, closed with GpuAppendPrim(1, 0x24).
 *           The tail re-arms the draw mode for whatever the caller emits next:
 *           the same graph-type selection with 0x25 for types 1 and 2 and 0x15
 *           for every other type, SetDrawMode at g_PrimCursor with dfe 0 and
 *           dtd 1, and a second 0xC-byte draw-mode packet.
 *           The 0x38-byte frame keeps $ra at 0x34 and $s0 through $s6 at
 *           0x18 to 0x30 across the calls. The only stack word written beyond
 *           the frame is the fifth SetDrawMode argument, nothing is returned
 *           and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation note: taking the two channel products before the packet seed
 * is what keeps the byte out of a callee-saved register and lets the shift
 * chain fill the SetPolyG4 delay slot. The seed that kept the work byte in a
 * live tint local and multiplied it at each store scored 102 of 131
 * instructions and 528 bytes, first difference at +0x70, where the byte landed
 * in a callee-saved register behind a load nop.
 */
void drawTwoTranslucentGouraudBandsGradedBetweenDarkRedAndWorkByte9TintThenRearmGraphTypeDrawModeBmagicMagic16103_801EF1DC(
    void) {
  POLY_G4 *prim;
  u8 *work;
  s32 mode;
  s32 tint;
  s32 red;
  s32 green;

  if (GetGraphType() == 1) {
    mode = 0xA5;
  } else if (GetGraphType() == 2) {
    mode = 0xA5;
  } else {
    mode = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
  GpuAppendPrim(1, 0xC);

  work = g_battle_work;
  prim = (POLY_G4 *)g_PrimCursor;
  tint = work[9];
  red = tint * 12;
  green = tint * 6;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  prim->x0 = 0;
  prim->y0 = 0;
  prim->x1 = 0x13F;
  prim->y1 = 0;
  prim->x2 = 0;
  prim->y2 = 0x78;
  prim->x3 = 0x13F;
  prim->y3 = 0x78;
  prim->r0 = 0xC0;
  prim->g0 = 0x60;
  prim->b0 = 0x60;
  prim->r1 = 0xC0;
  prim->g1 = 0x60;
  prim->b1 = 0x60;
  prim->r2 = red;
  prim->g2 = green;
  prim->b2 = green;
  prim->r3 = red;
  prim->g3 = green;
  prim->b3 = green;
  GpuAppendPrim(1, 0x24);

  prim = (POLY_G4 *)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  prim->x0 = 0;
  prim->y0 = 0x78;
  prim->x1 = 0x13F;
  prim->y1 = 0x78;
  prim->x2 = 0;
  prim->y2 = 0x167;
  prim->x3 = 0x13F;
  prim->y3 = 0x167;
  prim->r0 = red;
  prim->g0 = green;
  prim->b0 = green;
  prim->r1 = red;
  prim->g1 = green;
  prim->b1 = green;
  prim->r2 = 0xC0;
  prim->g2 = 0x60;
  prim->b2 = 0x60;
  prim->r3 = 0xC0;
  prim->g3 = 0x60;
  prim->b3 = 0x60;
  GpuAppendPrim(1, 0x24);

  if (GetGraphType() == 1) {
    mode = 0x25;
  } else if (GetGraphType() == 2) {
    mode = 0x25;
  } else {
    mode = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
  GpuAppendPrim(1, 0xC);
}
