#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map as g_battle_work. The original forms the cell with its own `lui 0x1F80`
 * plus `lw 0x44` pair at the opening byte reads, at the packet link and the
 * loop test inside the pass, and again at the second packet link, because the
 * rotator calls and the primitive stores sit between those sites. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad cells this body publishes before the pass loop. The original
 * reaches each of them through its own `lui` plus `sw`/`sh` pair at the fixed
 * address, and reads the two radius words and the phase cell back the same
 * way, so they are separate fixed-address objects here rather than members of
 * one pointer-based view: 0x1F800000 holds the near radius 0x180, 0x1F800004
 * the far radius 0x1E0, 0x1F800008 the phase sample and 0x1F80000C the tint
 * word the pass loop decrements by four. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 D_1F800008; /* @source 0x1F800008 @kind data */
extern s32 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Scratchpad vertex quad at 0x1F800014: the four SVECTOR inputs of
 * RotTransPers4. The original forms the address once as `lui` plus `addiu` and
 * addresses the twelve halfwords as displacements of it, the one-instruction
 * `sh` form the quad needs, with the single priming store made through a
 * register holding 0x1F80001C and the loop base then derived from it with one
 * `addiu -8`. */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */

/* The remaining primed halfwords of that quad. The original materialises each
 * of these addresses on its own before the store instead of reusing the quad
 * base above, so they stay separate fixed-address objects. */
extern u16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern u16 D_1F800020; /* @source 0x1F800020 @kind data */
extern u16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern u16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern u16 D_1F800030; /* @source 0x1F800030 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the samples of a 12-bit
 * turn in the fixed-point scale the `>> 12` products below divide out. The
 * opening sample takes the starting phase shifted left six; every later call
 * in this body takes the phase cell read back, the cosine sample feeding the y
 * halfwords and the sine sample the z halfwords. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair, links the primitive at g_PrimCursor with the given kind and
 * advances the cursor by the given size class, which is why this body links a
 * 0xC-byte packet at the work record words 0x34 and 0x38 and then one
 * 0x24-byte POLY_G4 packet at those same words per pass, both with kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EFAE0
 * @behavior Translucent Gouraud quad-ring emitter of this overlay draw group:
 *           no `jal`, pointer word or table entry of this payload names the
 *           span and it reads no incoming register, so the overlay loader
 *           entry is its only route and it takes no arguments. It stores the
 *           near radius 0x180 into the scratchpad word 0x1F800000 and the far
 *           radius 0x1E0 into 0x1F800004, then seeds the colour word
 *           0x1F80000C with work byte 0x0A shifted right one and multiplied by
 *           four plus one, and takes that shift plus one as the pass count,
 *           which it compares against the low six bits of work byte 0x09. It
 *           primes the phase cell 0x1F800008 with the sine of the starting
 *           phase shifted left six, scaled by `<< 9` and `>> 12` and biased by
 *           0x400, and primes the leading edge of the quad at 0x1F800014: the
 *           second and fourth vertices take a zero x and the cosine and sine
 *           samples times the far and near radius words shifted right twelve.
 *           Each pass then publishes the phase cell again for the stepped
 *           phase, moves that primed edge into the first and third vertices,
 *           builds the new second and fourth vertices from four more rotator
 *           samples against the same two radius words, picks the draw-mode
 *           tpage argument 0xA5 when GetGraphType returns 1 or 2 and 0x35
 *           otherwise, seeds SetDrawMode at g_PrimCursor with dfe 0, dtd 1 and
 *           a null RECT, links the 0xC-byte packet at the work record words
 *           0x34 and 0x38 with kind 2, seeds the cursor packet through
 *           SetPolyG4, marks it semi-transparent, projects the quad through
 *           RotTransPers4 into the primitive vertex pairs at 0x08, 0x10, 0x18
 *           and 0x20 with two frame words as the depth and flag results, gives
 *           all four vertices the colour word in red with 1 in green and blue
 *           after subtracting 4 from that word between the first and second
 *           vertices, links the 0x24-byte quad at the same record words with
 *           kind 2, and steps the six-bit phase down by one. The 0x50-byte
 *           frame keeps $ra, all six $s registers and the six outgoing stack
 *           words of RotTransPers4.
 * @status partial
 * @match 95.45
 * @residual Eleven of the 242 instructions differ at the payload-exact 968
 *           bytes, all of them in the block before the pass loop. The original
 *           materialises the primed halfword address 0x1F80001C into a saved
 *           register and stores through it, then derives the quad base
 *           0x1F800014 from that register with one `addiu -8` in the loop
 *           preheader; this source spells the same store as `shape[4]` over a
 *           quad base the compiler folds to a constant address, so it emits
 *           the store through `$at` and materialises the quad base with its
 *           own `lui`/`addiu` pair. The same fold leaves the tint pointer in
 *           `$s5` instead of the original `$s0` plus the preheader `move $s5,
 *           $s0`, and puts the loop-condition decrement in `$v0` instead of
 *           `$v1`. Every instruction from the loop body's first rotator call
 *           to the epilogue matches, and the two radius stores, the
 *           preheader phase store and the five absolute primed-halfword
 *           stores are identical, so the residual is the allocation of one
 *           saved register rather than a body shape.
 */
void drawTranslucentGouraudQuadRingBetweenRadii384And480SteppingSixtyFourthTurnFadingRedByFourFromWorkByteABmagicMagic10703_801EFAE0(
    void) {
  s32 *tint;
  s32 *phase;
  u16 *shape;
  POLY_G4 *prim;
  s32 steps;
  s32 step;
  s32 phase_value;
  s32 tpage;
  long depth;
  long flag;

  tint = &D_1F80000C;
  *tint = (g_battle_work[0xA] >> 1) * 4 + 1;
  D_1F800000 = 0x180;
  D_1F800004 = 0x1E0;
  step = g_battle_work[0x9] & 0x3F;
  steps = (g_battle_work[0xA] >> 1) + 1;

  D_1F800008 = ((func_801782FC(step << 6) << 9) >> 12) + 0x400;
  shape = &D_1F800014;
  shape[4] = 0;
  D_1F80001E = (func_801783C8(D_1F800008) * D_1F800000) >> 12;
  D_1F800020 = (func_801782FC(D_1F800008) * D_1F800000) >> 12;
  D_1F80002C = 0;
  D_1F80002E = (func_801783C8(D_1F800008) * D_1F800004) >> 12;
  D_1F800030 = (func_801782FC(D_1F800008) * D_1F800004) >> 12;

  phase = tint - 1;
  while ((step = (step - 1) & 0x3F) != ((g_battle_work[0x9] - steps) & 0x3F)) {
    phase_value = ((func_801782FC(step << 6) << 9) >> 12) + 0x400;
    *phase = phase_value;
    shape[0] = 0;
    shape[4] = 0;
    shape[1] = shape[5];
    shape[2] = shape[6];
    shape[5] = (func_801783C8(phase_value) * *(phase - 2)) >> 12;
    shape[6] = (func_801782FC(*phase) * *(phase - 2)) >> 12;
    shape[8] = 0;
    shape[0xC] = 0;
    shape[9] = shape[0xD];
    shape[0xA] = shape[0xE];
    shape[0xD] = (func_801783C8(*phase) * *(phase - 1)) >> 12;
    shape[0xE] = (func_801782FC(*phase) * *(phase - 1)) >> 12;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0xA5;
    } else {
      tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0xC);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    RotTransPers4((SVECTOR *)shape, (SVECTOR *)(shape + 4),
                  (SVECTOR *)(shape + 8), (SVECTOR *)(shape + 0xC),
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);

    prim->r0 = *tint;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = *tint;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = (*tint -= 4);
    prim->g2 = 1;
    prim->b2 = 1;
    prim->r3 = *tint;
    prim->g3 = 1;
    prim->b3 = 1;

    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x24);
  }
}
