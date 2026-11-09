#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The original
 * re-materializes the cell with its own lui/lw pair at every use, so the plain
 * named cell suffices; the same-target siblings
 * copyRecordHalfwordPairIntoWorkThenOnWorkByteBFFSubmitBattlerRecordForwardBattleByteRaiseFlag4AndInvokeHelperBmagicMagic09403_801EED60
 * and incrementWorkByte9ThenSetGlobalWorkByteBToFFAndInvokeHelperWhenWorkByte9Is20BmagicMagic09403_801EF0FC
 * bind it the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad band scalar row at 0x1F800000, the three band scalars this body
 * publishes per band and the radius the band opening vertex is scaled by. The
 * original keeps one base register for the three stores and the two opening
 * reads, so the row is one object rather than a folded constant per store. */
typedef struct BmagicMagic09403BandScalars {
  s32 bandRadius; /* 0x0000 */
  s32 bandScale;  /* 0x0004 */
  s32 bandDepth;  /* 0x0008 */
} BmagicMagic09403BandScalars;

extern BmagicMagic09403BandScalars D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad colour word at 0x1F80000C. The original forms its address once
 * per band and keeps it live across the byte stores of the primitive, since
 * those stores may alias the cell and force a fresh read before each. */
extern u32 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Scratchpad vertex triplet at 0x1F800014, 0x1F80001C and 0x1F800024, the three
 * SVECTOR inputs of RotTransPers3. The original materialises the address of the
 * third vector 0x1F800024 and derives the triplet base from it by two vectors,
 * so the base stays a register and the later vectors are its 0x8 and 0x10
 * displacements. The six halfwords the band opening publishes are written
 * through literal constants instead, the absolute `lui $at` plus `sh` form the
 * compiler emits for a direct fixed-address store. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared phase samplers used by every drawing step of this overlay family: both
 * return the sample of a 12-bit phase in the same fixed-point scale as the
 * `>> 12` products below. The body takes the 0x801783C8 sample for every x
 * offset and the 0x801782FC sample for every y offset, as the exact bmagic
 * sibling drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetimeBmagicMagic10603_801EF5F4
 * does. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper, called on the fresh cursor primitive before
 * this body writes its colour bytes. */
void func_8017A97C(void *arg0);

/* Shared cell-primitive linker: it takes the record cell origin pair, the kind
 * and the packet size, links the primitive at g_PrimCursor and advances the
 * cursor by that size, which is why this body links a 0xC-byte packet once per
 * band and one 0x1C-byte packet per triangle. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EF14C
 * @behavior Battle-magic drawing step, the large draw step the overlay
 *           dispatcher at 0x801EEE4C calls between its transform step and its
 *           second draw step; it takes no arguments and returns nothing.
 *           The body runs six bands and eight triangles per band. Before the
 *           bands it seeds three band scalars, a 0x10 radius, a 1 row and a
 *           0x40 span, and keeps a pointer to the scratchpad scalar cells at
 *           0x1F800000 plus a pointer to the three vertex vectors at
 *           0x1F800014. Each band first picks a profile from work byte 1 and
 *           publishes three scalars through the 0x1F800000 pointer: the radius
 *           cell always takes the band radius, the second cell takes the span
 *           (profile 2), the span minus work byte 9 (profile 3) or four times
 *           the row scaled by work byte 9 (every other value), and the third
 *           cell takes the negated five-step ramp of the remaining bands,
 *           scaled by work byte 9 in the default profile and by 16 in
 *           profiles 2 and 3, where profile 3 additionally offsets it by twice
 *           work byte 9. It then publishes the band's opening outer vertex into
 *           the two cells at 0x1F800024 and 0x1F800026, the sample of phase
 *           zero scaled by the radius cell, and clears 0x1F800028.
 *           The inner run takes eight triangles of one spiral band: it samples
 *           the phase helper at its own phase left shifted 8 for the rim
 *           vertex, scaled by the second scalar cell and shifted right 12, with
 *           the third scalar cell as that vertex's depth; copies the previous
 *           band vertex into the middle vertex with depth zero, so neighbouring
 *           triangles meet; and samples the phase helper again at the triangle
 *           counter left shifted 9 for the outer ring vertex, scaled by the
 *           radius cell and shifted right 12, with depth zero. It derives the
 *           output-list cell pair from that ring vertex: x shifted left 9 plus
 *           the work record word 0x34, and y shifted left 9 plus word 0x38. It
 *           then picks the draw-mode tpage argument, 0xA5 when GetGraphType
 *           returns 1 or 2 and 0x35 otherwise, seeds SetDrawMode on
 *           g_PrimCursor with dfe 0 and dtd 1, links the 0xC-byte cell, and per
 *           triangle seeds the fresh cursor primitive through func_8017A97C
 *           and marks it semi-transparent. The colour bytes depend on work byte
 *           1: in profile 3 the tint cell at 0x1F80000C takes the band radius
 *           reduced by a sixteenth of it per halved work byte 9, clamped to one
 *           when that goes negative, the first vertex takes white and both
 *           later vertices take that tint with a blue channel of 0xC1 minus six
 *           times work byte 9; every other profile stores the plain band radius
 *           and gives all three vertices that tint with a blue channel of 0xC0.
 *           It projects the three vectors through RotTransPers3 into the
 *           primitive's three vertex pairs, links the 0x1C-byte packet, steps
 *           the triangle counter and the inner phase by two, and after eight
 *           triangles adds 0x10 to the radius, one to the row and 0x40 to the
 *           span and moves to the next band. The 0x78-byte frame keeps $ra,
 *           $s0 through $s7 and $fp, the fifth draw-mode argument at 0x10 and
 *           the two RotTransPers3 output words at 0x20 and 0x24, and the body
 *           writes no state outside the scratchpad cells and the primitives it
 *           emits.
 *
 *           Shape and naming evidence: no span of any payload under
 *           out/binaries/emi holds this 1288-byte body byte for byte, so the
 *           clone lever had no source and the body was seeded from the original
 *           bytes. The two-helper sampling, the published vertex pairs, the
 *           tpage disjunction over GetGraphType, the 0xC-then-0x1C linker calls
 *           and the white-first-vertex colour run follow the exact siblings
 *           drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetimeBmagicMagic10603_801EF5F4
 *           and
 *           drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairTintedByWorkByteAWithJitteredRadiusBmagicMagic09703_801F083C
 *           of this family, and the transform step of this same target
 *           supplies the record word pair 0x34 and 0x38 as the object position
 *           the cell pair below is offset from.
 * @status partial
 * @match 62.11
 * @residual 200 of 322 instructions match; 1268 bytes versus 1288 original.
 *           The frame now matches 0x78 and the scalar-row address survives in
 *           a saved register. Allocation still differs: the row uses $s7
 *           instead of $fp, the band radius stays in $s3 instead of spilling
 *           to 0x38($sp), and the literal scale-read base spills. The original
 *           uses direct radius reads in the inner loop, whereas this candidate
 *           reads that same word through the scalar-row object. Entry order,
 *           saved-register choices and stack slots remain non-exact.
 */

void drawSixSpiralBandsOfEightTrianglesTintedByWorkByte9WithWorkByte1RadiusProfileBmagicMagic09403_801EF14C(
    void) {
  SVECTOR *vertex;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 radius;
  s32 row;
  s32 span;
  s32 band;
  s32 step;
  u32 *cell;
  s32 x;
  s32 y;
  s32 tpage;
  s32 j;

  vertex = (SVECTOR *)((u16 *)&D_1F800024 - 8);
  radius = 0x10;
  row = 1;
  span = 0x40;
  band = 0;
  do {
    switch (g_battle_work[1]) {
    case 0:
    case 1:
    default:
      D_1F800000.bandRadius = radius;
      D_1F800000.bandScale = (row << 2) * g_battle_work[9];
      D_1F800000.bandDepth = -(((band ^ 7) + 1) * 5 * g_battle_work[9]);
      break;
    case 2:
      D_1F800000.bandRadius = radius;
      D_1F800000.bandScale = span;
      D_1F800000.bandDepth = -(((band ^ 7) + 1) * 5 * 16);
      break;
    case 3:
      D_1F800000.bandRadius = radius;
      D_1F800000.bandScale = span - g_battle_work[9];
      D_1F800000.bandDepth = g_battle_work[9] * 2 - ((band ^ 7) + 1) * 5 * 16;
      break;
    }

    *(u16 *)0x1F800024 = func_801783C8(0) * D_1F800000.bandRadius >> 12;
    *(u16 *)0x1F800026 = func_801782FC(0) * D_1F800000.bandRadius >> 12;
    *(u16 *)0x1F800028 = 0;

    cell = &D_1F80000C;
    j = 1;
    step = 1;
    do {
      vertex[0].vx = func_801783C8(step << 8) * *(s32 *)0x1F800004 >> 12;
      vertex[0].vy = func_801782FC(step << 8) * *(s32 *)0x1F800004 >> 12;
      vertex[0].vz = *(s32 *)0x1F800008;
      vertex[1].vx = vertex[2].vx;
      vertex[1].vy = vertex[2].vy;
      vertex[1].vz = 0;
      vertex[2].vx = func_801783C8(j << 9) * D_1F800000.bandRadius >> 12;
      vertex[2].vy = func_801782FC(j << 9) * D_1F800000.bandRadius >> 12;
      vertex[2].vz = 0;

      x = (vertex[2].vx << 9) + *(s32 *)(g_battle_work + 0x34);
      y = (vertex[2].vy << 9) + *(s32 *)(g_battle_work + 0x38);

      if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0xA5;
      } else {
        tpage = 0x35;
      }
      SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
      func_80155A08(x, y, 2, 0xC);

      prim = (POLY_G3 *)g_PrimCursor;
      func_8017A97C(prim);
      SetSemiTrans(prim, 1);

      if (g_battle_work[1] == 3) {
        *cell = radius;
        tpage = radius - radius / 16 * (g_battle_work[9] >> 1);
        if (tpage < 0) {
          *cell = 1;
        } else {
          *cell = tpage;
        }
        prim->r0 = 1;
        prim->g0 = 1;
        prim->b0 = 1;
        prim->r1 = *cell;
        prim->g1 = *cell;
        prim->b1 = -0x3F - g_battle_work[9] * 6;
        prim->r2 = prim->r1;
        prim->g2 = prim->g1;
        prim->b2 = prim->b1;
      } else {
        *cell = radius;
        prim->r0 = 1;
        prim->g0 = 1;
        prim->b0 = 1;
        prim->r1 = *cell;
        prim->g1 = *cell;
        prim->b1 = 0xC0;
        prim->r2 = *cell;
        prim->g2 = *cell;
        prim->b2 = 0xC0;
      }

      RotTransPers3(vertex, vertex + 1, vertex + 2, (long *)&prim->x0,
                    (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);
      func_80155A08(x, y, 2, 0x1C);
      j++;
      step += 2;
    } while (j < 9);

    radius += 0x10;
    row += 1;
    span += 0x40;
    band++;
  } while (band < 6);
}
