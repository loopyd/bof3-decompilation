#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. This body never keeps the pointer live: every angle, size
 * and count selector re-reads the cell, because the primitive stores and the
 * sampler calls between the reads may alias it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad ribbon block at 0x1F800000, the eight halfwords this body drives.
 * The original holds one base register at the 0x1F80000C cell for the whole
 * loop, so the cells below it become the negative displacements 0x0($s1) down
 * to -0xC($s1) and the two it addresses most, 0x1F80000C and 0x1F80000E, are the
 * non-negative ones; a second base is formed once at 0x1F800000 and a third at
 * 0x1F800002 for the two width cells. The original re-reads the block instead of
 * keeping its cells in registers, and this body reproduces that without a
 * volatile qualifier: the cursor primitive is not known to be distinct from the
 * block, so each colour store through it forces the shade cell back in, which is
 * why the shade is read twelve times for twelve colour bytes with no call
 * between them, and the sampler calls do the same for the width, angle and point
 * cells. The load widths are the widths the original picks: the two width cells
 * load
 * unsigned for the jitter sums and signed for the scale products, so those
 * products carry an explicit (s16) cast; the angle turn is signed throughout;
 * the shade loads unsigned for the plain colour bytes and signed for the
 * quarter division; the four point cells load unsigned. The two point cells are
 * the drifting centre the step advances, and the two prior cells hold the
 * centre the same step started from. */
typedef struct {
  u16 widthPrior; /* 0x0000 */
  u16 widthStep;  /* 0x0002 */
  s16 angleTurn;  /* 0x0004 */
  u16 shade;      /* 0x0006 */
  u16 pointX;     /* 0x0008 */
  u16 pointY;     /* 0x000A */
  u16 priorX;     /* 0x000C */
  u16 priorY;     /* 0x000E */
} BmagicMagic12103RibbonBlock;

extern BmagicMagic12103RibbonBlock D_1F800000; /* @source 0x1F800000 @kind data */

s32 GetGraphType(void);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the >> 12 products below. This body takes the 0x801782FC
 * sample for every x offset and the 0x801783C8 sample for every y offset. */
s32 func_801782FC(s32 angle);
s32 func_801783C8(s32 angle);

/* @source 0x801EF414
 * @behavior Draw routine of the MAGIC121 work object, called once per frame
 *           from the nop delay slot of the jal at 0x801EF024 inside this
 *           overlay's per-frame function func_801EEEAC, and reached by no other
 *           site. It publishes work halfwords 0x2E and 0x30 into the scratchpad
 *           centre pair, seeds a 0..3 jitter accumulator at zero and walks the
 *           centre once per step: work byte 0x0A plus one steps, each step
 *           copying the centre into the prior pair first and then advancing it
 *           by twelve along the twelve-bit work angle at 0x14, the x offset from
 *           the 0x801782FC sample and the y offset from the 0x801783C8 sample.
 *           Each step draws four semi-transparent Gouraud quads through the
 *           primitive cursor and appends each with GpuAppendPrim(3, 0x24), the
 *           POLY_G4 packet size. The two widths of a quad are work byte 0x0B
 *           times the step index and the index minus one, and for the third and
 *           fourth quad work byte 0x0B plus four, each sum displaced by the
 *           0..3 jitter of this step and of the step before; the two wide quads
 *           are built from both width pairs in turn, so they span the wide and
 *           the narrow pair. The first and second quad turn the angle by plus
 *           and minus a quarter turn and form the two side corners from the
 *           prior pair and the drifting pair; the third and fourth quad use the
 *           wide and the narrow width pair for the side corners and the centre
 *           corners respectively. The first and third quad colour the two centre
 *           corners with work byte 0x09 times fifteen and the two side corners
 *           with that value and its signed quarter; the second and fourth quad
 *           swap those two colour runs. The third and fourth quad also write the
 *           four side-corner colour bytes with the literal one. Both ends of the
 *           body repeat the same GetGraphType / SetDrawMode / GpuAppendPrim(3,
 *           0xC) prologue, the draw mode tpage being 0xA5 for graph types one and
 *           two and 0x35 otherwise. Takes no arguments, returns nothing and keeps
 *           no register across steps except the jitter and the step.
 * @status partial
 * @match 81.84
 * @residual Body shape, ownership, C89 spelling and the 0x60 frame with its four
 *           stack cells now agree: the original saves $s0-$s8 and $ra and keeps
 *           the step at 0x18, the jitter at 0x20, the prior jitter at 0x28 and
 *           the narrow-step flag at 0x30, and this seed reproduces that layout.
 *           Five instructions and the register assignment still differ. The
 *           original keeps the sign-extended step in $s5 and the step-minus-one
 *           in $s6 for the whole body, while this spelling re-derives both from
 *           the step cell in the third-quad block, which costs the two
 *           instructions that re-sign-extend the step there and pushes one base
 *           off its register; the same cause leaves the fourth-quad narrow-width
 *           test reading the spilled flag instead of recomputing it. Byte
 *           equality is not established.
 */
void drawFourTranslucentJitteredGouraudQuadsDriftingTwelveAlongWorkAngle14SizedByWorkByteBCountedByWorkByteAAndTintedByWorkByte9ShadeBmagicMagic12103_801EF414(
    void) {
  POLY_G4 *prim;
  s16 n;
  s32 prior;
  s32 turn;
  s32 tpage;
  u16 step;
  u16 jitter;
  u16 priorJitter;
  s32 narrowStep;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(3, 0xC);

  D_1F800000.pointX = FIELD_REF(u16, g_battle_work, 0x2Eu);
  D_1F800000.pointY = FIELD_REF(u16, g_battle_work, 0x30u);
  jitter = 0;
  priorJitter = 0;
  step = 1;

  for (; (s16)step < (s32)FIELD_REF(u8, g_battle_work, 0x0Au) + 1; step++) {
    n = (s16)step;
    D_1F800000.priorX = D_1F800000.pointX;
    D_1F800000.priorY = D_1F800000.pointY;
    D_1F800000.pointX = D_1F800000.pointX + ((func_801782FC(FIELD_REF(s32, g_battle_work, 0x14u)) * 12) >> 12);
    D_1F800000.pointY = D_1F800000.pointY + ((func_801783C8(FIELD_REF(s32, g_battle_work, 0x14u)) * 12) >> 12);

    D_1F800000.widthPrior = D_1F800000.widthStep = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) << 2);
    if (n - 1 < 4) {
      D_1F800000.widthPrior = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) * (n - 1));
    }
    if (n < 4) {
      D_1F800000.widthStep = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) * n);
    }
    D_1F800000.shade = (u16)(FIELD_REF(u8, g_battle_work, 0x09u) * 15);
    priorJitter = jitter;
    jitter = rand() & 3;

    prim = (POLY_G4 *)g_PrimCursor;
    D_1F800000.widthPrior = D_1F800000.widthPrior + priorJitter;
    D_1F800000.widthStep = D_1F800000.widthStep + jitter;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    turn = (FIELD_REF(s32, g_battle_work, 0x14u) + 0x400) & 0xFFF;
    D_1F800000.angleTurn = turn;
    prim->x1 = D_1F800000.priorX + ((func_801782FC(turn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->y1 = D_1F800000.priorY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->x3 = D_1F800000.pointX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->y3 = D_1F800000.pointY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->x0 = D_1F800000.priorX;
    prim->y0 = D_1F800000.priorY;
    prim->x2 = D_1F800000.pointX;
    prim->y2 = D_1F800000.pointY;
    prim->r0 = (u8)D_1F800000.shade;
    prim->g0 = (u8)D_1F800000.shade;
    prim->b0 = (u8)D_1F800000.shade;
    prim->r2 = (u8)D_1F800000.shade;
    prim->g2 = (u8)D_1F800000.shade;
    prim->b2 = (u8)D_1F800000.shade;
    prim->r1 = (u8)D_1F800000.shade;
    prim->g1 = (u8)((s16)D_1F800000.shade / 4);
    prim->b1 = (u8)((s16)D_1F800000.shade / 4);
    prim->r3 = (u8)D_1F800000.shade;
    prim->g3 = (u8)((s16)D_1F800000.shade / 4);
    prim->b3 = (u8)((s16)D_1F800000.shade / 4);
    GpuAppendPrim(3, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    turn = (FIELD_REF(s32, g_battle_work, 0x14u) - 0x400) & 0xFFF;
    D_1F800000.angleTurn = turn;
    prim->x0 = D_1F800000.priorX + ((func_801782FC(turn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->y0 = D_1F800000.priorY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->x2 = D_1F800000.pointX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->y2 = D_1F800000.pointY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->x1 = D_1F800000.priorX;
    prim->y1 = D_1F800000.priorY;
    prim->x3 = D_1F800000.pointX;
    prim->y3 = D_1F800000.pointY;
    prim->r1 = (u8)D_1F800000.shade;
    prim->g1 = (u8)D_1F800000.shade;
    prim->b1 = (u8)D_1F800000.shade;
    prim->r3 = (u8)D_1F800000.shade;
    prim->g3 = (u8)D_1F800000.shade;
    prim->b3 = (u8)D_1F800000.shade;
    prim->r0 = (u8)D_1F800000.shade;
    prim->g0 = (u8)((s16)D_1F800000.shade / 4);
    prim->b0 = (u8)((s16)D_1F800000.shade / 4);
    prim->r2 = (u8)D_1F800000.shade;
    prim->g2 = (u8)((s16)D_1F800000.shade / 4);
    prim->b2 = (u8)((s16)D_1F800000.shade / 4);
    GpuAppendPrim(3, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    turn = (FIELD_REF(s32, g_battle_work, 0x14u) + 0x400) & 0xFFF;
    prior = n - 1;
    D_1F800000.angleTurn = turn;
    D_1F800000.widthPrior = D_1F800000.widthStep = (u16)((FIELD_REF(u8, g_battle_work, 0x0Bu) + 4) << 2);
    narrowStep = prior < 4;
    if (narrowStep) {
      D_1F800000.widthPrior = (u16)((FIELD_REF(u8, g_battle_work, 0x0Bu) + 4) * prior);
    }
    if (n < 4) {
      D_1F800000.widthStep = (u16)((FIELD_REF(u8, g_battle_work, 0x0Bu) + 4) * n);
    }
    D_1F800000.widthPrior = D_1F800000.widthPrior + priorJitter;
    D_1F800000.widthStep = D_1F800000.widthStep + jitter;
    prim->x1 = D_1F800000.priorX + ((func_801782FC(turn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->y1 = D_1F800000.priorY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->x3 = D_1F800000.pointX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->y3 = D_1F800000.pointY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    D_1F800000.widthPrior = D_1F800000.widthStep = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) << 2);
    if (narrowStep) {
      D_1F800000.widthPrior = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) * prior);
    }
    if (n < 4) {
      D_1F800000.widthStep = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) * n);
    }
    D_1F800000.widthPrior = D_1F800000.widthPrior + priorJitter;
    D_1F800000.widthStep = D_1F800000.widthStep + jitter;
    prim->x0 = D_1F800000.priorX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->y0 = D_1F800000.priorY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->x2 = D_1F800000.pointX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->y2 = D_1F800000.pointY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->r0 = (u8)D_1F800000.shade;
    prim->g0 = (u8)((s16)D_1F800000.shade / 4);
    prim->b0 = (u8)((s16)D_1F800000.shade / 4);
    prim->r2 = (u8)D_1F800000.shade;
    prim->g2 = (u8)((s16)D_1F800000.shade / 4);
    prim->b2 = (u8)((s16)D_1F800000.shade / 4);
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r3 = 1;
    prim->g3 = 1;
    prim->b3 = 1;
    GpuAppendPrim(3, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    turn = (FIELD_REF(s32, g_battle_work, 0x14u) - 0x400) & 0xFFF;
    D_1F800000.angleTurn = turn;
    D_1F800000.widthPrior = D_1F800000.widthStep = (u16)((FIELD_REF(u8, g_battle_work, 0x0Bu) + 4) << 2);
    if (narrowStep) {
      D_1F800000.widthPrior = (u16)((FIELD_REF(u8, g_battle_work, 0x0Bu) + 4) * prior);
    }
    if (n < 4) {
      D_1F800000.widthStep = (u16)((FIELD_REF(u8, g_battle_work, 0x0Bu) + 4) * n);
    }
    D_1F800000.widthPrior = D_1F800000.widthPrior + priorJitter;
    D_1F800000.widthStep = D_1F800000.widthStep + jitter;
    prim->x0 = D_1F800000.priorX + ((func_801782FC(turn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->y0 = D_1F800000.priorY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->x2 = D_1F800000.pointX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->y2 = D_1F800000.pointY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    D_1F800000.widthPrior = D_1F800000.widthStep = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) << 2);
    if (narrowStep) {
      D_1F800000.widthPrior = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) * prior);
    }
    if (n < 4) {
      D_1F800000.widthStep = (u16)(FIELD_REF(u8, g_battle_work, 0x0Bu) * n);
    }
    D_1F800000.widthPrior = D_1F800000.widthPrior + priorJitter;
    D_1F800000.widthStep = D_1F800000.widthStep + jitter;
    prim->x1 = D_1F800000.priorX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->y1 = D_1F800000.priorY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthPrior) >> 12);
    prim->x3 = D_1F800000.pointX + ((func_801782FC(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->y3 = D_1F800000.pointY + ((func_801783C8(D_1F800000.angleTurn) * (s16)D_1F800000.widthStep) >> 12);
    prim->r1 = (u8)D_1F800000.shade;
    prim->g1 = (u8)((s16)D_1F800000.shade / 4);
    prim->b1 = (u8)((s16)D_1F800000.shade / 4);
    prim->r3 = (u8)D_1F800000.shade;
    prim->g3 = (u8)((s16)D_1F800000.shade / 4);
    prim->b3 = (u8)((s16)D_1F800000.shade / 4);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    GpuAppendPrim(3, 0x24);
  }

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(3, 0xC);
}
