#include "bof3/bof3.h"
#include "gpu/prim.h"
#include <stdlib.h>

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work, the name this overlay's lifts already use. The cell is
 * re-read for every use in this body: the opening jitter, the two profile
 * constants, and the record words 0x34 and 0x38 of the loop tail, because the
 * sampler and draw calls between them may republish the pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad phase cell at 0x1F800004, the halfword every sampler call in this
 * body reads back. The original forms its address once with a `lui %hi` plus
 * `addiu %lo` pair into a register it keeps from entry to the loop and reaches
 * its neighbours 0x1F800002 and 0x1F800008 and its base-relative radius halfword
 * 0x1F800000 as -2, +4 and -4 of that base, so the four stay one live pointer
 * and the cell is signed: the run seed and the two pass samples are read with
 * `lh`. The radius halfword 0x1F800000 is only ever written as a constant, so
 * it is reached through the scratchpad macro instead of this base. */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Scratchpad quad base at 0x1F800014, reached through the 0x1F80001C address the
 * original forms with its own `lui %hi` plus `addiu %lo` pair and then re-bases
 * by -8. The seed block writes the second vertex through the unbased pointer; the
 * loop walks the four 8-byte vertices as 0x00, 0x04, 0x08 and 0x0C of a u16
 * array, which is the same spelling this overlay's other quad lift uses. */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine samples
 * of a 12-bit phase in the fixed-point scale the `>> 12` products divide out.
 * The run seed samples phase zero, the profile constants sample the seeded phase
 * cell, and each pass samples its own phase `i << 6` for the vertex products. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive linker: links the packet now at g_PrimCursor under the
 * ordering-table words 0x34 and 0x38 of the work record with the given kind and
 * length. This body links an empty 0xC-byte packet before each projection and
 * the 0x24-byte POLY_G4 packet after it, both with kind 0. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* Seed provenance: the statement order, the operand forms and the scratchpad
 * access spellings follow the bmagic quad-band family's lifted source
 * drawTranslucentGouraudQuadBandOfSixtyFourQuadsSteppingSixtyFourthTurnWithRand
 * JitteredWorkByteAProfileTintedByPaletteRowAtWorkByte4ScaledByWorkByte9 at
 * 0x801EFAA4 of bmagic magic039, which shares the rand jitter read, the
 * `i << 6` phase walk of sixty-four passes, the `i == 0x40` closing-phase test,
 * the four-vertex RotTransPers4 projection and the two func_80155A08 links. That
 * sibling is measured partial in its own target, so it supplied shape only. The
 * literals, the scratchpad cells, the fixed 0x80 radius, the two constant vertex
 * colours, the frame size and the span's own store order are this body's. A
 * payload scan of every extracted binary under out/binaries found this 1096-byte
 * span only at its own offset, so the clone lever was declined. */
/* @source 0x801EF670
 * @behavior The work-record draw arm of this overlay's byte 2 dispatcher: its
 *           only in-image reference is the jal word at 0x801EF208 inside
 *           dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkB
 *           yte0Bit0AndWorkByte2SetBmagicMagic04103_801EF1D4, taken after
 *           projectWorkRecordPositionAndInstallTransformWithZeroRotation has
 *           installed the work record's transform and before that function's
 *           PopMatrix; no payload word holds 0x801EF670. It therefore draws
 *           under the installed matrix, pops no matrix of its own, reads no
 *           incoming register, takes no arguments and returns nothing.
 *           It publishes 0x80 into the scratchpad radius halfword 0x1F800000 and
 *           seeds the scratchpad phase halfword 0x1F800004 with
 *           ((rand() & 3) + work byte 0x9) & 0xF shifted left eight; that seeded
 *           value is kept for the closing pass. The signed phase cell then seeds
 *           the two profile radii: 0x1F800002 takes work byte 0x9 plus 0xA0 minus
 *           one sixteenth of the sine of the seeded phase, and the following
 *           sample of the same cell gives 0x1F800008 the value work byte 0xA times
 *           eight minus (one eighth of that sine, biased by -0x20).
 *           The quad is then primed at phase zero: the second vertex pair takes
 *           the cosine and sine of phase zero scaled by the signed 0x1F800002
 *           radius, its z takes the negated 0x1F800008 cell, and the fourth
 *           vertex pair takes the same samples scaled by the signed 0x1F800000
 *           radius, all shifted right twelve.
 *           The loop runs sixty-four passes, each with the phase `i << 6`, one
 *           sixty-fourth of the 0x1000 turn, jittered per pass: the phase cell is
 *           rewritten with ((rand() & 3) + i + work byte 0x9) & 0xF shifted left
 *           eight, and on the closing pass i == 0x40 it is rewritten with the
 *           seeded value instead, so the band closes on its first radius. Every
 *           pass copies the previous pass's second vertex into the first and its
 *           fourth vertex into the third, rebuilds the second vertex from the two
 *           profile radii above and the fourth from the fixed 0x80 radius, and
 *           projects all four through RotTransPers4 into the primitive vertex
 *           pairs at 0x08, 0x10, 0x18 and 0x20.
 *           The two record words 0x34 and 0x38 are the band's two ordering-table
 *           bases, each biased by the fourth vertex's x shifted left nine. The
 *           draw-mode texture page is 0xA5 when GetGraphType returns 1 or 2 and
 *           0x35 otherwise, armed through SetDrawMode on g_PrimCursor with dfe 0,
 *           dtd 1 and a null RECT, and the empty 0xC-byte packet is linked under
 *           those bases with kind 0. The POLY_G4 primitive is then seeded with
 *           SetPolyG4, marked semi-transparent with SetSemiTrans, and its first
 *           two vertices are white 1/1/1 while the last two take 0xC0 red with
 *           0x30 green and blue. After the projection the same bases link the
 *           0x24-byte packet with kind 0 and the body returns.
 *           The 0x58-byte frame keeps $ra and all seven $s registers, re-using the
 *           $s0 slot that held the seeded phase base for the primitive cursor,
 *           and the six outgoing stack words of RotTransPers4 sit at 0x10 to 0x24.
 * @status partial
 * @match 91.24
 * @residual Measured live: 250 of 274 instructions at 1092 bytes against the
 *           original 1096, first difference at +0x60. The entry block's phase
 *           cell store is the original's second shift of the masked nibble:
 *           0x801EF670 masks the nibble once, shifts it into the seeded local
 *           and shifts it a second time for the cell store, which sits in the
 *           func_801782FC jal delay slot with that fresh shift ahead of it.
 *           Every spelling that builds the store value from the same masked
 *           nibble is common-subexpression-eliminated into one shift here, so
 *           the store reads the seeded local and the body is one instruction
 *           and four bytes short. Probes on that site, all measured at the same
 *           1096-byte size: shifting for the store while the seeded local keeps
 *           its own shift, masking the nibble at its assignment, masking the
 *           shifted value instead, and spelling one side as a multiply each
 *           measured 218 of 274; storing the local and reading it back measured
 *           241 of 275 and the store written ahead of the local 238 of 275; a
 *           s16 or u16 conversion at the store, an s16 sampler formal and an
 *           initialised shape pointer measured neutral or worse. Every remaining
 *           difference is statement-order and register pick inside blocks that
 *           this site re-schedules.
 */
void renderRandProfileGraded(
    void) {
  s16 *cells;
  s16 *phases;
  u16 *shape;
  POLY_G4 *prim;
  s32 jitter;
  s32 sample;
  s32 delta;
  s32 initial;
  s32 phase;
  s32 phase16;
  s32 offset;
  s32 origin_x;
  s32 origin_y;
  s32 tpage;
  s32 i;
  long depth;
  long flag;

  jitter = (rand() & 3) + g_battle_work[9];
  cells = &D_1F800004;
  *SPAD_ADDR(u16, 0x00) = 0x80;
  initial = (jitter & 0xF) << 8;
  cells[0] = (jitter & 0xF) << 8;
  sample = func_801782FC(initial);
  i = 1;
  cells[-1] = g_battle_work[9] + 0xA0 - ((sample << 4) >> 12);
  sample = func_801782FC(cells[0]);
  delta = ((sample << 3) >> 12) - 0x20;
  cells[2] = (g_battle_work[0xA] << 3) - delta;

  shape = &D_1F80001C;
  shape[0] = (func_801783C8(0) * cells[-1]) >> 12;
  *SPAD_ADDR(u16, 0x1E) = (func_801782FC(0) * cells[-1]) >> 12;
  *SPAD_ADDR(u16, 0x20) = -*(u16 *)(cells + 2);
  *SPAD_ADDR(u16, 0x2C) = (func_801783C8(0) * cells[-2]) >> 12;
  *SPAD_ADDR(u16, 0x2E) = (func_801782FC(0) * cells[-2]) >> 12;
  *SPAD_ADDR(u16, 0x28) = 0;
  *SPAD_ADDR(u16, 0x30) = 0;
  phases = cells;
  shape -= 4;

  do {
    jitter = ((rand() & 3) + i + g_battle_work[9]) & 0xF;
    phase16 = jitter << 8;
    phases[0] = phase16;
    if (i == 0x40) {
      phases[0] = initial;
    }
    sample = func_801782FC(phases[0]);
    phases[-1] = g_battle_work[9] + 0xA0 - ((sample << 4) >> 12);
    phase = i << 6;
    sample = func_801782FC(phases[0]);

    shape[0] = shape[4];
    shape[1] = shape[5];
    shape[2] = shape[6];
    phases[2] = (g_battle_work[0xA] << 3) - (((sample << 3) >> 12) - 0x20);
    shape[4] = (func_801783C8(phase) * phases[-1]) >> 12;
    shape[5] = (func_801782FC(phase) * phases[-1]) >> 12;
    shape[6] = -*(u16 *)(phases + 2);
    shape[8] = shape[0xC];
    shape[9] = shape[0xD];
    shape[0xC] = (func_801783C8(phase) * phases[-2]) >> 12;
    shape[0xD] = (func_801782FC(phase) * phases[-2]) >> 12;

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
    prim->r2 = 0xC0;
    prim->g2 = 0x30;
    prim->b2 = 0x30;
    prim->r3 = 0xC0;
    prim->g3 = 0x30;
    prim->b3 = 0x30;

    i++;
    RotTransPers4((SVECTOR *)shape, (SVECTOR *)(shape + 4),
                  (SVECTOR *)(shape + 8), (SVECTOR *)(shape + 0xC),
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);
    func_80155A08(origin_x, origin_y, 0, 0x24);
  } while (i < 0x41);
}
