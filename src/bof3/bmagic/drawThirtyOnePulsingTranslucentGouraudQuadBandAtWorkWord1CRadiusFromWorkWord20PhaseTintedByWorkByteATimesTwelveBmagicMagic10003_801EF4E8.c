#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work, the name this overlay's exact lifts already use. This
 * body re-reads the cell once in the preheader and once per pass, because the
 * intervening calls may republish it, so every access is a fresh lui/lw pair. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius pair at 0x1F800000. The original forms its address once
 * with a lui %hi plus addiu %lo pair into a callee-saved register that lives
 * from the preheader to the tail, stores the pulsing radius through it and
 * reaches the fixed radius as +2 of the same base, so the two stay one live
 * pointer rather than two named halfwords. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad cell 0x1F80001C, the quad the pass loop walks. The original forms
 * its address once and re-bases it by -8 into the leading vertex pair, so the
 * four vertices are +0, +8, +0x10 and +0x18 of that base. */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* Scratchpad cells 0x1F800004 through 0x1F80000A, published in the preheader
 * and read in the pass loop through the radius base: the near vertex pair takes
 * the byte at 0x1F800004 and the far pair the byte at 0x1F800006, the phase
 * index lives at 0x1F800008 and its scaled angle at 0x1F80000A. Each is a named
 * halfword because the original publishes it with its own lui %hi plus offset
 * store and never forms a symbol address for the group. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind data */

/* Scratchpad cells 0x1F80001E, 0x1F80002C and 0x1F80002E: the y halfword of the
 * driving vertex pair and the x and y halfwords of the leading pair. The
 * preheader primes each with its own lui %hi plus offset store, while the pass
 * loop reaches the same three through the quad base at 0x1F800014. */
extern u16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern u16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern u16 D_1F80002E; /* @source 0x1F80002E @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the two samples of a
 * twelve-bit turn in the fixed-point scale the >> 12 products below divide out.
 * The 0x801782FC sample feeds every x halfword and the 0x801783C8 sample every
 * y halfword. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */

/* Shared primitive linker: links the packet at g_PrimCursor under the work
 * record words 0x34 and 0x38 with the given kind and size. This body links a
 * 0xC-byte packet before the pass loop and a 0x24-byte packet at the end of each
 * pass, both with kind 2. */
extern void func_80155A08(s32 x, s32 y, s32 kind,
                          s32 size); /* @source 0x80155A08 @kind function */

/* Shape evidence, not clone proof: a scan of every extracted binary for this
 * span's own 1024 bytes returned only its own payload at offset 0x8E8, so the
 * clone lever is ineligible and the body is a clean-C seed. The statement
 * order, the operand forms and the scratchpad spellings follow the bmagic draw
 * sibling at 0x801EF88C of emi/bmagic/magic101/03, whose own live measurement
 * was 88.56 percent when this seed was written (thirty-one passes, the
 * graph-type draw mode, the primitive linker pairing, the four SVECTOR quad and
 * the RotTransPers4 projection). This span's radii come from work word 0x1C
 * halved, its phase from work word 0x20 divided by 64 and its two tints from
 * work byte 0x0A and twelve times that byte, all written from its own bytes. */
/* @source 0x801EF4E8
 * @behavior Draw step of the bmagic/100/03 overlay, called by the in-payload
 *           jal at 0x801EEF04 between calls to 0x801EF434 and 0x80178C18.
 *           The caller does not establish gameplay roles for the radius,
 *           phase or tint fields. The body reads no incoming register and
 *           returns nothing.
 *           It picks the draw-mode texture page, 0xA5 when GetGraphType answers
 *           1 or 2 and 0x35 otherwise, calling the query once per test with no
 *           cached answer, arms SetDrawMode on the primitive cursor g_PrimCursor
 *           with dfe 0, dtd 1 and a null RECT, and links a 0xC-byte packet under
 *           the work record words 0x34 and 0x38 with kind 2.
 *           The preheader samples the rotator at phase zero and publishes the
 *           pulsing radius at 0x1F800000 as work word 0x1C plus that sample
 *           times half of work word 0x1C shifted right twelve, the fixed radius
 *           at 0x1F800002 as work word 0x1C, the phase index at 0x1F800008 as
 *           work word 0x20 divided by 64 masked to six bits, the near tint at
 *           0x1F800004 as twelve times work byte 0x0A and the far tint at
 *           0x1F800006 as work byte 0x0A, and the scaled angle at 0x1F80000A as
 *           that index shifted left six. It then primes the driving vertex pair
 *           at 0x1F80001C from the two rotator samples of that angle times the
 *           pulsing radius and the leading vertex pair at 0x1F80002C from the
 *           same samples times the fixed radius, each shifted right twelve.
 *           The pass loop runs thirty-one times, advancing the phase index by
 *           one step of 0x40 each pass and resampling the pulsing radius from a
 *           rotator sample of the step shifted left six, which scales the half
 *           radius so the band pulses over the sweep. Each pass carries the
 *           previous outer and inner vertices forward into the leading pair and
 *           the leading half of the quad, clears the three vertex z halfwords,
 *           recomputes the driving and leading vertex pairs at the new angle
 *           from the pulsing and fixed radii, seeds the cursor packet through
 *           SetPolyG4, marks it semi-transparent, gives the near vertex pair the
 *           0x1F800004 byte in every channel and both far vertices the
 *           0x1F800006 byte, projects the quad through RotTransPers4 into the
 *           primitive vertex pairs at 0x08, 0x10, 0x18 and 0x20 with two frame
 *           words as the depth and flag results, and links the 0x24-byte packet
 *           under the same work words with kind 2.
 *           The 0x48-byte frame keeps $ra, all five $s registers and the six
 *           outgoing stack words of RotTransPers4. The roles of work word 0x1C,
 *           work word 0x20 and work byte 0x0A beyond the radius, phase and tint
 *           uses above are not established, so they stay offset-anchored.
 * @status partial
 * @match 83.40
 * @residual Measured live at 216 of 256 instructions in a 1036-byte body. The
 *           first difference is the preheader work-record field load, which
 *           lands in $a1 here against $a0 in the original. The rest is that same
 *           class: the pulsing-radius base takes $s2 instead of $s1 while the
 *           quad base takes $s1 instead of $s0, the preheader reaches the fixed
 *           radius halfword through its literal address instead of +2 of the
 *           live base, four loop call arguments load after the vertex copies
 *           rather than before the multiply's mflo, the pass counter increments
 *           one slot earlier, and the two preheader tint stores swap order with
 *           the scaled angle store. Sweeps from this seed (a typed tint view,
 *           a copied radius base, an in-place radius accumulation, a preheader
 *           accumulation) moved the measured match from 195 to 216 of the
 *           original instructions; the family sibling settled on the same
 *           residual class at 88.56 percent.
 */
void drawThirtyOnePulsingTranslucentGouraudQuadBandAtWorkWord1CRadiusFromWorkWord20PhaseTintedByWorkByteATimesTwelveBmagicMagic10003_801EF4E8(
    void) {
  POLY_G4 *prim;
  SVECTOR *shape;
  s16 *radius;
  s16 *phase;
  u8 *work;
  s32 sample;
  s32 angle;
  s32 start;
  s32 step;
  s32 tpage;
  s32 field;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                2, 0xC);

  sample = func_801782FC(0);
  work = g_battle_work;
  radius = &D_1F800000;
  field = *(s32 *)(work + 0x1C);
  field += (sample * (field / 2)) >> 12;
  *radius = field;
  radius[1] = *(s32 *)(work + 0x1C);
  start = (*(s32 *)(work + 0x20) / 64) & 0x3F;
  D_1F800008 = start;
  D_1F800004 = work[0xA] * 12;
  angle = start << 6;
  D_1F80000A = angle;
  D_1F800006 = work[0xA];
  shape = (SVECTOR *)&D_1F80001C;
  shape->vx = (func_801782FC(angle) * *radius) >> 12;
  D_1F80001E = (func_801783C8(D_1F80000A) * *radius) >> 12;
  D_1F80002C = (func_801782FC(D_1F80000A) * radius[1]) >> 12;
  D_1F80002E = (func_801783C8(D_1F80000A) * radius[1]) >> 12;

  step = 1;
  phase = radius + 5;
  shape--;

  do {
    sample = step + phase[-1];
    *phase = (sample & 0x3F) << 6;
    sample = func_801782FC(step << 6);
    field = *(s32 *)(g_battle_work + 0x1C);
    field += (sample * (field / 2)) >> 12;
    shape[0].vx = shape[1].vx;
    shape[0].vz = 0;
    shape[0].vy = shape[1].vy;
    phase[-5] = field;
    shape[1].vx = (func_801782FC(*phase) * phase[-5]) >> 12;
    shape[1].vy = (func_801783C8(*phase) * phase[-5]) >> 12;
    shape[1].vz = 0;
    shape[2].vz = 0;
    shape[2].vx = shape[3].vx;
    shape[2].vy = shape[3].vy;
    shape[3].vx = (func_801782FC(*phase) * radius[1]) >> 12;
    shape[3].vy = (func_801783C8(*phase) * radius[1]) >> 12;
    shape[3].vz = 0;
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->r0 = ((u16 *)radius)[2];
    prim->g0 = ((u16 *)radius)[2];
    prim->b0 = ((u16 *)radius)[2];
    prim->r1 = ((u16 *)radius)[2];
    prim->g1 = ((u16 *)radius)[2];
    prim->b1 = ((u16 *)radius)[2];
    prim->r2 = ((u16 *)radius)[3];
    prim->g2 = ((u16 *)radius)[3];
    prim->b2 = ((u16 *)radius)[3];
    prim->r3 = ((u16 *)radius)[3];
    prim->g3 = ((u16 *)radius)[3];
    prim->b3 = ((u16 *)radius)[3];
    RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                  &depth, &flag);
    step++;
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x24);
  } while (step < 0x20);
}
