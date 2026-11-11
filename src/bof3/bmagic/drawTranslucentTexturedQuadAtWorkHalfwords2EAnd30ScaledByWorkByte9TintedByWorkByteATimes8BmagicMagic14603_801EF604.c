#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original re-forms the cell address with one lui 0x1F80
 * plus lw 0x44 for every read of the object, because a call always sits between
 * the reads, so no base is kept across a call. The one site that holds the
 * loaded pointer in a register is the scratchpad publication block, whose two
 * halfword stores through a constant address may alias the object; the original
 * keeps that pointer in $v1 across both stores rather than reloading the cell,
 * so the publication block binds it to a local and no other site does. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The original forms its address as one
 * lui 0x1F80 plus addiu 0 pair, keeps that base in $s1 across all eight rotator
 * calls and re-reads the cell with lh 0($s1) before each product, while the
 * single publish of the radius is a sh through the same base. The base is
 * therefore a live pointer to a signed halfword cell, not a folded constant. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad tint cell at 0x1F800004. The original publishes work byte 0xA
 * shifted left 3 into it as one halfword store through a constant address
 * (lui $at, 0x1F80 / sh), and every later read re-forms the address with one
 * lui 0x1F80 plus lhu at offset 4, never through a live base register, so the
 * cell is named at its own address rather than folded. Each of the three reads
 * follows a byte store through the primitive pointer, which may alias the cell,
 * so the original reloads it for every store instead of sharing one value. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

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
 * WorkByteATimes10And9BmagicMagic00903_801EF3B8 and drawRotatedTexturedQuadAtWo
 * rkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimes8AtTopCorners
 * BmagicMagic15903_801EF08C use. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EF604
 * @behavior Quad emitter of this overlay's work-byte-2 draw step. Its only
 *           in-image reference is the jal at 0x801EF3A0 in func_801EF340, which
 *           dispatches the handler table through work byte 0x2 and then reaches
 *           this body with no arguments once work bytes 0x0 and 0x2 are both
 *           non-zero; the body reads no incoming register, so it takes none.
 *           It reads the shared graphic type twice and selects the draw-mode
 *           texture page 0x125 when either answer is 1 or 2, else 0x55, then
 *           links a 0xC-byte draw-mode packet through SetDrawMode at
 *           g_PrimCursor with dfe 0, dtd 1 and tw 0 anchored at the work object
 *           projection words 0x34 and 0x38 with kind 2 through func_80155A08.
 *           It then publishes two scratchpad cells from the work object: work
 *           byte 9 shifted left 1 as the radius in the signed halfword at
 *           0x1F800000, and work byte 0xA shifted left 3 in the halfword at
 *           0x1F800004. It takes a fresh g_PrimCursor POLY_FT4, seeds it through
 *           SetPolyFT4, marks it semi-transparent and writes four corners around
 *           the work object screen halfwords 0x2E (x) and 0x30 (y): each corner
 *           x is that halfword plus the func_801783C8 sample of the corner angle
 *           scaled by the published radius and shifted right 12, and each corner
 *           y is the halfword 0x30 plus the func_801782FC sample of the same
 *           angle. The corner angles are 0x200, 0x600, 0xE00 and 0xA00, eighth
 *           turns of the 12-bit circle, so the four corners sit on one
 *           half-diagonal-equals-radius diamond around the work screen pair.
 *           The texture is tpage GetTPage(0, 2, 0x340, 0x100) and clut
 *           GetClut(0x20, 0x1FA), and the uv rectangle is the 0x20-by-0x20 window
 *           at (0x80, 0x20): u 0x80 on corners 0 and 2, u 0xA0 on corners 1 and
 *           3, v 0x20 on corners 0 and 1 and v 0x40 on corners 2 and 3. The flat
 *           quad colour is grey: red, green and blue each read the scratchpad
 *           halfword 0x1F800004 back and truncate it to a byte, so all three
 *           channels carry work byte 0xA shifted left 3, eight times that byte,
 *           and each of the three stores re-reads the cell because the primitive
 *           pointer may alias it.
 *           Finally it links the 0x28-byte polygon at the same work projection
 *           words with kind 2 and returns, having stored nothing but the two
 *           scratchpad cells and the primitive itself. The 0x28-byte frame holds
 *           $ra at 0x20, $s1 at 0x1C, $s0 at 0x18 and the fifth draw-mode
 *           argument at 0x10.
 *           No twin exists: the 0x2FC payload bytes were searched across all 396
 *           extracted payloads under out/binaries and appear byte-identical only
 *           at this span's own offset 0xA04; with each j and jal word masked in
 *           payload byte order the span also equals emi/bmagic/magic145/03
 *           payload offset 0x518, runtime 0x801EF118, in every word but the
 *           internal j at +0x34, which differs by exactly the 0x4EC address delta
 *           between the two spans, and that sibling's own row is still asm, so it
 *           holds no source to clone. The body was written from the original
 *           bytes with the shape of the exact family sibling at
 *           emi/bmagic/magic009/03 0x801EF3B8, whose draw-mode block, projection
 *           link, radius publish, corner expressions, uv window, clut pair and
 *           trailing 0x28 link are instruction-identical modulo this body's own
 *           constants. Every literal, passed constant and cell address here was
 *           read from this span's own bytes.
 * @status partial
 * @match 98.96
 * @residual 190/192 instructions and 768 of 764 bytes. Only the publication
 *           block differs: the build loads g_PrimCursor before work byte 9,
 *           then leaves a load-delay nop before the radius shift. The original
 *           loads work byte 9 before g_PrimCursor, filling that delay with the
 *           cursor address and load. Every other instruction matches.
 */
void drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimes8BmagicMagic14603_801EF604(
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

  do {
    work = g_battle_work;
    scale = &D_1F800000;
  } while (0);
  primitive = (POLY_FT4 *)g_PrimCursor;
  *scale = work[9] << 1;
  D_1F800004 = work[0xA] << 3;
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

  /* The clut store is the one access in this body the original commits straight
   * after its call: li a0,0x80 fills the call delay slot's successor and the sh
   * lands one instruction later, ahead of the whole uv and colour group. Written
   * as a plain member store the build sinks that sh below the uv group, keeps the
   * GetClut answer live in $v0 and materialises the uv constants in $a1/$v1/$a0,
   * which measured 159 of 191 instructions against this span. The family sibling
   * drawRotatedTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedBy
   * WorkByteATimes8AtTopCornersBmagicMagic15903_801EF08C recorded the same
   * residual and closed it with a volatile-qualified store, so the same form is
   * used here. */
  primitive->tpage = GetTPage(0, 2, 0x340, 0x100);
  *(volatile u16 *)&primitive->clut = GetClut(0x20, 0x1FA);

  primitive->u0 = 0x80;
  primitive->v0 = 0x20;
  primitive->u1 = 0xA0;
  primitive->v1 = 0x20;
  primitive->u2 = 0x80;
  primitive->v2 = 0x40;
  primitive->u3 = 0xA0;
  primitive->v3 = 0x40;

  primitive->r0 = D_1F800004;
  primitive->g0 = D_1F800004;
  primitive->b0 = D_1F800004;

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0x28);
}
