#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map.
 * The original re-forms the cell address with one lui 0x1F80 plus lw 0x44 before
 * every read of the object, because a call sits between the reads, so no base is
 * kept across a call. The tint block re-reads the cell for each of its two
 * products, because the byte stores through the primitive pointer may alias the
 * object, and the original therefore materialises two separate lui/lw pairs
 * there rather than one shared base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The original forms its address as one
 * lui 0x1F80 plus addiu 0 pair, keeps that base in $s1 across the whole quad,
 * publishes the radius with a sw and re-reads it with lw before each of the
 * eight corner products, so the base is a live pointer to a signed word cell
 * rather than a folded constant. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair at 0x80159320/0x80159322, links the primitive at g_PrimCursor
 * with the given kind and advances the cursor by the given size. This body links
 * a 0xC-byte draw-mode packet at the work object projection words 0x34 and 0x38
 * before the quad and the 0x28-byte quad itself after it, both with kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared rotator pair at 0x801782FC and 0x801783C8, both returning the sample of
 * the 12-bit angle in the fixed-point scale this body shifts right 12 over the
 * published radius. This body takes the 0x801783C8 sample for every corner x and
 * the 0x801782FC sample for every corner y, the same split the family siblings
 * drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedBy
 * WorkByteATimesBytes5d5e5fBmagicMagic04403_801F06D8 and drawRotatedTextured
 * QuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimes8AtTopCor
 * nersBmagicMagic15903_801EF08C use. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EF3B8
 * @behavior Quad emitter of this overlay's work-byte-2 handler group. Its only
 *           caller is the dispatcher
 *           dispatchWorkByte2HandlerThenDrawWhenWorkBytes0And2SetBmagicMagic
 *           00903_801EF120 at 0x801EF120, which reaches it with no arguments
 *           once the work object bytes 0x0 and 0x2 are both non-zero, and the
 *           body reads no incoming register, so it takes none.
 *           It reads the shared graphic type twice and selects the draw-mode
 *           value 0xA5 when either answer is 1 or 2, else 0x35, then links a
 *           0xC-byte draw-mode packet through SetDrawMode at g_PrimCursor with
 *           dfe 0 and dtd 1 anchored at the work object projection words 0x34
 *           and 0x38 with kind 2 through func_80155A08.
 *           It then publishes work byte 9 shifted left 1 into the signed word
 *           cell at 0x1F800000, takes a fresh g_PrimCursor POLY_FT4, seeds it
 *           through SetPolyFT4, marks it semi-transparent and writes four corners
 *           around the work object screen halfwords 0x2E (x) and 0x30 (y): each
 *           corner x is that halfword plus the func_801783C8 sample of the corner
 *           angle scaled by the published radius and shifted right 12, and each
 *           corner y is the halfword 0x30 plus the func_801782FC sample of the
 *           same angle. The corner angles are 0x200, 0x600, 0xE00 and 0xA00,
 *           eighth turns of the 12-bit circle, so each corner x alternates
 *           between the halfword and the halfword offset by the radius sample
 *           and the four corners sit on one half-diagonal-equals-radius diamond
 *           around the work screen pair.
 *           The texture is tpage GetTPage(0, 1, 0x340, 0x100) and clut
 *           GetClut(0x20, 0x1FA), and the uv rectangle is the 0x20-by-0x20 window
 *           at (0x80, 0x20): u 0x80 on corners 0 and 2, u 0xA0 on corners 1 and
 *           3, v 0x20 on corners 0 and 1 and v 0x40 on corners 2 and 3. The flat
 *           quad tint comes from work byte 0xA: red is that byte times 10, green
 *           is 0 and blue is that byte times 9, and since the primitive pointer
 *           may alias the work object each of the two products reloads the
 *           pointer cell.
 *           Finally it links the 0x28-byte polygon at the same work projection
 *           words with kind 2 and returns, having stored nothing but the
 *           scratchpad radius cell and the primitive itself. The 0x28-byte frame
 *           holds $ra at 0x20, $s1 at 0x1C, $s0 at 0x18 and the fifth draw-mode
 *           argument at 0x10.
 *           No twin exists: all seventeen call targets of this span and every
 *           symbol it touches resolve in the shared main executable, so only its
 *           carried constants could differ, and the 190-word span searched across
 *           all 396 extracted payloads under out/binaries with each j and jal
 *           immediate masked in payload byte order matched only its own offset
 *           0x7B8. It was written from the original bytes with the shape of the
 *           family sibling at emi/bmagic/magic044/03@0x801F06D8, which shares the
 *           draw-mode block, the POLY_FT4 field set, the uv window and the trailing
 *           link size, and with the draw-mode constants of the sibling at
 *           emi/bmagic/magic159/03@0x801EF08C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimes10And9BmagicMagic00903_801EF3B8(
    void) {
  POLY_FT4 *primitive;
  s32 *scale;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
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

  primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
  primitive->clut = GetClut(0x20, 0x1FA);

  primitive->u0 = 0x80;
  primitive->v0 = 0x20;
  primitive->u1 = 0xA0;
  primitive->v1 = 0x20;
  primitive->u2 = 0x80;
  primitive->v2 = 0x40;
  primitive->u3 = 0xA0;
  primitive->v3 = 0x40;

  primitive->r0 = g_battle_work[0xA] * 10;
  primitive->g0 = 0;
  primitive->b0 = g_battle_work[0xA] * 9;

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0x28);
}
