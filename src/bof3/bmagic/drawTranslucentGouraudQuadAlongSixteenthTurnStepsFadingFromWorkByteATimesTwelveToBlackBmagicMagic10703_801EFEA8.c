#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map as g_battle_work. The original forms the cell with its own `lui 0x1F80`
 * plus `lw 0x44` pair at the opening read, at the packet link inside the loop
 * and again in the loop test, because a call and a run of primitive stores sit
 * between those sites. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad colour word at 0x1F80000C, the tint this body publishes before the
 * loop, reads back for the four middle colour bytes and decrements by 0xC at
 * the end of every pass. The original materialises this address once as `lui`
 * plus `addiu` into $a1, derives the phase cell at 0x1F800008 as that register
 * minus four into $s2 and keeps the cell itself in $s3, so both later cells are
 * reached as displacements of this one base rather than through their own
 * address pairs: the phase store is `sw 0($s2)` and the radius word
 * 0x1F800000 is `lw -0x8($s2)`. */
extern s32 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Scratchpad vertex quad at 0x1F800014: the four SVECTOR inputs of
 * RotTransPers4. The original forms the address once as `lui` plus `addiu`
 * into $s1 on the loop entry path and addresses every one of the twelve
 * halfwords as a displacement of it, the one-instruction `sh` form the quad
 * needs; a per-cell absolute address would emit an extra address pair for each
 * of them. */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the samples of a 12-bit
 * turn in the fixed-point scale the `>> 12` products below divide out. This
 * body calls the pair once for the phase cell with the pass phase shifted left
 * six, then eight more times with the phase cell itself as the argument, the
 * cosine sample feeding the y halfwords and the sine sample the z halfwords. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper: it sits between the cursor read and
 * SetSemiTrans, the position this target's exact draw-group siblings give
 * their own seed helper, and its answer is unused. */
void func_8017AB08(void *arg0);

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair, links the primitive at g_PrimCursor with the given kind and
 * advances the cursor by the given size class, which is why this body links a
 * 0xC-byte packet at the work record words 0x34 and 0x38 and then one
 * 0x28-byte quad at those same words per pass, both with kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EFEA8
 * @behavior Translucent Gouraud quad emitter of this overlay draw group: no
 *           `jal`, pointer word or table entry of this payload names the span
 *           and it reads no incoming register, so the overlay loader entry is
 *           its only route and it takes no arguments. It reads the scratchpad
 *           work object cell once, stores work byte 0x0A shifted right two and
 *           multiplied by twelve plus one into the scratchpad colour word
 *           0x1F80000C, and takes that shift plus one as the pass count, which
 *           it compares against the low six bits of work byte 0x09 to decide
 *           whether any pass runs. Every pass clears the phase cell 0x1F800008
 *           to the sine of the pass phase shifted left six, scaled by `<< 9`
 *           and `>> 12` and biased by 0x400, then samples the rotator pair
 *           eight times with that cell as the phase and builds the four vertex
 *           quad at 0x1F800014: all four x halfwords are zero, the first
 *           vertex takes the sine and cosine samples times the radius word
 *           0x1F800000 shifted right twelve, the second takes the samples
 *           shifted right eight, the third twenty-one times that and the
 *           fourth three times shifted right five. It then picks the draw-mode
 *           tpage argument 0xA5 when GetGraphType returns 1 or 2 and 0x35
 *           otherwise, seeds SetDrawMode at g_PrimCursor with dfe 0, dtd 1 and
 *           a null RECT, links the 0xC-byte packet at the work record words
 *           0x34 and 0x38 with kind 2, takes the cursor primitive, seeds it
 *           through func_8017AB08, marks it semi-transparent, projects the quad
 *           through RotTransPers4 into the primitive vertex pairs at 0x08,
 *           0x10, 0x18 and 0x20 with two frame words as the depth and flag
 *           results, writes 1 into the first and last vertex colour triplets
 *           and the colour word into the two middle ones, subtracts 0xC from
 *           that word and links the 0x28-byte quad at the same record words
 *           with kind 2 while stepping the six-bit phase down by one. The
 *           0x50-byte frame keeps $ra, all six $s registers and the six
 *           outgoing stack words of RotTransPers4.
 * @status partial
 * @match 97.41
 * @residual The opening base handling is the only residual: five instructions
 *           of the preheader differ while every other instruction of the 193
 *           matches at the payload's own 772 bytes. The original forms the
 *           colour cell address once into $a1, stores through it, derives the
 *           phase cell as $a1 minus four into $s2 and copies $a1 into $s3 for
 *           the loop; this source binds the cell twice, once as the symbol
 *           store and once as the loop pointer, so the build materialises the
 *           phase cell address into $s2, derives the tint address as $s2 plus
 *           four into $s3 and reaches the pre-loop store through an absolute
 *           base in $at. Seeding the same base as one pointer local kept the
 *           loop pointer in $s3 with no copy, and giving the store its own
 *           local plus a copy reproduced the original's $a1 and move but
 *           moved the work-cell load and frame adjustment ahead of it.
 */
void drawTranslucentGouraudQuadAlongSixteenthTurnStepsFadingFromWorkByteATimesTwelveToBlackBmagicMagic10703_801EFEA8(
    void) {
  s32 *tint;
  s32 *phase;
  u16 *shape;
  POLY_G4 *prim;
  s32 level;
  s32 steps;
  s32 step;
  s32 phase_value;
  s32 tpage;
  long depth;
  long flag;

  level = g_battle_work[0xA] >> 2;
  D_1F80000C = level * 12 + 1;
  steps = level + 1;
  phase = &D_1F80000C - 1;
  step = g_battle_work[0x9] & 0x3F;
  if (step != ((g_battle_work[0x9] - steps) & 0x3F)) {
    shape = &D_1F800014;
    tint = &D_1F80000C;
    do {
      phase_value = ((func_801782FC(step << 6) << 9) >> 12) + 0x400;
      *phase = phase_value;
      shape[0] = 0;
      shape[1] = (func_801783C8(phase_value) * *(phase - 2)) >> 12;
      shape[2] = (func_801782FC(*phase) * *(phase - 2)) >> 12;
      shape[4] = 0;
      shape[5] = (u32)func_801783C8(*phase) >> 8;
      shape[6] = (u32)func_801782FC(*phase) >> 8;
      shape[8] = 0;
      shape[9] = ((u32)func_801783C8(*phase) * 21) >> 8;
      shape[10] = ((u32)func_801782FC(*phase) * 21) >> 8;
      shape[12] = 0;
      shape[13] = ((u32)func_801783C8(*phase) * 3) >> 5;
      shape[14] = ((u32)func_801782FC(*phase) * 3) >> 5;

      if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0xA5;
      } else {
        tpage = 0x35;
      }
      SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
      func_80155A08(*(s32 *)(g_battle_work + 0x34),
                    *(s32 *)(g_battle_work + 0x38), 2, 0xC);

      prim = (POLY_G4 *)g_PrimCursor;
      func_8017AB08(prim);
      SetSemiTrans(prim, 1);
      RotTransPers4((SVECTOR *)shape, (SVECTOR *)(shape + 4),
                    (SVECTOR *)(shape + 8), (SVECTOR *)(shape + 0xC),
                    (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                    (long *)&prim->x3, &depth, &flag);

      prim->r0 = 1;
      prim->g0 = 1;
      prim->b0 = 1;
      prim->r1 = *tint;
      prim->g1 = *tint;
      prim->b1 = *tint;
      prim->r2 = *tint;
      prim->g2 = *tint;
      prim->b2 = *tint;
      prim->r3 = 1;
      prim->g3 = 1;
      prim->b3 = 1;
      *tint = *tint - 0xC;

      func_80155A08(*(s32 *)(g_battle_work + 0x34),
                    *(s32 *)(g_battle_work + 0x38), 2, 0x28);
      step = (step - 1) & 0x3F;
    } while (step != ((g_battle_work[0x9] - steps) & 0x3F));
  }
}
