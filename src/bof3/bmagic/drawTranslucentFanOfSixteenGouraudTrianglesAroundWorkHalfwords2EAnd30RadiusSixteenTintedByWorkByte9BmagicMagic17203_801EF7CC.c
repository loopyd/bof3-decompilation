#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. This body reads the cell
 * once, after the opening draw-mode packet is linked, and reaches the work
 * record through that one load: the first unsigned byte 9 read, the two screen
 * halfwords at 0x2E and 0x30 and the three tint reads that follow all use the
 * same register. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000 and phase cell at 0x1F800004. The body
 * publishes the radius 0x10 once before the loop and the phase 0 at the same
 * time, then reads the radius four times per pass and reads, stores and
 * re-reads the phase around the second vertex pair. The original materialises
 * the phase cell address once before the work-record load, carries it into
 * $s3 for the loop and reaches the radius and the four tint cells at -0x4 and
 * +0x6..+0xC off that register, so the pair stays two live named rows and the
 * loop works through them. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Shared phase samplers of the main executable, bound by this target's
 * generated undefined_funcs_auto.txt. Both take a 12-bit phase and return a
 * fixed-point sample in the scale the `>> 12` products below divide by; the
 * bmagic fan siblings declare the same pair with the same prototypes. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper that sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic fan sibling
 * drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100TintedBy
 * WorkByte9WeightedEightTwoOneBmagicMagic00603_801F0790 gives its own seed
 * helper. */
void func_8017A97C(void *arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* @source 0x801EF7CC
 * @behavior Overlay draw step of this payload with two callers and no pointer
 *           word naming it: the overlay render body at 0x801EEE7C calls it with
 *           a bare `jal` twice, once between a helper call and the overlay draw
 *           step at 0x801EFA2C. It reads no incoming register, so it takes no
 *           arguments and returns nothing.
 *           It picks the draw-mode texture page, 0xA5 when GetGraphType returns
 *           1 and 0x35 when it returns 2, calling the query once per test with
 *           no cached result, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links that
 *           0xC-byte packet under ordering-table slot 3. It then publishes the
 *           fan state in scratchpad, a radius of 0x10 at 0x1F800000, a phase of
 *           0 at 0x1F800004 and four tint cells scaled from work byte 9, and
 *           emits sixteen translucent Gouraud triangles into ordering-table slot
 *           3 at 0x1C bytes each. Every triangle shares the screen centre at
 *           unsigned halfwords 0x2E and 0x30 of the work record, a grey core
 *           colour, and two rim vertices tinted from the remaining cells; the
 *           first rim vertex pair is sampled at the phase published on the
 *           previous pass and the second at the phase advanced by 0x100 per
 *           pass, so the sixteen phases 0x100 to 0x1000 close the circle.
 * @status partial
 * @match 95.42
 * @residual Two residuals, both measured live on this source, whose statement
 *           order, call order, constants and stores match the original.
 *           First, the loop-invariant phase address: the original materialises
 *           &0x1F800004 into $a1 and copies it into $s3 at loop entry, then
 *           derives the tint base as $s2 = $s3 - 4. Every shape tried here
 *           makes the compiler materialise that address straight into $s3. A
 *           second phase pointer, a seed pointer for the pre-loop store, a
 *           constant-address pre-loop store, a declaration-time initialiser and
 *           a phase derived from the tint base were each measured and none
 *           reproduces the copy. Second, the loop counter is held in $s2 with
 *           the tint base in $s1. With this `u16` counter the pair lands on the
 *           original registers but the comparison then costs an `andi` before
 *           its `sltiu` where the original has the plain `slti $s1,16`, so the
 *           original counter is signed and this narrowing is a frontier:
 *           measured 146 of 153 instructions and 608 to 612 bytes as committed,
 *           against 133 of 152 instructions with `s32 i` (which keeps the plain
 *           `slti` but takes the other register pair).
 */
void drawTranslucentFanOfSixteenGouraudTrianglesAroundWorkHalfwords2EAnd30RadiusSixteenTintedByWorkByte9BmagicMagic17203_801EF7CC(
    void) {
  u8 *work;
  POLY_G3 *prim;
  u16 *cells;
  s16 *phase;
  u16 centreX;
  u16 centreY;
  s32 graph;
  u16 i;

  if (GetGraphType() == 1) {
    graph = 0xA5;
  } else if (GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);

  phase = (s16 *)&D_1F800004;
  *phase = 0;
  SPAD_REF(u16, 0x00) = 0x10;

  work = g_battle_work;
  centreX = *(u16 *)(work + 0x2E);
  centreY = *(u16 *)(work + 0x30);
  SPAD_REF(u16, 0x06) = work[9] * 12;
  SPAD_REF(u16, 0x08) = work[9] * 2;
  SPAD_REF(u16, 0x0A) = work[9] * 2;
  SPAD_REF(u16, 0x0C) = work[9] * 14;

  i = 0;
  cells = (u16 *)(phase - 2);
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = centreX;
    prim->y0 = centreY;

    i++;
    prim->x1 = centreX + ((func_801782FC(*phase) * phase[-2]) >> 12);
    prim->y1 = centreY + ((func_801783C8(*phase) * phase[-2]) >> 12);
    *phase = i << 8;
    prim->x2 = centreX + ((func_801782FC(*phase) * phase[-2]) >> 12);
    prim->y2 = centreY + ((func_801783C8(*phase) * phase[-2]) >> 12);

    prim->r0 = cells[3];
    prim->g0 = cells[3];
    prim->b0 = cells[3];
    prim->r1 = cells[4];
    prim->g1 = cells[5];
    prim->b1 = cells[6];
    prim->r2 = cells[4];
    prim->g2 = cells[5];
    prim->b2 = cells[6];

    GpuAppendPrim(3, 0x1C);
  } while (i < 0x10);
}
