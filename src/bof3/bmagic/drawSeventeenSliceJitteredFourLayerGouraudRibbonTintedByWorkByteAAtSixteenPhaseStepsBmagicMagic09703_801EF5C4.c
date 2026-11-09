#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);

/* Scratchpad work-record pointer cell at 0x1F800044. The original re-reads the
 * cell with its own lui/lw pair at every use, so the cell is bound by name and
 * read once per consumer, as in the exact bmagic sibling
 * drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairTintedByWorkByteAWithJitteredRadiusBmagicMagic09703_801F083C. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad scalar cells of the band builder. The original reaches every one
 * of them through its own %hi/%lo pair per access, the form it uses for a named
 * fixed address that no source variable holds, so each is a separate named row
 * here. The thickness pair at 0x1F80000A and 0x1F80000C and the band block at
 * 0x1F800024 are the two exceptions: the original keeps one register base per
 * group and addresses those cells as offsets of it. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F800016; /* @source 0x1F800016 @kind data */
extern u16 D_1F800018; /* @source 0x1F800018 @kind data */
extern u16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern u16 D_1F800026; /* @source 0x1F800026 @kind data */
extern u16 D_1F80002E; /* @source 0x1F80002E @kind data */

/* Scratchpad vertex quad at 0x1F800014 and its third vertex at 0x1F800024: the
 * four SVECTOR inputs of RotTransPers4. Each address is materialised from its
 * own %hi/%lo pair at every projection, except 0x1F800024, which is the band
 * base the original keeps live in a register. */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */
extern u16 D_1F80002C; /* @source 0x1F80002C @kind data */

/* Shared grid-cell primitive linker: it resolves the record cell word pair,
 * links a primitive at g_PrimCursor with the given kind and advances the
 * cursor by the given size class, which is why this body links a 0xC-byte
 * packet once and then one 0x24-byte POLY_G4 packet per quad. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared phase sampler: it returns the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below. */
s32 func_801782FC(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EF5C4
 * @behavior Battle-magic drawing step reached from the overlay dispatcher, it
 *           takes the thickness in arg0, the base vertical position in arg1
 *           and the vertical jitter mask in arg2. It seeds the scratchpad
 *           scalar block: the vertical cell 0x1F800000 with arg1, the phase
 *           cell 0x1F800002 with the low nibble of work byte 0x0B shifted left
 *           eight, the colour cells 0x1F800004 and 0x1F800006 with fifteen and
 *           two times work byte 0x0A, and the thickness cell 0x1F80000C with
 *           arg0 minus the low two bits of rand. It then samples the seed
 *           phase through func_801782FC, clears 0x1F800018 and stores the
 *           product with the vertical cell shifted right twelve into
 *           0x1F800016 as the first band top, picks the draw-mode tpage
 *           argument 0xA5 when GetGraphType returns 1 or 2 and 0x35 otherwise,
 *           and links the 0xC-byte draw-mode packet at the record words 0x34
 *           and 0x38 with kind 2. Seventeen bands follow: each advances the
 *           phase by the band index, re-seeds the vertical cell from arg1 plus
 *           or minus a random fraction of arg2, moves the previous thickness
 *           into 0x1F80000A, seeds the new thickness from arg0 minus the low
 *           two bits of rand, builds the four-vertex scratchpad quad from the
 *           previous band top and the previous thickness with the far edge at
 *           the running depth, projects it with RotTransPers4 and links one
 *           0x24-byte POLY_G4 packet, then emits three more packets from the
 *           same quad after shifting the four band tops by the new and previous
 *           thickness, shifting them up by twice and three times and then by
 *           one and two times, and finally lowers the top by three thicknesses
 *           and the running depth by 0x40. Vertex colours are the thick-rim
 *           shades fifteen and two times work byte 0x0A, white, and, on the
 *           first band, white for the two trailing vertices of every quad.
 * @status partial
 * @match 73.30
 * @residual The frame is 0x70 rather than 0x80. The band base, counter and
 *           constant one occupy different saved registers, and the pair base
 *           folds into one relocation addend rather than two additions.
 *           Explicit old-band values remove the setup reloads. Projection
 *           output pointers are materialised in the loop, and the running
 *           top occupies the original saved register, but allocation and
 *           scheduling differences remain. The source is review-pending.
 */
void drawSeventeenSliceJitteredFourLayerGouraudRibbonTintedByWorkByteAAtSixteenPhaseStepsBmagicMagic09703_801EF5C4(
    s16 arg0, s16 arg1, s16 arg2) {
  u16 *pair;
  u16 *band;
  u16 *top;
  u16 *vertical;
  POLY_G4 *prim;
  s32 tpage;
  s32 i;
  s32 z;
  s32 sample;
  u16 old_top;
  u16 old_depth;
  long depth;
  long flag;
  long *depth_out;
  long *flag_out;

  vertical = &D_1F800000;
  *vertical = arg1;
  D_1F800002 = (g_battle_work[0xB] & 0xF) << 8;
  D_1F800004 = g_battle_work[0xA] * 15;
  D_1F800006 = g_battle_work[0xA] * 2;
  D_1F80000C = arg0 - (rand() & 3);
  sample = func_801782FC(*(s16 *)&D_1F800002);
  D_1F800018 = 0;
  D_1F800016 = (sample * *(s16 *)vertical) >> 12;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }

  i = 1;
  pair = &D_1F800002 + 4;
  band = &D_1F800024;
  top = band - 7;
  z = -0x40;
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                FIELD_REF(s32, g_battle_work, 0x38u), 2, 0xC);

  do {
    D_1F800002 = ((g_battle_work[0xB] + i) & 0xF) << 8;
    if (rand() & 1) {
      D_1F800000 = arg1 + (rand() & arg2);
    } else {
      D_1F800000 = arg1 - (rand() & arg2);
    }
    pair[0] = pair[1];
    pair[1] = arg0 - (rand() & 3);
    old_top = band[-7];
    old_depth = band[-6];
    band[1] = old_top;
    band[0] = 0;
    band[4] = 0;
    band[-8] = 0;
    band[2] = old_depth;
    band[5] = old_top - pair[0];
    band[6] = old_depth;

    sample = func_801782FC(*(s16 *)(pair - 4));
    prim = (POLY_G4 *)g_PrimCursor;
    band[-6] = z;
    band[-4] = 0;
    band[-2] = z;
    band[-7] = (sample * *(s16 *)(pair - 5)) >> 12;
    band[-3] = band[-7] - pair[1];
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->r0 = pair[-3];
    prim->g0 = pair[-3];
    prim->b0 = pair[-3];
    prim->r1 = pair[-2];
    prim->g1 = pair[-2];
    prim->b1 = pair[-3];
    prim->r2 = pair[-3];
    prim->g2 = pair[-3];
    prim->b2 = pair[-3];
    prim->r3 = pair[-2];
    prim->g3 = pair[-2];
    prim->b3 = pair[-3];
    if (i == 1) {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    }

    depth_out = &depth;
    flag_out = &flag;
    RotTransPers4((SVECTOR *)&D_1F800014, (SVECTOR *)&D_1F80001C,
                  (SVECTOR *)band, (SVECTOR *)&D_1F80002C,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, depth_out, flag_out);
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    top[0] = top[0] - pair[1];
    D_1F80001E = D_1F80001E - (s16)pair[1] * 2;
    D_1F800026 = D_1F800026 - pair[0];
    D_1F80002E = D_1F80002E - (s16)pair[0] * 2;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->r0 = pair[-2];
    prim->g0 = pair[-2];
    prim->b0 = pair[-3];
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = pair[-2];
    prim->g2 = pair[-2];
    prim->b2 = pair[-3];
    prim->r3 = 1;
    prim->g3 = 1;
    prim->b3 = 1;
    if (i == 1) {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    }

    depth_out = &depth;
    flag_out = &flag;
    RotTransPers4((SVECTOR *)&D_1F800014, (SVECTOR *)&D_1F80001C,
                  (SVECTOR *)band, (SVECTOR *)&D_1F80002C,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, depth_out, flag_out);
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    top[0] = top[0] + *(s16 *)&D_1F80000C * 2;
    D_1F80001E = D_1F80001E + *(s16 *)&D_1F80000C * 3;
    D_1F800026 = D_1F800026 + *(s16 *)pair * 2;
    D_1F80002E = D_1F80002E + *(s16 *)pair * 3;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = pair[-2];
    prim->g1 = pair[-2];
    prim->b1 = pair[-3];
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    prim->r3 = pair[-2];
    prim->g3 = pair[-2];
    prim->b3 = pair[-3];
    if (i == 1) {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    }

    depth_out = &depth;
    flag_out = &flag;
    RotTransPers4((SVECTOR *)&D_1F800014, (SVECTOR *)&D_1F80001C,
                  (SVECTOR *)band, (SVECTOR *)&D_1F80002C,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, depth_out, flag_out);
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    top[0] = top[0] + *(s16 *)&D_1F80000C * 2;
    D_1F80001E = D_1F80001E + *(s16 *)&D_1F80000C;
    D_1F80002E = D_1F80002E + *(s16 *)pair;
    D_1F800026 = D_1F800026 + *(s16 *)pair * 2;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = pair[-2];
    prim->g1 = pair[-2];
    prim->b1 = pair[-3];
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    prim->r3 = pair[-2];
    prim->g3 = pair[-2];
    prim->b3 = pair[-3];
    if (i == 1) {
      prim->r2 = 1;
      prim->g2 = 1;
      prim->b2 = 1;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
    }

    depth_out = &depth;
    flag_out = &flag;
    RotTransPers4((SVECTOR *)&D_1F800014, (SVECTOR *)&D_1F80001C,
                  (SVECTOR *)band, (SVECTOR *)&D_1F80002C,
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, depth_out, flag_out);
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x24);

    z -= 0x40;
    i++;
    top[0] = top[0] - *(s16 *)&D_1F80000C * 3;
  } while (i < 0x12);
}
