#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name this overlay's exact lifts
 * drawTranslucentGouraudQuadAroundWorkHalfwords2EAnd30WithHalfExtentsFromWork
 * Bytes9AndAGradedBlackToDarkBlueBmagicMagic00603_801F0308 and
 * add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvoke
 * HelperWhenZeroBmagicMagic00603_801F0290 already use. This body reads the cell
 * once, after the opening draw-mode packet is linked and before the first
 * publish, and reaches its unsigned byte 9 twice from that one load, so the
 * pointer stays in a caller-saved register across the two reads and the cell is
 * never re-formed. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000, the cell this overlay's radial emitters
 * scale their phase samples by. The original builds its address with one
 * `lui 0x1F80` plus `addiu` pair into $s1 before the first publish and copies
 * that register to $s5 before the loop, then addresses the radius as offset 0
 * and the three tint cells as offsets 0xA, 0xC and 0xE of it, so the cell is a
 * live base pointer here. The radius itself is the signed halfword read the
 * `lh` and the following `sra` in both products show, while the three tint
 * reads are unsigned halfwords. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad next-sample cell at 0x1F800024, the x halfword this body writes
 * before entering the loop and then carries forward each pass. The original
 * forms this address into $s0 with its own `lui 0x1F80` plus `addiu` pair before
 * the first sampler call and later derives the vertex triplet base by
 * subtracting 0x10 from that register, so the triplet is reached through this
 * cell rather than through a symbol at its own first vertex. Its y neighbour at
 * 0x1F800026 and the three z halfwords at 0x1F800028, 0x1F800020 and 0x1F800018
 * are published through literal constants instead, the absolute `lui $at` plus
 * `sh` form the compiler emits for a direct fixed-address store. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared phase samplers: both return the sine and cosine of a 12-bit phase in
 * the same fixed-point scale as the `>> 12` products below. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper that sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic siblings give their own seed
 * helper. */
void func_8017A97C(void *arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* @source 0x801F0790
 * @behavior Translucent radial fan emitter of this overlay draw group with no
 *           caller in the payload: no `jal`, no jump and no pointer word of any
 *           extracted payload names 0x801F0790, and the body reads no incoming
 *           register, so the overlay loader entry is its only route and it takes
 *           no arguments and returns nothing.
 *           It first picks the draw-mode texture page, 0xA5 when GetGraphType
 *           returns 1 or 2 and 0x35 otherwise, calling it once per test with no
 *           cached result, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links that
 *           0xC-byte packet under ordering-table slot 5.
 *           It then reads the scratchpad work-object pointer cell at 0x1F800044
 *           once and publishes the numbers the fan runs on: 0x100 into the
 *           scratchpad radius cell 0x1F800000, the work-object unsigned byte
 *           0x9 times eight into the red cell 0x1F80000A, the same byte times
 *           two into the green cell 0x1F80000C and the constant 1 into the blue
 *           cell 0x1F80000E. Byte 0x9 is read once per product, so the tint of
 *           every vertex is the fixed triple (byte 0x9 * 8, byte 0x9 * 2, 1).
 *           The three tint stores are the ascending source order the emitted
 *           schedule reorders: the constant 1 reaches the blue cell after the
 *           green product because the constant itself only becomes free once
 *           the work pointer it shared a register with is dead.
 *           The next scratchpad sample is then seeded at phase zero: the x
 *           halfword 0x1F800024 takes func_801782FC(0) times the signed radius
 *           shifted right twelve, its y neighbour 0x1F800026 takes
 *           func_801783C8(0) times the same radius shifted right twelve, and
 *           the three z halfwords of the vertex triplet are cleared.
 *           The loop runs sixteen passes with the phase advancing by 0x100 each
 *           time, so the sixteen wedges span the whole 0x1000 phase circle at
 *           one radius and form a full disc. Each pass takes a fresh
 *           g_PrimCursor primitive, seeds it through func_8017A97C, marks it
 *           semi-transparent, clears the origin vertex pair, carries the
 *           previous pass sample pair into the second vertex, and fills the
 *           third vertex pair with the func_801782FC and func_801783C8 samples
 *           of the current phase scaled by the signed radius and shifted right
 *           twelve. The three vertices are then projected through
 *           RotTransPers3 into the primitive vertex pairs at 0x08, 0x10 and
 *           0x18, and the primitive receives the tint triplet in the red, green
 *           and blue bytes of its first vertex, 1 in the same three bytes of the
 *           second and the third, and is linked as the 0x1C-byte POLY_G3 size
 *           under the same slot 5.
 *           After the loop the body selects the draw-mode texture page again,
 *           this time 0x25 when GetGraphType returns 1 or 2 and 0x15 otherwise,
 *           re-arms SetDrawMode the same way and links a second 0xC-byte packet
 *           under slot 5, then returns.
 *           The 0x48-byte frame keeps $ra, $s5, $s4, $s3, $s2, $s1 and $s0 plus
 *           the fifth SetDrawMode argument at 0x10, the four RotTransPers3 stack
 *           arguments at 0x10 to 0x1C and the sampler outputs at 0x20 and 0x24.
 *           The roles of work-object byte 0x9 and of the radius beyond this
 *           published geometry are not established, so they stay
 *           offset-anchored, and the scratchpad cells are named by address.
 *           Shape evidence, not clone proof: no payload under out/binaries holds
 *           this 660-byte span with its sixteen `jal` and `j` words masked, and
 *           the nearest payload body, the radial emitter at 0x801EF9E4 of
 *           another bmagic overlay payload, shares 98 of the 149 non-relocation
 *           words and carries its own texture page constants, so it is a shape
 *           donor at most. The statement order and the scratchpad access forms
 *           follow the exact bmagic siblings
 *           drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetime at
 *           0x801EF5F4 and drawTranslucentGouraudTriangleFromProjectedOrigin
 *           AlongZAtTwoDepthStepsFadingFromWorkByteATimesSixToBlack at
 *           0x801EF0E4, and the exact scenario fan
 *           drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFrom
 *           StartAngle.
 * Live evidence: 165 of 165 instructions and 660 of 660 bytes, at the original
 *           span size with no differing word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100TintedByWorkByte9WeightedEightTwoOneBmagicMagic00603_801F0790(
    void) {
  u8 *work;
  u16 *cells;
  /* Same cell as cells, taken as its own local: the original keeps the
   * pre-loop base and the loop base in different callee-saved registers
   * because the loop re-uses the first one for the shifted phase, and copies
   * one into the other before the loop, which only a second live value
   * reproduces. */
  u16 *tint;
  u16 *next;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 graph_value;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph_value = 0xA5;
  } else {
    graph_value = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph_value, 0);
  GpuAppendPrim(5, 0xC);

  next = &D_1F800024;
  work = g_battle_work;
  cells = &D_1F800000;
  *cells = 0x100;
  SPAD_REF(u16, 0x0A) = work[9] * 8;
  SPAD_REF(u16, 0x0C) = work[9] * 2;
  SPAD_REF(u16, 0x0E) = 1;

  *next = (func_801782FC(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800028 = 0;
  *(u16 *)0x1F800020 = 0;
  *(u16 *)0x1F800018 = 0;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  tint = cells;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;

    phase = i << 8;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)tint) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)tint) >> 12;

    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);

    prim->r0 = tint[5];
    prim->g0 = tint[6];
    prim->b0 = tint[7];
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;

    GpuAppendPrim(5, 0x1C);
    i++;
  } while (i < 0x11);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph_value = 0x25;
  } else {
    graph_value = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph_value, 0);
  GpuAppendPrim(5, 0xC);
}
