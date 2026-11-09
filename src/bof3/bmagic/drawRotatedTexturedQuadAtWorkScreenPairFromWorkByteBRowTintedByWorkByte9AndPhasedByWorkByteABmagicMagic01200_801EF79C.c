#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map.
 * Every one of this body's eleven reads re-materialises the cell with its own
 * lui 0x1F80 plus lw 0x44 pair: the byte 9 tint sample, the byte 0xB texel row
 * sample that shares the phase-0 store, the two halfwords 0x2E and 0x30 of
 * every vertex, the byte 0xB sample of each of the four uv corners, and the two
 * projection words 0x34 and 0x38 of each primitive link share no live base, so
 * the cell is named rather than written as a bare scratchpad constant. The
 * exact same-target siblings advanceWorkByteAStepWorkPositionPairByAngleWord0C
 * AddWord20ToWord14AccumulateIntoHalfword3ECountdownWorkByte9AdvanceWorkByte2
 * OnZeroAndDrawSpriteMode1BmagicMagic01200_801EF3C4 and drawBattlerRecordPoint
 * SnapPositionWhenSlotUnavailableAndOnRangeHitRaiseFlag2000ForwardSlotSetHeading
 * TowardBattleRecordAndAdvanceWorkByte2BmagicMagic01200_801EF188 declare the
 * same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The original forms its address as one
 * lui 0x1F80 plus addiu 0 pair, keeps it in $s1 across all eight rotator calls
 * and re-reads it with lw 0(s1) before each product, so the base is a live
 * pointer and the cell therefore needs a symbol address rather than a folded
 * constant. The value published there, 0x24, is the sprite radius the two
 * shared rotator samples are scaled by and shifted right 12. The exact sibling
 * reseedScratchRotationAndAdvance in the scenario overlay publishes the same
 * cell the same way. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad tint and phase cells at 0x1F800004 and 0x1F800008. Both keep their
 * absolute-address form: every access re-forms the address with one lui 0x1F80
 * plus the store or load at offset 4 or 8, never through a live base register,
 * so each cell needs a symbol at its own address rather than a folded constant.
 * The word at 0x1F800004 is published once from work byte 9 shifted left 3 and
 * read back three times as the quad's r0, g0 and b0, so the store and the three
 * reads each re-form the address. The word at 0x1F800008 carries the published
 * phase angle: the first rotator call of every vertex takes the computed value
 * directly, while the second call reads the cell back after the first call, so
 * that one load survives as a reload. */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair at 0x80159320/0x80159322, links the primitive at g_PrimCursor
 * with the given kind and advances the cursor by the given size. This body
 * links a 0xC-byte draw-mode packet before the quad and a 0x28-byte packet
 * after it, both at the work object's projection words 0x34 and 0x38. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared rotator pair. Both return the sample of the 12-bit angle in the same
 * fixed-point scale as the `>> 12` products below; this body takes the
 * 0x801783C8 sample for every vertex x and the 0x801782FC sample for its y, the
 * same split the exact same-target sibling advanceWorkByteAStepWorkPositionPair
 * ByAngleWord0CAddWord20ToWord14AccumulateIntoHalfword3ECountdownWorkByte9
 * AdvanceWorkByte2OnZeroAndDrawSpriteMode1BmagicMagic01200_801EF3C4 uses. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EF79C
 * @behavior Sprite emitter of this overlay's work-byte-2 handler group: the
 *           exact same-target siblings at 0x801EF188 and 0x801EF3C4 are its
 *           only two in-image callers and both forward work byte 0xB as the
 *           first argument and 0 or 1 as the second, so the second argument is
 *           a per-phase draw selector. It reads the shared graphic type twice,
 *           and when either answer is 1 or 2 it selects the 7-bit draw-mode
 *           field, else the 5-bit one, in both cases with the low two bits of
 *           the selector shifted in and 0x25 or 0x15 or-ed on, and links a
 *           0xC-byte draw-mode packet through SetDrawMode at g_PrimCursor with
 *           dfe 0 and dtd 1. The packet is anchored at the work object's
 *           projection words 0x34 and 0x38 together with kind 2 through
 *           func_80155A08.
 *           It then publishes three scratchpad cells: the fixed radius 0x24 in
 *           scratchpad 0x1F800000, work byte 9 shifted left 3 in the word cell
 *           at 0x1F800004, and per vertex the phase angle (work byte 0xA plus a
 *           per-corner constant, masked to four bits and shifted left 8) in the
 *           word cell at 0x1F800008. It takes a fresh g_PrimCursor POLY_FT4,
 *           seeds it with SetPolyFT4 and marks it semi-transparent with the
 *           selector's low byte, then writes the four rotated corners: each
 *           corner x is the work object's unsigned halfword 0x2E plus the
 *           func_801783C8 sample of the published angle scaled by the published
 *           radius and shifted right 12, and each corner y is the halfword 0x30
 *           plus the same product over func_801782FC. The per-corner phase
 *           constants are 0xD, 0xB, 0x3 and 0x5, which place the corners a
 *           quarter turn apart in the order 3, 5, 0xD, 0xB of the object's
 *           phase byte.
 *           The texture comes from tpage GetTPage(1, selector, 0x340, 0x100) and
 *           clut GetClut(0, 0x1FA), and the uv rectangle from work byte 0xB
 *           shifted left 5 as the u of the left corners and that value plus
 *           0x1F as the u of the right ones, with v 0 on the top edge and 0x40
 *           on the bottom. The quad's r0, g0 and b0 all take the low byte of the
 *           word cell at 0x1F800004, so the tint is work byte 9 shifted left 3.
 *           Finally it links the 0x28-byte polygon at the same work projection
 *           words with kind 2 and returns.
 *           The first argument is never read: the body takes every quantity it
 *           needs from the shared work object and the scratchpad, so the
 *           argument stays declared for the callers' ABI only. The 0x28-byte
 *           frame holds $ra, the three saved registers and the fifth draw-mode
 *           argument at 0x10. No twin exists: every one of the span's seventeen
 *           call targets is a main-executable address outside this overlay, so
 *           the body was searched across all extracted payloads with those
 *           immediates masked and no other copy exists; it was written from the
 *           original bytes.
 * @status partial
 * @match 88.51
 * @residual 231 of 261 instructions match; the build is 1008 bytes against the
 *           original 1044 and the first difference is at +0x0004, an extra
 *           `sw $s2,32($sp)` ahead of the opening `jal`. The original keeps the
 *           incoming draw selector in $s0 and a second, later-narrowed copy of
 *           the same value in $s2, an allocation split of one pseudo that this
 *           body does not reproduce: here the selector stays in a single
 *           register and the $s0 save slot lands after the opening call, which
 *           also moves every `andi ...,$s0,3` onto that register. The four
 *           corner y halves follow from the same shape: the original re-reads
 *           the phase cell after the x store and leaves the rotator call's
 *           delay slot empty, while this build hoists that read into the
 *           preceding load-delay slot and fills the call delay slot with the x
 *           store, two instructions per corner. Operands, constants, call
 *           order, store order, uv and tint bytes and the 0x28-byte frame are
 *           the original's. Swept and rejected: holding the scale product in
 *           its own local (199 of 261), swapping the addition's operands on all
 *           eight corners and on the four y corners only (both 231, neutral),
 *           and an explicit second selector local for the two byte conversions
 *           (231, coalesced away). Both residuals are allocation and scheduling
 *           facts of this one body, not source spelling. */
void drawRotatedTexturedQuadAtWorkScreenPairFromWorkByteBRowTintedByWorkByte9AndPhasedByWorkByteABmagicMagic01200_801EF79C(
    u8 index, s32 mode) {
  POLY_FT4 *primitive;
  s32 *scale;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = ((mode & 3) << 7) | 0x25;
  } else {
    graph = ((mode & 3) << 5) | 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34),
                *(s32 *)(g_battle_work + 0x38), 2, 0xC);

  scale = &D_1F800000;
  *scale = 0x24;
  D_1F800004 = g_battle_work[9] << 3;

  primitive = (POLY_FT4 *)g_PrimCursor;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, (u8)mode);

  D_1F800008 = ((g_battle_work[0xA] + 0xD) & 0xF) << 8;
  primitive->x0 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(D_1F800008) * *scale) >> 12);
  primitive->y0 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(D_1F800008) * *scale) >> 12);

  D_1F800008 = ((g_battle_work[0xA] + 0xB) & 0xF) << 8;
  primitive->x1 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(D_1F800008) * *scale) >> 12);
  primitive->y1 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(D_1F800008) * *scale) >> 12);

  D_1F800008 = ((g_battle_work[0xA] + 0x3) & 0xF) << 8;
  primitive->x2 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(D_1F800008) * *scale) >> 12);
  primitive->y2 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(D_1F800008) * *scale) >> 12);

  D_1F800008 = ((g_battle_work[0xA] + 0x5) & 0xF) << 8;
  primitive->x3 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(D_1F800008) * *scale) >> 12);
  primitive->y3 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(D_1F800008) * *scale) >> 12);

  primitive->tpage = GetTPage(1, (u8)mode, 0x340, 0x100);
  primitive->clut = GetClut(0, 0x1FA);

  primitive->u0 = g_battle_work[0xB] << 5;
  primitive->v0 = 0;
  primitive->u1 = (g_battle_work[0xB] << 5) + 0x1F;
  primitive->v1 = 0;
  primitive->u2 = g_battle_work[0xB] << 5;
  primitive->v2 = 0x40;
  primitive->u3 = (g_battle_work[0xB] << 5) + 0x1F;
  primitive->v3 = 0x40;

  primitive->r0 = D_1F800004;
  primitive->g0 = D_1F800004;
  primitive->b0 = D_1F800004;

  func_80155A08(*(s32 *)(g_battle_work + 0x34),
                *(s32 *)(g_battle_work + 0x38), 2, 0x28);
}
