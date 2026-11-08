#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
extern s32 D_80143E6C;

void func_8014E5A0(u8 arg0, u8 arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_8017A97C(void* arg0);

/* @source 0x801D17A0
 * @behavior Draws this overlay's translucent radial fade. It first arms the
 * draw mode exactly as the work-table step and the full-screen band of this
 * target do - GetTPage(0, 1, 0x380, 0x100) yields the texture page,
 * SetDrawMode installs that page on the primitive published at g_PrimCursor,
 * and func_8014E5A0 queues the 0x0C-byte DR_MODE packet - then stamps a fan of
 * thirty-two Gouraud triangles into that same primitive cursor and appends each
 * one under OT slot 7 as a 0x1C-byte packet, the POLY_G3 size. Every triangle
 * keeps its first vertex on the screen bottom centre (0xA0, 0xF0) and carries
 * the shade byte there as red with green and blue at a quarter of it, while its
 * two arc vertices carry black. The fan phase starts at 0x800 and advances by
 * 0x40 per triangle through an unsigned byte counter, so triangle n spans the
 * samples of phases 0x800 + 0x40n and 0x800 + 0x40(n + 1): the sample kept from
 * the previous iteration fills the triangle's first arc vertex and the freshly
 * taken one its second, so neighbouring triangles meet at the shared sample.
 * Each sample offsets the bottom centre by the two signed halfword arguments -
 * the reported pair the work object keeps at +0x2E and +0x30, which the run
 * lowers every frame - scaled through the local rotation helpers func_801783C8
 * for x and func_801782FC for y, both of which the sibling radial emitters of
 * this module multiply and shift down by twelve, so the two arguments are
 * fixed-point half extents rather than screen coordinates. The two arc offsets
 * also carry the parity offset (D_80143E6C & 1) << 2: the low bit of the
 * per-frame counter, shifted left twice, moves the arc four pixels down and to
 * the right on every other frame while the bottom centre vertex stays put. It
 * takes the two signed half extents and the shade, returns nothing and keeps no
 * result; the 0x50-byte frame holds $ra with $s0-$s7 and $fp across the calls,
 * keeps the two samples in adjacent halfword slots and spills the sign-extended
 * x into a word slot.
 *
 * Evidence: the draw-mode arming is the prologue the exact siblings
 * stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624
 * and drawFullScreenRedGradientBandCenteredOnScreenCenterScenarioSce15ef200_801D19A8
 * of this target share, and the per-triangle body repeats the exact sibling
 * emitRadialTranslucentQuads (0x801D240C) of the scenario module: cursor reload,
 * func_8017A97C, SetSemiTrans with one, the coordinate stores at +0x8, +0xA,
 * +0x10, +0x12, +0x18 and +0x1A, the three colour triples at +0x04, +0x0C and
 * +0x14 and the append through func_8014E5A0 - with 0x1C in place of that
 * body's 28. The single caller
 * dispatchWorkByte1HandlerThenDrawScreenPairScenarioSce15ef200_801D0EF8 loads
 * the first two arguments with `lh` from the work object at +0x2E and +0x30 and
 * passes the literal 0x80, which fixes the two signed halfword widths, the sign
 * extension of the first one at 0x801D182C, and the third argument's byte width
 * that the `sb` store of the raw register and the `andi`/`srl` pair deriving
 * green and blue from it both show. The two samples are read back from adjacent
 * halfword frame slots rather than kept in registers, so they are one pair local
 * of this body; the andi/srl pair is emitted once before the loop and reused by
 * both the green and the blue store, so those two stores repeat one expression
 * rather than reading a stored colour local.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfThirtyTwoGouraudTrianglesAroundScreenBottomCenterScenarioSce15ef200_801D17A0(
    s16 half_width, s16 half_height, u8 shade) {
  POLY_G3* prim;
  u16 tpage;
  s32 phase;
  u16 angle;
  u8 parity;
  u8 i;
  s16 next[2];

  tpage = GetTPage(0, 1, 0x380, 0x100);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(7, 0xC);
  parity = (D_80143E6C & 1) << 2;
  phase = 0x800;
  next[0] = ((half_width * func_801783C8(phase)) >> 12) + 0xA0 + parity;
  next[1] = ((half_height * func_801782FC(phase)) >> 12) + 0xF0 + parity;
  i = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = 0xA0;
    prim->y0 = 0xF0;
    phase += 0x40;
    prim->x1 = next[0];
    prim->y1 = next[1];
    angle = phase;
    next[0] = ((half_width * func_801783C8(angle)) >> 12) + 0xA0 + parity;
    next[1] = ((half_height * func_801782FC(angle)) >> 12) + 0xF0 + parity;
    prim->x2 = next[0];
    prim->y2 = next[1];
    prim->r0 = shade;
    prim->g0 = (u8)shade >> 2;
    prim->b0 = (u8)shade >> 2;
    prim->r1 = 0;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;
    func_8014E5A0(7, 0x1C);
    i++;
  } while (i < 0x20);
}
