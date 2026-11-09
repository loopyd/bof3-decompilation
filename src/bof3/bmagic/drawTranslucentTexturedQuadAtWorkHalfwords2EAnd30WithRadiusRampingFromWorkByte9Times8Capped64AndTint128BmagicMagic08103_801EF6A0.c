#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work the way the exact siblings of this target declare it. The
 * body reads the cell three times -- once for the radius ramp, once for the two
 * screen halfwords it publishes into scratchpad, and once for the trailing link
 * -- each with its own lui / lw pair, because a call sits between the reads.
 * The ramp mentions the cell directly instead of through the local: it names
 * byte 9 in the test and again in the then branch, so the build merges both
 * byte reads into one lbu before the compare and keeps the cell pointer in a
 * register of its own. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The original forms its address with one
 * lui 0x1F80 per access and reads it with lh straight into the product that
 * follows each rotator return, so the cell is a named signed halfword read in
 * place rather than a live base pointer. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad screen-pair cells at 0x1F800004 and 0x1F800006. Each is published
 * once from the work object halfwords 0x2E and 0x30 and then read once per
 * corner through its own lui plus lhu, never through a live base, so both stay
 * named cells. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Scratchpad tint cell at 0x1F800008. It is seeded 0x80 once and then read back
 * before every one of the twelve colour stores, through the held base $s0,
 * which is why the body keeps it in a register across the seeding, both texture
 * accessors and the polygon seeder. The primitive pointer it is stored through
 * may alias the cell, so every read is a fresh lhu. */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair and links the primitive at g_PrimCursor with the given kind,
 * advancing the cursor by the given size. This body links the 0xC-byte
 * draw-mode packet at the work object projection words 0x34 and 0x38 before the
 * quad and the finished 0x34-byte quad at the same words after it, both with
 * kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared rotator pair at 0x801782FC and 0x801783C8, both returning the
 * fixed-point sample of the 12-bit angle this body shifts right 12 over the
 * published radius. Unlike the family sibling whose corners take the 0x801783C8
 * sample for x, this body takes the 0x801782FC sample for every corner x and the
 * 0x801783C8 sample for every corner y. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed at 0x8017A9E0, sitting in the SDK library slot between
 * SetPolyG4 at 0x8017A9CC and SetSprt8 at 0x8017A9F4. The object this body
 * builds is 0x34 bytes and every field it writes sits at the POLY_GT4 offsets
 * (colours at 0x4/0x10/0x1C/0x28, vertices at 0x8/0x14/0x20/0x2C, uv pairs at
 * 0xC/0x18/0x24/0x30, clut 0xE, tpage 0x1A). The shared SDK map carries no name
 * for it, so it keeps its address-qualified prototype. */
void func_8017A9E0(POLY_GT4 *arg0);

/* @source 0x801EF6A0
 * @behavior Translucent textured-quad draw pass of this overlay's action-slot
 *           handler group, emitted between the work-object step handler at
 *           0x801EF614 and the record claim at 0x801EFA20.
 *           No word of this payload, of the main executable or of any other
 *           extracted payload under out/binaries holds this address and no jal
 *           anywhere reaches it, so no call site establishes its role; the
 *           boundary comes from its own bytes alone, one 0x380-byte body with a
 *           prologue that saves $ra, $s1 and $s0 and the fifth draw-mode
 *           argument at 0x10 of a 0x28-byte frame, and a single return. It
 *           reads no incoming register, so it takes no arguments.
 *           It reads the shared graphic type twice and selects the draw-mode
 *           texture page 0x125 when either answer is 1 or 2, else 0x55, then
 *           links a 0xC-byte draw-mode packet through SetDrawMode at
 *           g_PrimCursor with dfe 0 and dtd 1, anchored at the work object
 *           projection words 0x34 and 0x38 with kind 2 through func_80155A08.
 *           It takes the next g_PrimCursor POLY_GT4, seeds it through the
 *           shared seeder at 0x8017A9E0, and publishes three scratchpad cells
 *           from the work object: the signed halfword at 0x1F800000 takes
 *           (work byte 9 + 1) * 8 while that byte is below 8 and 0x40 once it
 *           reaches 8, which ramps the quad's radius over the first eight ticks
 *           of the counter and then holds it at 0x40; the halfword at
 *           0x1F800004 takes the work object screen halfword 0x2E and the one at
 *           0x1F800006 takes 0x30; and the halfword at 0x1F800008 is seeded 0x80
 *           and read back for every colour store. The ramp names the cell
 *           directly in both branches rather than through the work pointer it
 *           later re-reads, which is what lets the build merge its two byte
 *           reads into one lbu before the compare; a local pointer form
 *           measured 221 of 224 instructions with the cell pointer and the byte
 *           sharing one register.
 *           It writes four corners around the published screen pair: each
 *           corner x is the 0x1F800004 halfword plus the func_801782FC sample of
 *           the corner angle scaled by the published radius and shifted right
 *           12, and each corner y is the 0x1F800006 halfword plus the
 *           func_801783C8 sample of the same angle. The corner angles are 0xC00,
 *           0x800, 0 and 0x400, three-quarter, half and quarter turns of the
 *           12-bit circle with the zero angle third, which places the four
 *           corners of a half-diagonal-equals-radius diamond around the screen
 *           pair.
 *           The texture is tpage GetTPage(0, 2, 0x340, 0x100) and clut
 *           GetClut(0, 0x1FA), and the uv rectangle is the 0x1E-by-0x1E window
 *           at (1, 1): u 1 on corners 0 and 2, u 0x1F on corners 1 and 3, v 1 on
 *           corners 0 and 1 and v 0x1F on corners 2 and 3. All twelve colour
 *           bytes are the 0x1F800008 halfword, so the quad is a flat 0x80 tint
 *           marked semi-transparent through SetSemiTrans on the primitive
 *           pointer. Finally it links the finished 0x34-byte polygon at the same
 *           work projection words with kind 2 and returns, having stored nothing
 *           but the three scratchpad cells and the primitive itself.
 *           Shape evidence: the family sibling
 *           drawRotatedTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9Tinted
 *           ByWorkByteATimes8AtTopCornersBmagicMagic15903_801EF08C at
 *           emi/bmagic/magic159/03@0x801EF08C carries the same graphic-type
 *           selection, the same draw-mode link, the same seeder and rotator
 *           pairing and the same trailing 0x34-byte link; the span at
 *           emi/bmagic/magic046/03 offset 0xC14 repeats the rotator chain but
 *           differs in 235 of its 896 bytes, so it is shape evidence and not a
 *           clone source. Every published cell, corner angle, uv constant,
 *           texture page and tint value here follows this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30WithRadiusRampingFromWorkByte9Times8Capped64AndTint128BmagicMagic08103_801EF6A0(
    void) {
  POLY_GT4 *primitive;
  s16 radius;
  s32 graph;
  u16 *tint;
  u8 *work;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x125;
  } else {
    graph = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0xC);

  primitive = (POLY_GT4 *)g_PrimCursor;
  func_8017A9E0(primitive);

  if (g_battle_work[9] < 8) {
    D_1F800000 = (g_battle_work[9] + 1) << 3;
  } else {
    D_1F800000 = 0x40;
  }
  tint = &D_1F800008;
  work = g_battle_work;
  *tint = 0x80;
  D_1F800004 = *(u16 *)(work + 0x2E);
  D_1F800006 = *(u16 *)(work + 0x30);

  primitive->x0 = D_1F800004 + ((func_801782FC(0xC00) * D_1F800000) >> 12);
  primitive->y0 = D_1F800006 + ((func_801783C8(0xC00) * D_1F800000) >> 12);
  primitive->x1 = D_1F800004 + ((func_801782FC(0x800) * D_1F800000) >> 12);
  primitive->y1 = D_1F800006 + ((func_801783C8(0x800) * D_1F800000) >> 12);
  primitive->x2 = D_1F800004 + ((func_801782FC(0) * D_1F800000) >> 12);
  primitive->y2 = D_1F800006 + ((func_801783C8(0) * D_1F800000) >> 12);
  primitive->x3 = D_1F800004 + ((func_801782FC(0x400) * D_1F800000) >> 12);
  primitive->y3 = D_1F800006 + ((func_801783C8(0x400) * D_1F800000) >> 12);

  primitive->tpage = GetTPage(0, 2, 0x340, 0x100);
  primitive->clut = GetClut(0, 0x1FA);

  primitive->u0 = 1;
  primitive->v0 = 1;
  primitive->u1 = 0x1F;
  primitive->v1 = 1;
  primitive->u2 = 1;
  primitive->v2 = 0x1F;
  primitive->u3 = 0x1F;
  primitive->v3 = 0x1F;

  primitive->r0 = *tint;
  primitive->g0 = *tint;
  primitive->b0 = *tint;
  primitive->r1 = *tint;
  primitive->g1 = *tint;
  primitive->b1 = *tint;
  primitive->r2 = *tint;
  primitive->g2 = *tint;
  primitive->b2 = *tint;
  primitive->r3 = *tint;
  primitive->g3 = *tint;
  primitive->b3 = *tint;

  SetSemiTrans(primitive, 1);

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0x34);
}
