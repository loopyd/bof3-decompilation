#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. This body reads the cell
 * once, after the opening draw-mode packet is linked, and reaches the work
 * record through that one load: the two screen halfwords at 0x2E and 0x30, the
 * unsigned byte 9 and the unsigned byte 0xB reads that follow all use the same
 * register. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000 and phase cell at 0x1F800004. The body
 * publishes the radius 0x18 once before the loop and the phase 0 at the same
 * time, then reads the radius four times per pass and reads, stores and
 * re-reads the phase around the second vertex pair. The original materialises
 * the phase cell address once and keeps it live across the loop, so the pair
 * stays two named rows. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* The two interleaved coefficient bytes of the fan tint, at 0x801F0B9C and
 * 0x801F0B9D inside the fourteen-word code-pointer run this overlay stores at
 * 0x801F0B74. Bytes 0x9C to 0xAB of the payload hold eight byte pairs
 * (05 08, 05 07, 04 06, 04 05, 03 04, 03 03, 02 02, 01 01), so each row here is
 * a byte array the work record's byte 0xB scales by two before indexing. The
 * externs are declared at the indexed byte and the run stays the single
 * D_801F0B74 row in this target's map; the generated undefined_syms_auto.txt
 * rows for 0x801F0B9C and 0x801F0B9D already name these bytes. */
extern u8 D_801F0B9C[]; /* @source 0x801F0B9C @kind data */
extern u8 D_801F0B9D[]; /* @source 0x801F0B9D @kind data */

/* Shared phase samplers of the main executable, bound by this target's
 * generated undefined_funcs_auto.txt. Both take a 12-bit phase and return a
 * fixed-point sample in the scale the `>> 12` products below divide by; the
 * exact bmagic fan siblings declare the same pair with the same prototypes. */
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

/* @source 0x801F0908
 * @behavior Overlay draw step of this payload with one caller and no pointer
 *           word naming it: the dispatch step at 0x801EF518 calls it with a
 *           bare `jal` from the arm it takes when the work record's bytes 0 and
 *           2 are both non-zero, after the shared helper at 0x800AF764. It
 *           reads no incoming register, so it takes no arguments and returns
 *           nothing.
 *           It picks the draw-mode texture page, 0xA5 when GetGraphType returns
 *           1 or 2 and 0x35 otherwise, calling the query once per test with no
 *           cached result, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links that
 *           0xC-byte packet under ordering-table slot 3. It then publishes the
 *           fan state in scratchpad, a radius of 0x18 at 0x1F800000, a phase of
 *           0 at 0x1F800004 and four tint cells scaled from the work record: a
 *           first product of unsigned byte 9 and unsigned byte 0xB stored at
 *           0x1F800006 that this body never reads, the tints 0x1F800008 and
 *           0x1F80000C as byte 9 scaled by the coefficient pair at 0x801F0B9C
 *           and 0x801F0B9D selected by byte 0xB scaled by two, and byte 9 alone
 *           at 0x1F80000A.
 *           It then emits sixteen translucent Gouraud triangles into
 *           ordering-table slot 3 at 0x1C bytes each. Every triangle shares the
 *           screen centre at unsigned halfwords 0x2E and 0x30 of the work
 *           record, a grey core colour of the three stored cells 0x1F800008,
 *           0x1F80000A and 0x1F80000C, and two rim vertices held at colour one;
 *           the first rim vertex pair is sampled at the phase published on the
 *           previous pass and the second at the phase advanced by 0x100 per
 *           pass, so the sixteen phases 0x100 to 0x1000 close the circle.
 * @status partial
 * @match 96.13
 * @residual Every word except the phase pointer's register agrees with the
 *           original: 149 of 155 instructions, 620 to 616 bytes, and the four
 *           mismatching words and the one missing word share a single root
 *           cause. The original materialises the address 0x1F800004 into $a1
 *           for the pre-loop reset store, keeps it there across the tint
 *           computation and copies it into $s2 at loop entry, where
 *           $s6 = $s2 - 4 derives the tint base and the phase store follows the
 *           work load. This body makes every shape materialise the address
 *           straight into $s2, which drops the copy and shifts the loop's
 *           backward branch one instruction earlier. Measured live: the seed
 *           statement order (149/155), the work load hoisted before the phase
 *           store (149/155), a constant-address pre-loop store with the pointer
 *           assigned at loop entry (137/155), that same pair plus the hoisted
 *           work load (137/155), and a dedicated seed pointer for the reset
 *           with the loop pointer formed later (139/155). The exact bmagic fan
 *           sibling 0x801EF7CC carries the identical $a1-then-copy signature,
 *           so the split is an allocation frontier of this overlay's source
 *           idiom rather than a spelling gap.
 */
void drawTranslucentFanOfSixteenGouraudTrianglesAroundWorkHalfwords2EAnd30RadiusTwentyFourTintedByWorkByte9CoefficientsOfWorkByte0BBmagicMagic17203_801F0908(
    void) {
  u8 *work;
  POLY_G3 *prim;
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
  *phase = 0;
  SPAD_REF(u16, 0x00) = 0x18;

  work = g_battle_work;
  centreX = *(u16 *)(work + 0x2E);
  centreY = *(u16 *)(work + 0x30);
  SPAD_REF(u16, 0x06) = work[9] * work[0xB];
  SPAD_REF(u16, 0x08) = work[9] * D_801F0B9C[work[0xB] * 2];
  SPAD_REF(u16, 0x0A) = work[9];
  SPAD_REF(u16, 0x0C) = work[9] * D_801F0B9D[work[0xB] * 2];

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

    prim->r0 = cells[4];
    prim->g0 = cells[5];
    prim->b0 = cells[6];
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;

    GpuAppendPrim(3, 0x1C);
  } while (i < 0x10);
}
