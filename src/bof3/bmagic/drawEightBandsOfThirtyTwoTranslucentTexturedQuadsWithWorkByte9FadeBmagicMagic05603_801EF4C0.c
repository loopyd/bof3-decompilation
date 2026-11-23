#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The band head reads it once for the byte-9 fade pair and the
 * second fade arm re-forms the cell, and the inner loop re-forms it once for the
 * two screen words the projection is biased by. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad fade halfwords at 0x1F800006 and 0x1F800008, the two graded edge
 * values of each band. The first arm publishes through the literal-address form
 * of the 0x1F800006 cell, which the `cell` base below also materialises for the
 * two derived pointers; the second arm and every colour store reach the same
 * words through the `shade` pointer. */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Scratchpad halfwords of the two rim vectors the fan walks: the pair at
 * 0x1F800018 and 0x1F800020 carries the shared x halfword, and the band head
 * writes them together. */
extern u16 D_1F800018; /* @source 0x1F800018 @kind data */
extern SVECTOR D_1F80001C; /* @source 0x1F80001C @kind data */
extern u16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern u16 D_1F800020; /* @source 0x1F800020 @kind data */
extern u16 D_1F800028; /* @source 0x1F800028 @kind data */
extern SVECTOR D_1F80002C; /* @source 0x1F80002C @kind data */
extern u16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern u16 D_1F800030; /* @source 0x1F800030 @kind data */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine of a
 * 12-bit phase in the fixed-point scale the `>> 12` products below divide out.
 * Both the radii and every vertex component of the fan come from these two. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */

/* Shared primitive connector that links a packet to an ordering bucket at a
 * screen position; this body seeds each quad packet with the projected origin
 * and links both the draw-mode and quad packets to bucket 2. */
extern void func_80155A08(s32 x, s32 y, s32 list, s32 size); /* @source 0x80155A08 @kind function */

/* Shared quad seed helper, called between the cursor read and SetSemiTrans. */
extern void func_8017A9E0(POLY_GT4 *primitive); /* @source 0x8017A9E0 @kind function */

extern s32 GetGraphType(void);

/* @source 0x801EF4C0
 * @behavior Eight-band semi-transparent textured Gouraud quad fan of this
 *           overlay's draw group, reached from the work record's byte-2 handler
 *           arm, which calls it between the transform installer at 0x801EF410
 *           and PopMatrix with no arguments.
 *           Each band grades its two edge colours from work byte 9: the value
 *           `byte 9 - 15 - count` is published at 0x1F800006 when
 *           `count - 1 < byte 9 - 16` and `byte 9 - 16 - count` at 0x1F800008
 *           when `count < byte 9 - 16`, each falling back to 0x81 - (value * 8)
 *           while it stays below 0x11 and to 1 otherwise.
 *           The band then sets the two radius halfwords at 0x1F800002 and
 *           0x1F800004 from the 0x1F800000 sample of the cosine of 0xB80 and of
 *           the band angle, seeds the four rim halfwords of both vectors from
 *           the sine and cosine of phase zero, and clears the shared x halfwords
 *           at 0x1F800018 and 0x1F800020.
 *           Thirty-two passes then walk the phase from 0x80 in steps of 0x80:
 *           each carries the previous rim point forward, rescales both rim
 *           vectors by the sine and cosine of the phase, projects four scratchpad
 *           vectors with RotTransPers4, and links a draw-mode packet and a
 *           0x34-byte textured quad to bucket 2 at the fourth rim point times 512
 *           plus work words 0x34 and 0x38. The quad takes tpage (0, 1, 0x340,
 *           0x100), clut (0, 0x1FA), draw mode 0xA5 for graph types 1 and 2 and
 *           0x35 otherwise, texels eight apart along the row, and the outer two
 *           edges at the first fade halfword and the inner two at the second.
 *           Eight bands run with the phase base and both texture rows descending
 *           by 0x80, 8 and 8, ending once the band angle drops below 0x11.
 *           It takes no arguments, returns nothing and its 0x88-byte frame saves
 *           $ra with $s0-$s7 and $fp, keeping the projection depth and flag words
 *           at 0x28 and 0x2C.
 *
 *           Statement order, operand forms and declaration style follow the
 *           same-family source
 *           drawEightBandsOfThirtyTwoTranslucentTexturedQuadsWithWorkByte9FadeBmagicMagic10303_801EF1D8,
 *           whose 0x5F4-byte body holds the same eight-band, thirty-two-quad
 *           walk, the same sampled radii and the same draw-mode, texture and
 *           packet forms; that body scrolls its texture rows by work byte A and
 *           reads one extra word, so neither its literals nor its size were
 *           copied. The 0x590-byte span is not byte-identical to any span of the
 *           398 payloads under out/binaries, so the clone lever is ineligible.
 *           Representation notes, each a measured contrast: the two rim copies
 *           read their source halfwords as unsigned halfwords while the
 *           projection reads the same words as signed SVECTOR components, so
 *           each copy source is an unsigned view; spelling them as an unsigned
 *           overlay pointer put the copies in their own register (s8) and
 *           evicted row from $fp, scoring 249 of 356 instructions. The band head
 *           reads the work cell inline instead of through the `work` local,
 *           which moves that load back to $v0 and scores 301. The two shared
 *           scratchpad halfword pairs take a chained assignment whose left side
 *           is the higher address, so the low halfword is stored first; through
 *           a named temporary their sign or zero extension landed in $a1 and
 *           scored 301. A per-band copy of the low texture row into its own local
 *           reproduces the loop-invariant load the original keeps in $fp, without
 *           which `count` took that register and the whole slot layout shifted by
 *           eight bytes.
 * @status partial
 * @match 88.48
 * @residual 315 of 356 instructions match at 1424 bytes with 41 operand
 *           differences and no size difference. The prologue materialises the
 *           0x1F80001C base in the original, where this spelling folds it to
 *           0x1F800014, and the original parks the band head cell in $t0 where
 *           this spelling uses $v0; the two fade arms keep their value in $v1
 *           where the original uses $v0 and schedules the second store after its
 *           branch; the inner loop's four sine and cosine products are
 *           materialised in $v1 and $a1 where the original uses $t0, and its
 *           final texture store block and the two loop increments are scheduled
 *           into different slots. Every remaining difference is a register or
 *           slot choice with an identical instruction count and byte size.
 */
void drawEightBandsOfThirtyTwoTranslucentTexturedQuadsWithWorkByte9FadeBmagicMagic05603_801EF4C0(
    void) {
  s16 *radii;
  u16 *cell;
  u16 *shade;
  SVECTOR *shape;
  POLY_GT4 *primitive;
  u8 *work;
  s16 fade;
  s32 band;
  s32 count;
  s32 saved_row;
  s32 row;
  s32 top_row;
  s32 lower_row;
  s32 lower_phase;
  s32 i;
  s32 u;
  s32 phase;
  s32 sample;
  s32 x;
  s32 y;
  s32 graph;
  long depth;
  long flag;

  band = 24;
  count = 1;
  cell = &D_1F800006;
  shade = cell + 1;
  radii = (s16 *)cell - 2;
  shape = &D_1F80001C - 1;
  row = 0x68;
  lower_row = 0x40;
  lower_phase = 0xB80;
  do {
    D_1F800006 = 129;
    D_1F800008 = 129;
    if (count - 1 < g_battle_work[9] - 16) {
      fade = g_battle_work[9] - 15 - count;
      D_1F800006 = fade;
      if (fade < 17) {
        D_1F800006 = 129 - (fade << 3);
      } else {
        D_1F800006 = 1;
      }
    }
    if (count < g_battle_work[9] - 16) {
      fade = g_battle_work[9] - 16 - count;
      shade[0] = fade;
      if (fade < 17) {
        shade[0] = 129 - (fade << 3);
      } else {
        shade[0] = 1;
      }
    }
    radii[0] = (func_801783C8(lower_phase) * radii[-1]) >> 12;
    radii[1] = (func_801783C8(band << 7) * radii[-1]) >> 12;
    D_1F80001C.vx = (func_801782FC(0) * radii[0]) >> 12;
    D_1F80001E = (func_801783C8(0) * radii[0]) >> 12;
    D_1F800020 = D_1F800018 =
        (u32)(func_801782FC(lower_phase) * radii[-1]) >> 12;
    D_1F80002C.vx = (func_801782FC(0) * radii[1]) >> 12;
    D_1F80002E = (func_801783C8(0) * radii[1]) >> 12;
    D_1F800030 = D_1F800028 =
        (u32)(func_801782FC(band << 7) * radii[-1]) >> 12;
    saved_row = lower_row;
    top_row = row;
    i = 1;
    u = 16;
    do {
      phase = i << 7;
      shape[0].vx = *(u16 *)&shape[1].vx;
      shape[0].vy = *(u16 *)&shape[1].vy;
      shape[1].vx = (func_801782FC(phase) * radii[0]) >> 12;
      sample = func_801783C8(phase) * radii[0];
      shape[2].vx = *(u16 *)&shape[3].vx;
      shape[2].vy = *(u16 *)&shape[3].vy;
      shape[1].vy = sample >> 12;
      shape[3].vx = (func_801782FC(phase) * radii[1]) >> 12;
      shape[3].vy = (func_801783C8(phase) * radii[1]) >> 12;
      work = g_battle_work;
      x = (shape[3].vx << 9) + FIELD_REF(s32, work, 0x34u);
      y = (shape[3].vy << 9) + FIELD_REF(s32, work, 0x38u);
      if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0xA5;
      } else {
        graph = 0x35;
      }
      SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
      func_80155A08(x, y, 2, 0xC);
      primitive = (POLY_GT4 *)g_PrimCursor;
      func_8017A9E0(primitive);
      SetSemiTrans(primitive, 1);
      RotTransPers4(shape, shape + 1, shape + 2, shape + 3,
                    (long *)&primitive->x0, (long *)&primitive->x1,
                    (long *)&primitive->x2, (long *)&primitive->x3, &depth,
                    &flag);
      primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
      primitive->clut = GetClut(0, 0x1FA);
      primitive->u0 = i << 3;
      primitive->u1 = u;
      primitive->u2 = i << 3;
      primitive->v2 = top_row;
      primitive->u3 = u;
      primitive->v3 = top_row;
      primitive->v0 = saved_row + 0x20;
      primitive->v1 = saved_row + 0x20;
      primitive->r0 = shade[0];
      primitive->g0 = shade[0];
      primitive->b0 = shade[0];
      primitive->r1 = shade[0];
      primitive->g1 = shade[0];
      primitive->b1 = shade[0];
      primitive->r2 = shade[-1];
      primitive->g2 = shade[-1];
      primitive->b2 = shade[-1];
      primitive->r3 = shade[-1];
      primitive->g3 = shade[-1];
      primitive->b3 = shade[-1];
      i++;
      u += 8;
      func_80155A08(x, y, 2, 0x34);
    } while (i < 33);
    row -= 8;
    lower_row -= 8;
    lower_phase -= 0x80;
    band--;
    count++;
  } while (band >= 17);
}
