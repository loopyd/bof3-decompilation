#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <stdlib.h>

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original re-reads the cell with its own lui/lw pair at every use,
 * so it is read once per consumer below. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Vertical cell of the band builder. The original seeds it with half the
 * caller's base angle, keeps a pointer to it materialised once for the seed
 * product, and reaches it as the seventh halfword below the phase cell from
 * inside the pass loop, always through a signed halfword load. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Tint pair of the ribbon edge: fifteen and thirteen times work byte 0x0A. The
 * original stores both through their literal addresses in the seed block and
 * reads them back inside the pass loop through the phase-cell pointer at
 * displacements -4 and -2, as unsigned halfwords, once per colour byte. */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Phase cell of the band builder: the low nibble of work byte 0x0B shifted left
 * eight, advanced by the pass index. The original holds a pointer to it in a
 * saved register across the whole loop and passes it to the phase sampler as a
 * signed halfword. */
extern s16 D_1F80000E; /* @source 0x1F80000E @kind data */

/* Band block of the scratchpad quad frame at 0x1F800024, with the first band top
 * at 0x1F800016 and the three further top halfwords the pass loop shifts between
 * quads. The original keeps the band base in a saved register and derives the
 * leading vertex from it with one `addiu -16`, so the block is reached relative
 * to the band below; the shifted tops keep their own literal addresses. The
 * cleared halfword 0x1F800018 is written through its literal address in the seed
 * block and relative to the band afterwards. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */
extern u16 D_1F800016; /* @source 0x1F800016 @kind data */
extern u16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern u16 D_1F800026; /* @source 0x1F800026 @kind data */
extern u16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern u16 D_1F800018; /* @source 0x1F800018 @kind data */

/* Shared grid-cell primitive linker: it resolves the record cell word pair,
 * links a primitive at g_PrimCursor with the given kind and advances the cursor
 * by the given size class, which is why this body links one 0xC-byte packet
 * before the pass loop and then one 0x24-byte POLY_G4 packet per quad. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared phase sampler: it returns the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below. */
s32 func_801782FC(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801F14D0
 * @behavior Tinted four-layer quad ribbon of the overlay work object published
 *           at the pointer cell 0x1F800044, the draw the work byte 2 step
 *           handler at 0x801F10E8 calls three times per frame with work byte
 *           0xB raised by four before each call. It takes the layer thickness
 *           in arg0, the base vertical position in arg1 and the jitter mask in
 *           arg2, returns nothing and reads or writes no state outside the
 *           scratchpad and the packets it links. The seed block stores half of
 *           arg1 in 0x1F800000, the low nibble of work byte 0x0B shifted left
 *           eight in the phase cell 0x1F80000E, fifteen and thirteen times work
 *           byte 0x0A in the tint pair 0x1F80000A and 0x1F80000C, clears
 *           0x1F800018 and samples the seed phase through func_801782FC times
 *           the vertical cell shifted right twelve into 0x1F800016 as the first
 *           band top. The draw-mode tpage argument is 0xA5 when GetGraphType
 *           returns 1 or 2, the second call made only when the first answers
 *           something else, and 0x35 otherwise; SetDrawMode then takes dfe 0,
 *           dtd 1 and a null RECT at g_PrimCursor, and one 0xC-byte packet is
 *           linked at the record words 0x34 and 0x38 with kind 2. Each of the
 *           seventeen passes advances the phase cell by the pass index, re-seeds
 *           the vertical cell with arg1 plus or minus a random fraction of arg2,
 *           copies the previous band top and depth into the third vertex pair
 *           with the fourth pair a thickness lower, and rebuilds the first pair
 *           from the new phase sample at the running depth -(index << 6) with its
 *           second vertex a thickness lower. The pass then emits four
 *           translucent Gouraud quads from that one scratchpad frame: the first
 *           pair of vertices takes the fifteen-times tint in all three channels
 *           and the second pair the thirteen-times tint with a 1 green, the four
 *           band tops then move down a thickness, up twice and up a third time
 *           between the remaining three quads, whose colours run from the
 *           thirteen-times tint through white to the fifteen-times tint, and on
 *           the first pass every trailing vertex pair is white. Each quad is
 *           projected with RotTransPers4 through the primitive vertex pairs at
 *           0x08, 0x10, 0x18 and 0x20 and two frame words as its depth and flag
 *           results, then linked as its own POLY_G4 packet with kind 2. The pass
 *           closes by lowering the band top by twice the thickness. Frame 0x60
 *           keeps $ra, $fp, all six $s registers and the six outgoing stack words
 *           of RotTransPers4 with its two results.
 * @status partial
 * @match 72.41
 * @residual The first difference is the frame: this build reserves 0x68 where
 *           the original reserves 0x60. The body matches instruction for
 *           instruction apart from one allocator class: the compiler homes the
 *           jitter argument to the stack instead of keeping it in $fp and
 *           rematerialises the constant 1 per colour store instead of holding
 *           it in a saved register, while it keeps the two frame words whose
 *           addresses RotTransPers4 returns its depth and flag in in saved
 *           registers instead of rebuilding them per call. Measured clean-C
 *           levers that did not change that choice: hoisting the frame words
 *           out of the pass loop, declaring one pair per projection, the array
 *           form `&spare[0]`/`&spare[1]`, `s32` and `volatile` views, an
 *           explicit constant-one local, a jitter local, deriving the leading
 *           vertex and the whole band from their literal addresses, removing
 *           the tint-pair pointer, and reordering the preheader to the original
 *           instruction order. */
void drawJitteredFourLayerQuadRibbonOfSeventeenSlicesSteppingDepthBySixtyFourTintedByWorkByteATimesFifteenWithShadeThirteenBmagicMagic15103_801F14D0(
    s32 arg0, u16 arg1, s32 arg2) {
  s16 *offset;
  u16 *edge;
  u16 *band;
  u16 *rim;
  SVECTOR *vertices;
  POLY_G4 *prim;
  s32 tpage;
  s32 shade;
  u16 prev_y;
  u16 prev_z;
  s32 i;

  offset = &D_1F800000;
  *offset = (s16)arg1 / 2;
  shade = (g_battle_work[0xB] & 0xF) << 8;
  D_1F80000E = shade;
  D_1F80000A = g_battle_work[0xA] * 15;
  D_1F80000C = g_battle_work[0xA] * 13;
  D_1F800016 = (u16)((func_801782FC(shade) * *offset) >> 12);
  D_1F800018 = 0;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }

  edge = &D_1F80000E;
  band = (u16 *)&D_1F800024;
  vertices = (SVECTOR *)(band - 8);
  i = 1;
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  rim = edge - 1;
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                2, 0xC);

  for (i = 1; i < 0x12; i++) {
    long depth;
    long flag;

    shade = ((g_battle_work[0xB] + i) & 0xF) << 8;
    edge[0] = shade;
    if (rand() & 1) {
      edge[-7] = arg1 + (rand() & arg2);
    } else {
      edge[-7] = arg1 - (rand() & arg2);
    }

    prev_y = band[-7];
    prev_z = band[-6];
    band[0] = 0;
    band[4] = 0;
    band[-8] = 0;
    band[1] = prev_y;
    band[2] = prev_z;
    band[5] = prev_y - arg0;
    band[6] = prev_z;
    band[-4] = 0;
    band[-6] = (u16)(-(i << 6));
    band[-2] = (u16)(-(i << 6));
    band[-7] =
        (u16)((func_801782FC(*(s16 *)edge) * *(s16 *)(edge - 7)) >> 12);
    band[-3] = band[-7] - arg0;

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->r0 = edge[-2];
    prim->g0 = edge[-2];
    prim->b0 = edge[-2];
    prim->r1 = edge[-1];
    prim->g1 = 1;
    prim->b1 = edge[-1];
    if (i == 1) {
      prim->r2 = i;
      prim->g2 = i;
      prim->b2 = i;
      prim->r3 = i;
      prim->g3 = i;
      prim->b3 = i;
    } else {
      prim->r2 = edge[-2];
      prim->g2 = edge[-2];
      prim->b2 = edge[-2];
      prim->r3 = edge[-1];
      prim->g3 = 1;
      prim->b3 = edge[-1];
    }

    RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    D_1F800016 = D_1F800016 - arg0;
    D_1F80001E = D_1F80001E - arg0;
    D_1F800026 = D_1F800026 - arg0;
    D_1F80002E = D_1F80002E - arg0;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->r0 = rim[0];
    prim->g0 = 1;
    prim->b0 = rim[0];
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    if (i == 1) {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    } else {
      prim->r2 = rim[0];
      prim->g2 = 1;
      prim->b2 = rim[0];
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    }

    RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    D_1F800016 = D_1F800016 + (s16)arg0 * 2;
    D_1F80001E = D_1F80001E + (s16)arg0 * 2;
    D_1F800026 = D_1F800026 + (s16)arg0 * 2;
    D_1F80002E = D_1F80002E + (s16)arg0 * 2;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->r0 = rim[0];
    prim->g0 = 1;
    prim->b0 = rim[0];
    prim->r1 = rim[-1];
    prim->g1 = rim[-1];
    prim->b1 = rim[-1];
    if (i == 1) {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    } else {
      prim->r2 = rim[0];
      prim->g2 = 1;
      prim->b2 = rim[0];
      prim->r3 = D_1F80000A;
      prim->g3 = D_1F80000A;
      prim->b3 = D_1F80000A;
    }

    RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    D_1F800016 = D_1F800016 + arg0;
    D_1F80001E = D_1F80001E + arg0;
    D_1F800026 = D_1F800026 + arg0;
    D_1F80002E = D_1F80002E + arg0;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = rim[0];
    prim->g1 = 1;
    prim->b1 = rim[0];
    if (i == 1) {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    } else {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = rim[0];
      prim->g3 = 1;
      prim->b3 = rim[0];
    }

    RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x24);

    D_1F800016 = D_1F800016 - (s16)arg0 * 2;
  }
}
