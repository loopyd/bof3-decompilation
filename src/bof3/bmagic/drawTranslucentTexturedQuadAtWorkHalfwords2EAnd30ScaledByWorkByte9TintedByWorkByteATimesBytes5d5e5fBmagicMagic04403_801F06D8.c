#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map.
 * This body reads the cell twelve times and never keeps one base across them.
 * The eight corner expressions and the radius seed each dereference it once, and
 * the original gives each of them its own lui 0x1F80 plus lw 0x44 pair, because
 * a call sits between every pair of them. Both func_80155A08 call sites pass two
 * quantities read from one cell load, so those arguments dereference the cell
 * twice inside one expression and the compiler merges the two reads into the
 * single lui/lw pair the original shows, landing in $v0. The tint block is the
 * only site that binds the load to a local, because each of its three products
 * stores a scratchpad halfword that may alias this pointer, so a fresh cell read
 * per product would emit two extra lui/lw pairs where the original keeps one
 * base in $v1. The cell therefore stays unqualified; the volatile spelling the
 * family sibling consumeRandomDecayWorkHalfwords2EAnd30AdvanceByte9By4
 * AdvanceWorkByte2At10BmagicMagic04403_801F05AC uses for its per-access reloads
 * measured 181 of 206 here with every base in $a1 and the tint products in
 * $a0/$v1. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The original forms its address as one
 * lui 0x1F80 plus addiu 0 pair, keeps that base in $s1 across all eight rotator
 * calls and re-reads the cell with lh 0($s1) before each product, while the
 * single publish of the radius is a sh through the same base. The base is
 * therefore a live pointer to a signed halfword cell, not a folded constant.
 * The exact sibling drawRotatedTexturedQuadAtWorkScreenPairFromWorkByteBRow
 * TintedByWorkByte9AndPhasedByWorkByteABmagicMagic01200_801EF79C publishes the
 * same cell the same way. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad halfword cells at 0x1F80000A, 0x1F80000C and 0x1F80000E. Each is
 * published once as work byte 0xA times one of the signed work bytes 0x5D,
 * 0x5E and 0x5F, and each keeps the absolute-address form: the store and the
 * read both re-form the address with one lui 0x1F80 plus the store or load at
 * its own offset, never through a live base register, so the cells are named
 * at their own addresses rather than folded. */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F80000E; /* @source 0x1F80000E @kind data */

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair at 0x80159320/0x80159322, links the primitive at g_PrimCursor
 * with the given kind and advances the cursor by the given size. This body
 * links a 0xC-byte draw-mode packet at the work object projection words 0x34
 * and 0x38 before the quad and the 0x28-byte quad itself after it, both with
 * kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared rotator pair at 0x801782FC and 0x801783C8, both returning the sample
 * of the 12-bit angle in the fixed-point scale the two bodies of this family
 * shift right 12 over the published radius. This body takes the 0x801783C8
 * sample for every corner x and the 0x801782FC sample for its y, the same split
 * the family sibling at 0x801EF79C uses. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query that selects the draw-mode texture page argument. */
s32 GetGraphType(void);

/* @source 0x801F06D8
 * @behavior Quad emitter of this overlay's work-byte-2 handler group. Its only
 *           in-image caller is the dispatcher
 *           dispatchWorkByte2HandlerThenDrawWhenActiveBmagicMagic04403_801F0478
 *           at 0x801F0478, which reaches it with no arguments once the work
 *           object byte 0x0 and the handler index byte 0x2 are both non-zero,
 *           and the body reads no incoming register, so it takes none.
 *           It reads the shared graphic type twice and, when either answer is 1
 *           or 2, selects the draw-mode texture page 0x125, else 0x55; the two
 *           values are the family formula ((2 & 3) << 7) | 0x25 and
 *           ((2 & 3) << 5) | 0x15 with the selector fixed at 2. It links a
 *           0xC-byte draw-mode packet through SetDrawMode at g_PrimCursor with
 *           dfe 0 and dtd 1, anchored at the work object projection words 0x34
 *           and 0x38 with kind 2 through func_80155A08.
 *           It then publishes the radius work byte 9 shifted left 1 into the
 *           scratchpad halfword at 0x1F800000, takes a fresh g_PrimCursor
 *           POLY_FT4, seeds it with SetPolyFT4, marks it semi-transparent and
 *           writes four corners around the work object screen halfwords 0x2E
 *           (x) and 0x30 (y): each corner x is that halfword plus the
 *           func_801783C8 sample of the corner angle scaled by the published
 *           radius and shifted right 12, and each corner y is the halfword 0x30
 *           plus the func_801782FC sample of the same angle. The corner angles
 *           are the constants 0x200, 0x600, 0xE00 and 0xA00, eighth turns of the
 *           12-bit circle, which place the four corners of a diamond whose
 *           half-diagonal is the published radius around the work screen pair.
 *           The texture is tpage GetTPage(0, 2, 0x340, 0x100) and clut
 *           GetClut(0x20, 0x1FA), and the uv rectangle is the fixed 0x20-by-0x20
 *           tile at (0x80, 0x20): u 0x80 on the corners 0 and 2, u 0xA0 on the
 *           corners 1 and 3, v 0x20 on the corners 0 and 1 and v 0x40 on the
 *           corners 2 and 3. The quad tint is the work byte 0xA index times one
 *           of the signed work bytes 0x5D, 0x5E and 0x5F for red, green and
 *           blue; the three products are published in the scratchpad halfwords
 *           0x1F80000A, 0x1F80000C and 0x1F80000E and read back for the colour
 *           bytes, and since the primitive pointer may alias those cells the two
 *           reads that follow the red store reload their cells.
 *           Finally it links the 0x28-byte polygon at the same work projection
 *           words with kind 2 and returns, having stored nothing but the
 *           scratchpad cells and the primitive itself. The 0x28-byte frame holds
 *           $ra, $s1, $s0 and the fifth draw-mode argument at 0x10.
 *           No twin exists: all seventeen call targets of this span resolve in
 *           the shared main executable and its data references are only the
 *           shared cursor cell at 0x8014598C and the scratchpad, so the span
 *           was searched across every extracted payload under out/binaries with
 *           each call and jump immediate masked, decoded in the payload byte
 *           order, and no other copy exists. It was written from the original
 *           bytes with the shape of the family sibling at 0x801EF79C, which
 *           carries its own partial measurement and so served as a shape donor
 *           rather than as this body seed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimesBytes5d5e5fBmagicMagic04403_801F06D8(
    void) {
  POLY_FT4 *primitive;
  s16 *scale;
  s32 graph;
  u8 *work;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x125;
  } else {
    graph = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0xC);

  scale = &D_1F800000;
  primitive = (POLY_FT4 *)g_PrimCursor;
  *scale = g_battle_work[9] << 1;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);

  primitive->x0 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0x200) * *scale) >> 12);
  primitive->y0 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0x200) * *scale) >> 12);
  primitive->x1 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0x600) * *scale) >> 12);
  primitive->y1 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0x600) * *scale) >> 12);
  primitive->x2 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0xE00) * *scale) >> 12);
  primitive->y2 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0xE00) * *scale) >> 12);
  primitive->x3 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0xA00) * *scale) >> 12);
  primitive->y3 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0xA00) * *scale) >> 12);

  primitive->tpage = GetTPage(0, 2, 0x340, 0x100);
  primitive->clut = GetClut(0x20, 0x1FA);

  primitive->u0 = 0x80;
  primitive->v0 = 0x20;
  primitive->u1 = 0xA0;
  primitive->v1 = 0x20;
  primitive->u2 = 0x80;
  primitive->v2 = 0x40;
  primitive->u3 = 0xA0;
  primitive->v3 = 0x40;

  work = g_battle_work;
  D_1F80000A = work[0xA] * ((s8 *)work)[0x5D];
  D_1F80000C = work[0xA] * ((s8 *)work)[0x5E];
  D_1F80000E = work[0xA] * ((s8 *)work)[0x5F];
  primitive->r0 = D_1F80000A;
  primitive->g0 = D_1F80000C;
  primitive->b0 = D_1F80000E;

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0x28);
}
