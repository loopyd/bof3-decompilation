#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. This body reads the cell
 * once, between the draw-mode packet and the ring-state stores, and reaches
 * the work record through that one load: the three tint reads of unsigned byte
 * 9 and the two centre halfwords at 0x2E and 0x30 all use the same register. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000, the ring's inner radius 0x10, and
 * phase cell at 0x1F800004. The body publishes the inner radius once before
 * the loop and materialises the phase cell address once, carrying it into the
 * loop where it reads, stores and re-reads the phase around the second vertex
 * pair. The outer radius at 0x1F800002 and the three tint cells at 0x1F800008,
 * 0x1F80000A and 0x1F80000C are reached from a base one cell below the phase,
 * so the radius pair stays two live named rows and the loop works through
 * them. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Shared phase samplers of the main executable, bound by this target's
 * generated undefined_funcs_auto.txt. Both take a 12-bit phase and return a
 * fixed-point sample in the scale the `>> 12` products below divide by; the
 * bmagic fan siblings declare the same pair with the same prototypes. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* @source 0x801EFA2C
 * @behavior Overlay draw step of this payload. The overlay render body at
 *           0x801EEE7C calls it with a bare `jal`, and it reads no incoming
 *           register, so it takes no arguments and returns nothing.
 *           It picks the draw-mode texture page, 0xA5 when GetGraphType
 *           returns 1 or 2 and 0x35 otherwise, calling the query once per test
 *           with no cached result, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links that
 *           0xC-byte packet under ordering-table slot 3. It then publishes the
 *           ring state in scratchpad: the phase 0 at 0x1F800004, the inner
 *           radius 0x10 at 0x1F800000, the outer radius 0x18 at 0x1F800002 and
 *           three tint cells at 0x1F800008, 0x1F80000A and 0x1F80000C holding
 *           work byte 9, work byte 9 again and fourteen times that byte.
 *           It emits sixteen translucent Gouraud quads into ordering-table slot
 *           3 at 0x24 bytes each, all four vertices measured from the work
 *           record's unsigned halfwords 0x2E and 0x30. Vertices 0 and 2 sit at
 *           the inner and outer radius on the phase the previous pass
 *           published, vertices 1 and 3 at the same two radii on the phase
 *           advanced by 0x100, so the sixteen passes 0x100 to 0x1000 close the
 *           ring. The inner pair, vertices 0 and 1, takes the colour triple
 *           work byte 9, work byte 9 and fourteen times that byte from the
 *           three cells; the outer pair, vertices 2 and 3, is the constant 1 in
 *           every channel.
 * @status partial
 * @match 73.37
 * @residual One residual, measured live on this source, whose statement order,
 *           call order, constants, stores and loop bounds match the original.
 *           The six constant-1 channel stores at primitive bytes 0x14, 0x15,
 *           0x16, 0x1C, 0x1D and 0x1E are the only loop-invariant value the
 *           pass loop materialises. This compiler moves that value out of the
 *           loop, which the original does not: the loop optimizer moves a
 *           movable when threshold (25 with a call in the loop) times savings
 *           (one set) times lifetime (six instructions of live range) reaches
 *           the 137-instruction loop body, so 150 against 137 moves it. The
 *           moved value then holds the callee-saved register $s6 for the whole
 *           pass and pushes the phase pointer, the cell base and the counter
 *           from the original's $s2, $s3 and $s1 to $s1, $s2 and $s3, which is
 *           why the body measures 135 of 182 instructions. Measured 728 to 736
 *           bytes; the eight extra bytes are the $s6 save and restore pair.
 *           Variants measured and rejected: a signed 16-bit counter, an
 *           unsigned counter, the counter chained and initialised earlier,
 *           chained and single-statement channel stores, a channel value held
 *           in a local, a shared sampled-phase local, and one or two vertex
 *           temporaries per corner. The last of those drops the loop below the
 *           move threshold only by adding sixteen hidden copies, which no
 *           evidenced statement justifies, and it measures four bytes short.
 */
void drawTranslucentGouraudQuadRingBetweenRadiiSixteenAndTwentyFourAtSixteenPhaseStepsAroundWorkHalfwords2EAnd30TintedByWorkByte9BmagicMagic17203_801EFA2C(
    void) {
  u8 *work;
  POLY_G4 *prim;
  u16 *cells;
  s16 *phase;
  u16 centreX;
  u16 centreY;
  s32 graph;
  s32 i;

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
  work = g_battle_work;
  *phase = 0;
  SPAD_REF(u16, 0x00) = 0x10;
  SPAD_REF(u16, 0x02) = 0x18;
  SPAD_REF(u16, 0x08) = work[9];
  SPAD_REF(u16, 0x0A) = work[9];
  SPAD_REF(u16, 0x0C) = work[9] * 14;
  centreX = *(u16 *)(work + 0x2E);
  centreY = *(u16 *)(work + 0x30);

  i = 0;
  cells = (u16 *)(phase - 2);
  do {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    i++;
    prim->x0 = centreX + ((func_801782FC(*phase) * phase[-2]) >> 12);
    prim->y0 = centreY + ((func_801783C8(*phase) * phase[-2]) >> 12);
    prim->x2 = centreX + ((func_801782FC(*phase) * ((s16 *)cells)[1]) >> 12);
    prim->y2 = centreY + ((func_801783C8(*phase) * ((s16 *)cells)[1]) >> 12);
    *phase = i << 8;
    prim->x1 = centreX + ((func_801782FC(*phase) * phase[-2]) >> 12);
    prim->y1 = centreY + ((func_801783C8(*phase) * phase[-2]) >> 12);
    prim->x3 = centreX + ((func_801782FC(*phase) * ((s16 *)cells)[1]) >> 12);
    prim->y3 = centreY + ((func_801783C8(*phase) * ((s16 *)cells)[1]) >> 12);

    prim->r0 = cells[4];
    prim->g0 = cells[5];
    prim->b0 = cells[6];
    prim->r1 = cells[4];
    prim->g1 = cells[5];
    prim->b1 = cells[6];
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    prim->r3 = 1;
    prim->g3 = 1;
    prim->b3 = 1;

    GpuAppendPrim(3, 0x24);
  } while (i < 0x10);
}
