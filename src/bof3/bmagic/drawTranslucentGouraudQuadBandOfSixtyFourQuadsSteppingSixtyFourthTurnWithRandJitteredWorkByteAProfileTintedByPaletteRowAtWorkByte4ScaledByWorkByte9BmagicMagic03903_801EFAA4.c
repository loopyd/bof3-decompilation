#include "bof3/bof3.h"
#include "gpu/prim.h"
#include <stdlib.h>

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name this overlay's exact lifts already use. Every use
 * in this body re-reads the cell, because an intervening call may republish it:
 * the opening amplitude read, the palette selector read after rand, the record
 * base of the colour block, and the two in-loop reads for the jitter and for
 * the record words 0x34 and 0x38 each load the cell again. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad amplitude cell at 0x1F800008, the scale both profile products use.
 * The original forms its address once with a `lui %hi` plus `addiu %lo` pair
 * into a callee-saved register that lives from the entry block to the seeding
 * block, stores the work record's byte 0x0A through it and reads it back signed
 * once per profile statement, so it is a named halfword rather than a literal
 * address. */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern s16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Scratchpad driven-radius cell at 0x1F80000E and the primed ring cell at
 * 0x1F80001C. The original keeps the 0x1F80000E address in a callee-saved
 * register from the seeding block onward and reaches the profile halfwords
 * 0x1F800006, 0x1F80000A and 0x1F80000E as -4, 0 and +4 of it, so the three
 * stay one live pointer base. The 0x1F80001C address is likewise formed once
 * and re-based by -8 into the quad base 0x1F800014 that the pass loop walks.
 * Calls between the stores and later reads preserve the original reloads
 * without volatile access. */
extern s16 D_1F80000E; /* @source 0x1F80000E @kind data */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* Three byte rows of the palette block at 0x801F0414, indexed by the work
 * object's byte 0x04 scaled by three. The original masks each row with its own
 * %hi/%lo pair over a base of 0x801F0414, 0x801F0415 and 0x801F0416, so the
 * three rows are separate symbols and the three-byte stride is what selects the
 * record. The block lives in this payload's trailing read-only subsegment, so
 * only the sub-span is bound here by its address label. */
extern u8 D_801F0414[]; /* @source 0x801F0414 @kind table */
extern u8 D_801F0415[]; /* @source 0x801F0415 @kind table */
extern u8 D_801F0416[]; /* @source 0x801F0416 @kind table */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the samples of a 12-bit
 * turn in the fixed-point scale the `>> 12` products below divide out. Both are
 * seeded with the profile angle shifted left eight in the entry block and with
 * the pass phase shifted left six in the loop, and the 0x801783C8 sample always
 * feeds the x halfwords while the 0x801782FC sample feeds the y halfwords. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive linker: links the packet at g_PrimCursor under the ordering
 * table words 0x34 and 0x38 of the work record with the given kind and size.
 * This body links a 0xC-byte packet before and a 0x24-byte POLY_G4 packet after
 * each projection, both with kind 0. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Shape evidence, not clone proof: a payload scan of all 122 extracted bmagic
 * binaries found this 1456-byte span only at its own offset and no 64-byte
 * prefix of it anywhere else, so the clone lever is ineligible and the body is
 * a clean-C seed. The statement order, the operand forms and the scratchpad
 * access spellings follow the exact bmagic draw-group siblings at 0x801F0054
 * (palette row selection by work byte 0x04 with the sixteen-step saturation
 * and the graded colour layout), 0x801EFA2C (the `lui $at` publish form for
 * fixed scratchpad cells beside a live pointer base) and 0x801EFAE0 (the four
 * SVECTOR quad, RotTransPers4 projection, func_80155A08 pairing and the
 * two-instruction tpage selection). The literals, the sixty-four-pass bound,
 * the 0x80 second ring radius and the palette rows at 0x801F0414 are this
 * span's own. */
/* @source 0x801EFAA4
 * @behavior Middle draw helper of this overlay's work-byte-2 handler group. Its
 *           only in-image reference is the jal at 0x801EF048 inside
 *           dispatchWorkByte2HandlerThenInvokeDrawHelpersAndPopWhenActive
 *           BmagicMagic03903_801EEFA8, taken while the work object's byte 0 is
 *           odd and its byte 2 is non-zero, between func_801EF9F4 and
 *           func_801F0054; the body reads no incoming register, so it takes no
 *           arguments and returns nothing.
 *           It publishes the work record's unsigned byte 0x0A into the
 *           scratchpad amplitude halfword 0x1F800008, calls rand, and takes the
 *           jittered profile angle ((rand() & 3) + byte 0x09) & 0xF shifted left
 *           eight into 0x1F80000A, the second-ring radius 0x80 into
 *           0x1F80000C, then the two profile radii from that same angle: the
 *           driven radius 0x1F800006 is twice the 0x801782FC sample times the
 *           amplitude shifted right twelve less six amplitudes, and the
 *           0x1F80000E radius is eight amplitudes less the quantity twice the
 *           0x801782FC sample of the published angle times the amplitude
 *           shifted right twelve minus 0x80.
 *           It then fills the three colour halfwords 0x1F800000, 0x1F800002 and
 *           0x1F800004 from the palette row the work record's unsigned byte
 *           0x04 selects at stride three: while its unsigned byte 0x09 is below
 *           0x10 each row byte is multiplied by that byte, and from 0x10 up
 *           each row byte is shifted left four and the byte itself is not read.
 *           The quad is primed at phase zero: 0x1F80001C and 0x1F80001E take the
 *           0x801783C8 and 0x801782FC samples times the driven radius shifted
 *           right twelve, 0x1F800020 takes the driven radius, 0x1F80002C and
 *           0x1F80002E take the two samples times 0x80 shifted right twelve, and
 *           0x1F800028 and 0x1F800030 are cleared.
 *           The pass loop runs sixty-four times, moving the phase by one
 *           sixty-fourth of a turn each time (i << 6 for i from 1 to 0x40), so
 *           the sixty-four translucent quads close the full turn. Each pass
 *           takes a fresh jittered angle ((rand() & 3) + i + byte 0x09) & 0xF
 *           shifted left eight, republishes the two profile radii from it, moves
 *           the primed pair into the leading quad edges, builds the far edges
 *           from the 0x801783C8 and 0x801782FC samples of the phase times the
 *           driven radius, and the second ring from the same samples times 0x80,
 *           and takes the signed 0x1F80002C halfword shifted left nine as the
 *           offset it adds to the work record words 0x34 and 0x38.
 *           It picks the draw-mode texture page, 0xA5 when GetGraphType returns
 *           1 or 2 and 0x35 otherwise, calling the query once per test with no
 *           cached answer, arms SetDrawMode on the primitive cursor g_PrimCursor
 *           with dfe 0, dtd 1 and a null RECT, links the 0xC-byte packet through
 *           func_80155A08 with kind 0, seeds the cursor packet through SetPolyG4,
 *           marks it semi-transparent, gives the near vertex pair the constant 1
 *           in every channel and both far vertices the published palette triple,
 *           projects the quad through RotTransPers4 into the primitive vertex
 *           pairs at 0x08, 0x10, 0x18 and 0x20 with two frame words as the depth
 *           and flag results, and links the 0x24-byte packet at the same record
 *           words with kind 0.
 *           The 0x58-byte frame keeps $ra, all six $s registers and the six
 *           outgoing stack words of RotTransPers4. The roles of work-object
 *           bytes 0x04 and 0x09 beyond the palette row and jitter uses above are
 *           not established, so they stay offset-anchored, and the scratchpad
 *           cells are named by address.
 * @status partial
 * @match 93.96
 * @residual Measured live: 342 of 364 instructions at the original 1456-byte
 *           size. The entry sampler argument shifts before the low-nibble
 *           mask and masks the shifted result in the call delay slot; the
 *           original masks first, shifts twice and leaves that slot empty.
 *           The pass sampler argument uses a move instead of a second shift.
 *           Other differences concern preheader address derivation and
 *           interleaving vertex copies with profile-radius calculations.
 */
void drawTranslucentGouraudQuadBandOfSixtyFourQuadsSteppingSixtyFourthTurnWithRandJitteredWorkByteAProfileTintedByPaletteRowAtWorkByte4ScaledByWorkByte9BmagicMagic03903_801EFAA4(
    void) {
  u8 *work;
  s16 *amp;
  s16 *edge;
  s16 *cells;
  u16 *tint;
  u16 *shape;
  POLY_G4 *prim;
  s32 jitter;
  s32 sample;
  s32 delta;
  u16 phase16;
  s32 phase;
  s32 offset;
  s32 origin_x;
  s32 origin_y;
  s32 tpage;
  s32 i;
  long depth;
  long flag;

  amp = &D_1F800008;
  *amp = g_battle_work[0xA];
  jitter = (rand() & 3) + g_battle_work[9];
  D_1F80000A = (jitter & 0xF) << 8;
  sample = func_801782FC((jitter << 8) & 0xF00);
  *SPAD_ADDR(u16, 0x0C) = 0x80;
  *SPAD_ADDR(s16, 0x06) = ((sample << 1) * *amp >> 12) - *amp * 6;
  sample = func_801782FC(D_1F80000A);
  delta = ((sample << 2) * *amp >> 12) - 0x80;
  *SPAD_ADDR(s16, 0x0E) = *amp * 8 - delta;

  work = g_battle_work;
  if (work[9] < 0x10) {
    *SPAD_ADDR(u16, 0x00) = D_801F0414[work[4] * 3] * work[9];
    *SPAD_ADDR(u16, 0x02) = D_801F0415[work[4] * 3] * work[9];
    *SPAD_ADDR(u16, 0x04) = D_801F0416[work[4] * 3] * work[9];
  } else {
    *SPAD_ADDR(u16, 0x00) = D_801F0414[work[4] * 3] << 4;
    *SPAD_ADDR(u16, 0x02) = D_801F0415[work[4] * 3] << 4;
    *SPAD_ADDR(u16, 0x04) = D_801F0416[work[4] * 3] << 4;
  }

  edge = &D_1F80000E;
  shape = &D_1F80001C;

  shape[0] = (func_801783C8(0) * *edge) >> 12;
  *SPAD_ADDR(u16, 0x1E) = (func_801782FC(0) * *edge) >> 12;
  *SPAD_ADDR(u16, 0x20) = *SPAD_ADDR(u16, 0x06);
  *SPAD_ADDR(u16, 0x2C) = (func_801783C8(0) * D_1F80000C) >> 12;
  *SPAD_ADDR(u16, 0x2E) = (func_801782FC(0) * D_1F80000C) >> 12;
  i = 1;
  cells = edge - 2;
  tint = (u16 *)(edge - 7);
  *SPAD_ADDR(u16, 0x28) = 0;
  *SPAD_ADDR(u16, 0x30) = 0;
  shape -= 4;

  do {
    phase = i << 6;
    jitter = ((rand() & 3) + i + g_battle_work[9]) & 0xF;
    phase16 = jitter << 8;
    cells[0] = phase16;
    sample = func_801782FC(phase16);
    cells[-2] = ((sample << 1) * cells[-1] >> 12) - cells[-1] * 6;
    delta = ((func_801782FC(cells[0]) << 2) * cells[-1] >> 12) - 0x80;
    cells[2] = cells[-1] * 8 - delta;

    shape[1] = shape[5];
    shape[2] = shape[6];
    shape[0] = shape[4];
    shape[4] = (func_801783C8(phase) * cells[2]) >> 12;
    shape[5] = (func_801782FC(phase) * cells[2]) >> 12;
    shape[6] = cells[-2];
    shape[8] = shape[0xC];
    shape[9] = shape[0xD];
    shape[0xC] = (func_801783C8(phase) * cells[1]) >> 12;
    shape[0xD] = (func_801782FC(phase) * cells[1]) >> 12;

    offset = *(s16 *)(shape + 0xC) << 9;
    origin_x = offset + *(s32 *)(g_battle_work + 0x34);
    origin_y = offset + *(s32 *)(g_battle_work + 0x38);

    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0xA5;
    } else {
      tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(origin_x, origin_y, 0, 0xC);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = tint[0];
    prim->g2 = tint[1];
    prim->b2 = tint[2];
    prim->r3 = tint[0];
    prim->g3 = tint[1];
    prim->b3 = tint[2];

    i++;
    RotTransPers4((SVECTOR *)shape, (SVECTOR *)(shape + 4),
                  (SVECTOR *)(shape + 8), (SVECTOR *)(shape + 0xC),
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);

    func_80155A08(origin_x, origin_y, 0, 0x24);
  } while (i < 0x41);
}
