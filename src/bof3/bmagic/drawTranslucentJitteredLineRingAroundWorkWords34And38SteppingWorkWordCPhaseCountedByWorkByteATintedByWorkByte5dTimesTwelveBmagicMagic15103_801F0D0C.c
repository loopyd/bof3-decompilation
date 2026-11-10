#include "bof3/bof3.h"
#include "gpu/prim.h"
#include <stdlib.h>

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original re-reads the cell with its own lui/lw pair before the
 * primitive count is sampled, once more for the phase word of each pass and
 * once more for the tpage bits, so the plain named cell suffices. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Radial scale cell 0x1F800000, 0x200, and the two radius cells the passes
 * divide the rotator samples by. The original stores the scale through its
 * literal address and reloads it for each of the three projections, and it
 * keeps the phase-reduced radius in 0x1F800002 for the shade sample of the
 * same pass. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */

/* Per-pass radius jitter, rand() & 3 plus four. Together with the radius, the
 * scale and the phase cell it forms the pass block the loop addresses through
 * one pointer biased at 0x1F800004: the jitter at displacement zero, the
 * radius at minus two, the scale at minus four and the phase at ten, which is
 * the displacement pair the original emits. */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Tint halfword at 0x1F80000A, work byte 0x5D times twelve. The original
 * reaches the cell twice: the seed store goes through the pass-block pointer
 * before it is stepped back three halfwords, and each pass re-reads the cell
 * through this named row for its three colour bytes, because those byte
 * stores may alias it. */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */

/* Phase cell 0x1F80000E. Each pass keeps the quarter-turn phase it scales for
 * the rotator there, then overwrites it with the four-bit-shifted warped angle
 * whose halfword the pass's cosine sample is taken from. */
extern s16 D_1F80000E; /* @source 0x1F80000E @kind data */

/* Current vertex of the ring at 0x1F80001C, and the shade-shifted y halfword
 * at 0x1F800020 that the next pass rolls into the same vertex. */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit turn, which the `>> 12` products below divide out. Every
 * call here takes a phase in the same 4096-step scale: the primed vertices
 * their turn shifted left four, each pass the quarter-turn phase shifted left
 * seven and the warped angle shifted left four again. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Shared helper that queues the primitive held by the shared cursor: it takes
 * the segment origin's x and y, the output list and the primitive size. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive header helper: stamps the code, length and tpage words of
 * the primitive the shared cursor holds. */
void func_8017AA80(void *primitive);

/* @source 0x801F0D0C
 * @behavior Translucent jittered line ring of this overlay's work object. It
 *           takes the shade offset the work byte 9 accumulator of the caller at
 *           0x801F0AC8 published as its first argument and the shade pair's
 *           high byte as its second, and reads the work object the scratchpad
 *           cell at 0x1F800044 publishes. It first seeds the scratchpad ring
 *           block: 0x200 as the radial scale at 0x1F800000, the sign-extended
 *           work byte 0x5D times twelve as the tint halfword at 0x1F80000A, the
 *           sine of the second argument shifted left four times the scale
 *           shifted right twelve at 0x1F800002, and the first ring vertex at
 *           0x1F80001C from the sine and cosine of the first argument shifted
 *           left four times that radius, with 0x1F800020 taking the cosine of
 *           the second argument times the scale. The work object byte 0xA is
 *           the segment count and ends the body when zero.
 *
 *           Each pass rolls the current vertex into the previous one, draws a
 *           new radius jitter of rand() & 3 plus four, takes the work object
 *           word 0xC plus its own index masked to five bits as the phase
 *           shifted left seven, and warps the second argument by the sine of
 *           that phase times the jitter shifted right twelve. That warped angle
 *           shifted left four is stored at 0x1F80000E and is the phase whose
 *           sine times the scale becomes the pass radius at 0x1F800002, while
 *           its cosine times the scale is stored at 0x1F800020. The first
 *           argument then accumulates rand() & 7, and the sine and cosine of
 *           that accumulated angle shifted left four times the pass radius
 *           become the new vertex x and y at 0x1F80001C and 0x1F80001E.
 *
 *           The pass origin is that vertex shifted left nine plus the work
 *           object words 0x34 and 0x38. The draw-mode tpage argument is the
 *           work byte 0xB low two bits shifted left seven or'd with 0x25 when
 *           GetGraphType answers 1 or 2 (two calls, the second only when the
 *           first answers something else) and shifted left five or'd with 0x15
 *           otherwise; SetDrawMode takes dfe 0, dtd 1 and a null RECT, and the
 *           helper at 0x80155A08 queues a 0xC-byte packet for output list 2
 *           from the pass origin. The shared cursor's LINE_F2 is then stamped
 *           by the header helper at 0x8017AA80, marked semi-transparent with
 *           the whole work byte 0xB, and both ring vertices are projected
 *           through RotTransPers into its vertex pairs at 0x08 and 0x0C with
 *           two frame words as the depth and flag results. All three colour
 *           bytes take the tint halfword's low byte and the pass closes by
 *           queueing the 0x10-byte packet for the same origin and list, so
 *           consecutive segments share their joint. Frame 0x48 keeps $ra, all
 *           six $s registers and $fp with the three outgoing argument words,
 *           and the body writes nothing outside the scratchpad and the
 *           primitives it emits.
 * @status partial
 * @match 93.12
 * @residual 230 of 247 original instructions match at the original byte size.
 *           The original keeps the raw second argument in $fp from function
 *           entry and re-masks it at both uses, while this source keeps a
 *           masked copy in $s3 and re-copies it into $fp in the loop preheader.
 *           That shifts the prologue register-save order and drops the second
 *           `andi` of the warped angle. The original also loads the work-object
 *           pointer before storing the jitter halfword, while this source
 *           loads it afterward and needs a load-delay `nop`.
 */
void drawTranslucentJitteredLineRingAroundWorkWords34And38SteppingWorkWordCPhaseCountedByWorkByteATintedByWorkByte5dTimesTwelveBmagicMagic15103_801F0D0C(
    u8 shade, u8 page) {
  SVECTOR *shape;
  s16 *pass;
  SVECTOR *mid;
  LINE_F2 *prim;
  s32 page_angle;
  s32 tint;
  u16 *tint_cell;
  s32 angle;
  u16 phase;
  s32 sample;
  s32 tpage;
  s32 x;
  s32 y;
  u8 i;
  long projected;
  long depth;

  page_angle = page << 4;
  pass = (s16 *)&D_1F80000A;
  tint = (s8)g_battle_work[0x5D] * 12;
  D_1F800000 = 0x200;
  *pass = tint;
  D_1F800002 = (func_801782FC(page_angle) * D_1F800000) >> 12;
  mid = (SVECTOR *)&D_1F80001C;
  mid->vx = (func_801782FC(shade << 4) * D_1F800002) >> 12;
  D_1F80001E = (func_801783C8(shade << 4) * D_1F800002) >> 12;
  D_1F800020 = (func_801783C8(page_angle) * D_1F800000) >> 12;

  if (g_battle_work[0xA] != 0) {
    i = 0;
    shape = mid - 1;
    pass -= 3;
    do {
      shape[0].vx = shape[1].vx;
      shape[0].vy = shape[1].vy;
      shape[0].vz = shape[1].vz;

      pass[0] = (rand() & 3) + 4;
      phase = (FIELD_REF(s32, g_battle_work, 0xC) + i) & 0x1F;
      pass[5] = phase << 7;
      sample = func_801782FC(phase << 7);
      angle = page + ((sample * pass[0]) >> 12);
      pass[5] = angle << 4;
      pass[-1] = (func_801782FC((s16)(angle << 4)) * pass[-2]) >> 12;
      shape[1].vz = (func_801783C8(pass[5]) * pass[-2]) >> 12;

      shade += rand() & 7;
      shape[1].vx = (func_801782FC(shade << 4) * pass[-1]) >> 12;
      shape[1].vy = (func_801783C8(shade << 4) * pass[-1]) >> 12;
      x = ((s32)shape[1].vx << 9) + FIELD_REF(s32, g_battle_work, 0x34);
      y = ((s32)shape[1].vy << 9) + FIELD_REF(s32, g_battle_work, 0x38);

      if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = ((g_battle_work[0xB] & 3) << 7) | 0x25;
      } else {
        tpage = ((g_battle_work[0xB] & 3) << 5) | 0x15;
      }
      SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
      func_80155A08(x, y, 2, 0xC);

      prim = (LINE_F2 *)g_PrimCursor;
      func_8017AA80(prim);
      SetSemiTrans(prim, g_battle_work[0xB]);
      RotTransPers(&shape[0], (long *)&prim->x0, &projected, &depth);
      RotTransPers(&shape[1], (long *)&prim->x1, &projected, &depth);
      tint_cell = (u16 *)&D_1F80000A;
      prim->r0 = (u8)*tint_cell;
      prim->g0 = (u8)*tint_cell;
      prim->b0 = (u8)*tint_cell;
      func_80155A08(x, y, 2, 0x10);
      i++;
    } while (i < g_battle_work[0xA]);
  }
}
