#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work, the name this overlay's exact lifts already use. This body
 * reads the cell once, after the opening draw-mode packet is linked and before
 * the first publish, and reaches its unsigned byte 0x09 from that one load, so
 * the pointer stays live across the two publishes below and the cell is never
 * re-formed. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000, the halfword both rotator samples are
 * multiplied by and the cell this body publishes 0x88 into before the first
 * publish. The original forms its address once with a `lui %hi` plus `addiu
 * %lo` pair into a callee-saved register, reads it with a signed `lh` after
 * every sampler call, keeps a second copy of that address in another
 * callee-saved register for the loop, and reaches the published tint halfword
 * as offset 0x6 of it, so the cell is a live pointer base rather than four
 * separate absolute addresses. The load widths differ the way the exact bmagic
 * fan siblings' do: signed for the scale products and for the quarter division,
 * unsigned for the plain tint bytes. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad fan cell at 0x1F800024, the x halfword this body publishes before
 * entering the loop and then carries forward each pass. The original forms this
 * address into its own callee-saved register with a `lui %hi` plus `addiu %lo`
 * pair and derives the three-vertex shape base by subtracting 0x10 from that
 * register, so the shape is reached through this cell rather than through a
 * symbol at its own first vertex. Its y neighbour at 0x1F800026 and the three z
 * halfwords are published through literal constants instead, the absolute `lui
 * $at` plus `sh` form the compiler emits for a direct fixed-address store. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared phase samplers: both return the sine and cosine of a 12-bit phase in
 * the same fixed-point scale as the `>> 12` products below. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper that sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic fan siblings give their own seed
 * helper. */
void func_8017A97C(void *arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* Seed provenance: the statement order, the operand forms and the scratchpad
 * access spellings are the exact bmagic fan siblings'
 * drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100TintedByWork
 * Byte9WeightedEightTwoOne at 0x801F0790 of bmagic magic006, which shares the
 * pre-loop sampler seed, the sixteen-state `i << 8` phase walk, the
 * RotTransPers3 projection, the tint-then-link tail and the closing second
 * draw-mode selection, and
 * drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius136GradedBlack
 * ToPaletteRowAtWorkByte4ScaledByWorkByte9 at 0x801F0054 of bmagic magic039,
 * which shares the published radius 0x88 and the same two texture-page
 * constant pairs. The literals, the single tint cell at 0x1F800006 with its
 * quarter division, the frame size and the span's own store order are this
 * body's. A payload scan of every extracted binary under out/binaries found
 * this 724-byte span only at its own offset, so the clone lever was declined. */
/* @source 0x801EFAB8
 * @behavior Second draw pass of this overlay's work-record frame driver. Its
 *           only in-image reference is the jal word at 0x801EF464 inside
 *           dispatchWorkByte2FourEntryHandlerThenProjectWorkRecordAndDrawTrian
 *           glesAndPopWhenWorkByte0Bit0AndWorkByte2Set
 *           BmagicMagic04103_801EF3D0, taken while the work object's byte 0 has
 *           bit 0 set and its byte 2 is non-zero, between
 *           projectWorkRecordPositionAndInstallTransformWithZeroRotation and
 *           PopMatrix; no payload word holds 0x801EFAB8. It therefore draws
 *           under the transform its caller installed, pops no matrix of its
 *           own, reads no incoming register, takes no arguments and returns
 *           nothing.
 *           It first picks the draw-mode texture page, 0xA5 when GetGraphType
 *           returns 1 or 2 and 0x35 otherwise, calling it once per test with no
 *           cached answer, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links that
 *           0xC-byte packet under ordering-table slot 5.
 *           It then reads the work-object pointer cell at 0x1F800044 once,
 *           publishes the constant 0x88 into the scratchpad radius halfword
 *           0x1F800000 and the work object's unsigned byte 0x09 shifted left
 *           three into the scratchpad tint halfword 0x1F800006.
 *           The fan is seeded at phase zero: the x halfword 0x1F800024 takes
 *           func_801782FC(0) times the signed radius shifted right twelve, the
 *           y halfword 0x1F800026 takes func_801783C8(0) times the same radius
 *           shifted right twelve, and the three z halfwords 0x1F800028,
 *           0x1F800020 and 0x1F800018 are cleared after it.
 *           The loop runs sixteen passes with the phase advancing by 0x100 each
 *           time, one sixteenth of the 0x1000 turn, so the wedges span the whole
 *           disc at one radius. Each pass takes a fresh g_PrimCursor primitive,
 *           seeds it through func_8017A97C, marks it semi-transparent, clears
 *           the origin vertex pair, carries the previous pass's rim sample pair
 *           into the second vertex, and fills the third vertex pair with the
 *           func_801782FC and func_801783C8 samples of the current phase scaled
 *           by the signed radius and shifted right twelve. The three vertices
 *           are projected through RotTransPers3 into the primitive vertex pairs
 *           at 0x08, 0x10 and 0x18; the origin vertex takes 1 in its red, green
 *           and blue bytes, the red byte of both rim vertices takes the
 *           published tint and their green and blue bytes take one quarter of
 *           it through the signed divide-by-four fixup sequence. Every rim byte
 *           is a fresh read of the tint cell, because the primitive stores
 *           between them may address the same scratchpad page. The 0x1C-byte
 *           POLY_G3 packet is then linked under the same slot 5.
 *           After the loop the body selects the draw-mode texture page again,
 *           this time 0x25 when GetGraphType returns 1 or 2 and 0x15 otherwise,
 *           re-arms SetDrawMode the same way, links a second 0xC-byte packet
 *           under slot 5 and returns.
 *           The 0x40-byte frame keeps $ra, $s4, $s3, $s2, $s1 and $s0 plus the
 *           fifth SetDrawMode argument at 0x10, the sampler outputs at 0x20 and
 *           0x24 and the four RotTransPers3 stack arguments at 0x10 to 0x1C. The
 *           work-object byte 0x09 beyond the tint above and the scratchpad cells
 *           are not established semantically, so they stay offset- and
 *           address-anchored.
 *           Probes: the three cleared z halfwords are what distinguishes the
 *           two seed orders. Clearing them between the two product stores, the
 *           order the bmagic fan siblings write, measured 175 of 181
 *           instructions with the scheduler hoisting all three clears above the
 *           func_801783C8(0) call; moving the two pre-loop pointer copies ahead
 *           of the clears measured 171 of 181. Clearing them after the y store
 *           measured 181 of 181 and byte-equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius136GradedWhiteAtOriginToWorkByte9WeightedEightTwoTwoBmagicMagic04103_801EFAB8(
    void) {
  u8 *work;
  u16 *cells;
  /* Same cell as cells, taken as its own local: the original keeps the
   * pre-loop base in one callee-saved register and the loop base in another
   * because the loop re-uses the first one for the shifted phase, and copies
   * one into the other before the loop, which only a second live value
   * reproduces. */
  u16 *tint;
  u16 *next;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 tpage;
  s32 phase;
  s32 i;
  s16 shade;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  cells = &D_1F800000;
  work = g_battle_work;
  *cells = 0x88;
  *(u16 *)0x1F800006 = work[9] << 3;

  next = &D_1F800024;
  *next = (func_801782FC(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800028 = 0;
  *(u16 *)0x1F800020 = 0;
  *(u16 *)0x1F800018 = 0;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  tint = cells;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    phase = i << 8;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)tint) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)tint) >> 12;

    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);

    shade = 1;
    prim->r0 = shade;
    prim->g0 = shade;
    prim->b0 = shade;
    prim->r1 = tint[3];
    prim->g1 = (u8)((s16)tint[3] / 4);
    prim->b1 = (u8)((s16)tint[3] / 4);
    prim->r2 = tint[3];
    prim->g2 = (u8)((s16)tint[3] / 4);
    prim->b2 = (u8)((s16)tint[3] / 4);

    GpuAppendPrim(5, 0x1C);
    i++;
  } while (i < 0x11);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
}
