#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name this overlay's exact lifts already use. This body
 * loads the cell once after the opening draw-mode packet is linked and reaches
 * its unsigned byte 0x09 and its unsigned byte 0x04 only through that one
 * register, so the pointer stays live across the whole publish block and the
 * cell is never re-formed. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800006, the scale both rotator samples are
 * multiplied by and the cell this body publishes 0x88 into before the first
 * publish. The original forms its address once with a `lui %hi` plus `addiu
 * %lo` pair into a callee-saved register, reads it with a signed `lh` after
 * every sampler call, keeps a second copy of that address in another
 * callee-saved register for the loop, and reaches the three published colour
 * halfwords as offsets -0x6, -0x4 and -0x2 of it, so the cell is a live pointer
 * base rather than three separate absolute addresses. */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Scratchpad fan cell at 0x1F800024, the x halfword this body publishes before
 * entering the loop and then carries forward each pass. The original forms this
 * address into its own callee-saved register with a `lui %hi` plus `addiu %lo`
 * pair and derives the three-vertex shape base by subtracting 0x10 from that
 * register, so the shape is reached through this cell rather than through a
 * symbol at its own first vertex. Its y neighbour at 0x1F800026 and the three z
 * halfwords are published through literal constants instead, the absolute `lui
 * $at` plus `sh` form the compiler emits for a direct fixed-address store. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Three byte rows of the palette block at 0x801F0404, indexed by the work
 * object's byte 0x04 scaled by three. The original masks each row with its own
 * %hi/%lo pair over a base of 0x801F0404, 0x801F0405 and 0x801F0406, so the
 * three rows are separate symbols and the three-byte stride is what selects the
 * record. The block lives in this payload's trailing read-only subsegment, so
 * only the sub-span is bound here by its address label. */
extern u8 D_801F0404[]; /* @source 0x801F0404 @kind table */
extern u8 D_801F0405[]; /* @source 0x801F0405 @kind table */
extern u8 D_801F0406[]; /* @source 0x801F0406 @kind table */

/* Shared phase samplers: both return the sine and cosine of a 12-bit phase in
 * the same fixed-point scale as the `>> 12` products below. This body seeds
 * them with phase zero and then with `pass << 8`. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper that sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic fan siblings give their own seed
 * helper. The payload names 0x8017A97C only from spans of this draw group, so
 * the tag carries the address. */
void func_8017A97C(void *arg0); /* @source 0x8017A97C */

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* Shape evidence, not clone proof: a payload scan of every extracted binary
 * found this 944-byte span only at its own offset, so the clone lever is
 * ineligible and the body is a clean-C seed. The statement order, the operand
 * forms and the scratchpad access spellings follow the exact bmagic fan
 * siblings drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100
 * TintedByWorkByte9WeightedEightTwoOne at 0x801F0790, which shares the
 * pre-loop sampler seed, the sixteen-state `i << 8` phase walk, the
 * RotTransPers3 projection and the closing second draw-mode selection, and
 * drawTranslucentGouraudQuadAboveWorkHalfwords2EAnd30WithHalfWidthFromWorkByteA
 * AndHeightFromWorkByte9GradedBlackToPaletteRowAtWorkByte4 at 0x801EF758 of
 * this same target, which shares the three-row palette table indexing and the
 * graded colour layout. The literals, the palette rows, the radius 0x88, the
 * frame size and the two texture-page constants are this span's own. */
/* @source 0x801F0054
 * @behavior Third draw helper of this overlay's work-byte-2 handler group. Its
 *           only in-image reference is the jal at 0x801EF050 inside
 *           dispatchWorkByte2HandlerThenInvokeDrawHelpersAndPopWhenActive
 *           BmagicMagic03903_801EEFA8, taken while the work object's byte 0 has
 *           bit 0 set and its byte 2 is non-zero, between func_801EFAA4 and
 *           PopMatrix; the body reads no incoming register, so it takes no
 *           arguments and returns nothing.
 *           It first picks the draw-mode texture page, 0xA5 when GetGraphType
 *           returns 1 or 2 and 0x35 otherwise, calling it once per test with no
 *           cached answer, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links that
 *           0xC-byte packet under ordering-table slot 5.
 *           It then reads the scratchpad work-object pointer cell at 0x1F800044
 *           once, publishes the constant 0x88 into the scratchpad radius
 *           halfword 0x1F800006, and drives the three colour halfwords
 *           0x1F800000, 0x1F800002 and 0x1F800004 from the palette row the work
 *           object's unsigned byte 0x04 selects: while its unsigned byte 0x09
 *           is below 0x10 each row byte is multiplied by that byte, and from
 *           0x10 up each row byte is shifted left four and the byte itself is
 *           not read, so the fan saturates at sixteen steps. Byte 0x09 and byte
 *           0x04 are re-read for every product because the publishes sit in the
 *           same scratchpad page the pointer may address.
 *           The fan is then seeded at phase zero: the x halfword 0x1F800024
 *           takes func_801782FC(0) times the signed radius shifted right twelve,
 *           its y neighbour 0x1F800026 takes func_801783C8(0) times the same
 *           radius shifted right twelve, and the three z halfwords 0x1F800018,
 *           0x1F800020 and 0x1F800028 are cleared.
 *           The loop runs sixteen passes with the phase advancing by 0x100 each
 *           time, one sixteenth of the 0x1000 turn, so the wedges span the whole
 *           circle at one radius. Each pass takes a fresh g_PrimCursor
 *           primitive, seeds it through func_8017A97C, marks it
 *           semi-transparent, clears the origin vertex pair, carries the
 *           previous pass sample pair into the second vertex, and fills the
 *           third vertex pair with the func_801782FC and func_801783C8 samples
 *           of the current phase scaled by the signed radius and shifted right
 *           twelve. The three vertices are projected through RotTransPers3 into
 *           the primitive vertex pairs at 0x08, 0x10 and 0x18; the origin vertex
 *           takes 1 in its red, green and blue bytes while both rim vertices
 *           take the published palette triple, and the 0x1C-byte POLY_G3 packet
 *           is linked under the same slot 5.
 *           After the loop the body selects the draw-mode texture page again,
 *           this time 0x25 when GetGraphType returns 1 or 2 and 0x15 otherwise,
 *           re-arms SetDrawMode the same way and links a second 0xC-byte packet
 *           under slot 5, then returns.
 *           The 0x40-byte frame keeps $ra, $s4, $s3, $s2, $s1 and $s0 plus the
 *           fifth SetDrawMode argument at 0x10, the sampler outputs at 0x20 and
 *           0x24 and the four RotTransPers3 stack arguments at 0x10 to 0x1C.
 *           The roles of work-object bytes 0x04 and 0x09 beyond the palette row
 *           and intensity uses above are not established, so they stay
 *           offset-anchored, and the scratchpad cells are named by address.
 *           A signed halfword colour local carries the dark literal and each
 *           later palette load. The palette reads retain their halfword width;
 *           assignment to the primitive bytes keeps each value's low byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius136GradedBlackToPaletteRowAtWorkByte4ScaledByWorkByte9BmagicMagic03903_801F0054(
    void) {
  u8 *work;
  u16 *cell;
  /* Same cell as cell, taken as its own local: the original keeps the pre-loop
   * base in $s0 and the loop base in $s3 and copies one into the other before
   * the loop, because the loop re-uses $s0 for the primitive cursor, which only
   * a second live value reproduces. */
  u16 *radius;
  u16 *next;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 tpage;
  s32 phase;
  s32 i;
  s16 shade;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  work = g_battle_work;
  *(u16 *)0x1F800006 = 0x88;
  if (work[9] < 0x10) {
    *(u16 *)0x1F800000 = D_801F0404[work[4] * 3] * work[9];
    *(u16 *)0x1F800002 = D_801F0405[work[4] * 3] * work[9];
    *(u16 *)0x1F800004 = D_801F0406[work[4] * 3] * work[9];
  } else {
    *(u16 *)0x1F800000 = D_801F0404[work[4] * 3] << 4;
    *(u16 *)0x1F800002 = D_801F0405[work[4] * 3] << 4;
    *(u16 *)0x1F800004 = D_801F0406[work[4] * 3] << 4;
  }

  next = &D_1F800024;
  cell = &D_1F800006;
  *next = (func_801782FC(0) * *(s16 *)cell) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *(s16 *)cell) >> 12;
  *(u16 *)0x1F800028 = 0;
  *(u16 *)0x1F800020 = 0;
  *(u16 *)0x1F800018 = 0;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  radius = cell;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    phase = i << 8;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)radius) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)radius) >> 12;

    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);

    shade = 1;
    prim->r0 = shade;
    prim->g0 = shade;
    prim->b0 = shade;
    shade = radius[-3];
    prim->r1 = shade;
    shade = radius[-2];
    prim->g1 = shade;
    shade = radius[-1];
    prim->b1 = shade;
    shade = radius[-3];
    prim->r2 = shade;
    shade = radius[-2];
    prim->g2 = shade;
    shade = radius[-1];
    prim->b2 = shade;

    GpuAppendPrim(5, 0x1C);
    i++;
  } while (i < 0x11);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
}
