#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target map
 * row g_battle_work the way the exact siblings of this target declare it. The
 * body takes four separate reads of the cell - one for the arguments of the
 * draw-mode link and one for the two arguments of the trailing polygon link,
 * both of which pass the work record projection words 0x34 and 0x38, one for
 * work byte 0x9 of the radius test, and one for work byte 0xA and the two
 * screen halfwords 0x2E and 0x30 - because every call in between may
 * republish the cell, so the original re-materialises the address with its own
 * lui / lw pair at each read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. It is seeded once from work byte 0x9
 * and then read back with a signed halfword load into each of the eight corner
 * products, so the scale is taken through the signed view while the seed is a
 * plain halfword store. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad screen-pair cells at 0x1F800004 and 0x1F800006. Each is published
 * once from work record halfwords 0x2E and 0x30 and then read once per corner
 * through its own lui plus lhu, never through a live base. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Scratchpad tint cell at 0x1F800008. It is seeded once from work byte 0xA and
 * read back before every one of the twelve colour stores through the held base
 * $s0, which is why the body keeps the cell in a register across both texture
 * accessors and the seeder call. The primitive stores may alias the cell, so
 * every read is a fresh lhu. */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared primitive linker: it takes the work record projection pair, the
 * primitive kind and the packet size, links the primitive at g_PrimCursor and
 * advances the cursor by that size. This body links one 0xC-byte draw-mode
 * packet before the quad and the finished 0x34-byte polygon after it, both with
 * kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive seed at 0x8017A9E0, sitting in the SDK library slot between
 * SetPolyG4 at 0x8017A9CC and SetSprt8 at 0x8017A9F4. The object this body
 * builds is 0x34 bytes and every field it writes sits at the POLY_GT4 offsets
 * (colours at 0x4/0x10/0x1C/0x28, vertices at 0x8/0x14/0x20/0x2C, uv pairs at
 * 0xC/0x18/0x24/0x30, clut 0xE, tpage 0x1A), so the prototype takes the typed
 * pointer as the exact family sibling does. */
void func_8017A9E0(POLY_GT4 *arg0);

/* Shared rotator pair at 0x801782FC and 0x801783C8, both returning the
 * fixed-point sample of a 12-bit angle that each corner product shifts right by
 * twelve. This body takes the 0x801782FC sample for every corner x and the
 * 0x801783C8 sample for every corner y. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EF7E8
 * @behavior Translucent textured-quad draw pass of the second stage of this
 *           overlay draw state machine. Its only in-image callers are the two
 *           stage handlers at 0x801EF540 and 0x801EF660, entry 1 and entry 2 of
 *           the three-entry code pointer table at 0x801EEC1C, and both call it
 *           after advancing the work record projection words 0x34 and 0x38, so
 *           the quad is drawn at the projection pair they published. It reads no
 *           incoming register, takes no arguments and returns nothing through
 *           the jr $ra that restores the 0x28-byte frame.
 *           It first selects the draw-mode texture page argument 0xA5 when the
 *           shared graphic type is 1 or 2 and 0x35 otherwise, installs it at
 *           g_PrimCursor with dfe 0 and dtd 1, and links a 0xC-byte draw-mode
 *           packet at work record words 0x34 and 0x38 with kind 2.
 *           It then seeds four scratchpad cells: the signed halfword at
 *           0x1F800000 takes six times work byte 0x9 plus one while that byte is
 *           below eight and twice the byte plus 0x20 otherwise, the halfword at
 *           0x1F800008 takes twenty times work byte 0xA as the flat tint, the
 *           halfword at 0x1F800004 takes work record halfword 0x2E and the one
 *           at 0x1F800006 takes work record halfword 0x30.
 *           It emits one semi-transparent POLY_GT4 at g_PrimCursor, seeded
 *           through the shared seeder at 0x8017A9E0: the four corners lie on the
 *           circle of the radius halfword around that screen pair, taken at
 *           phases 0xC00, 0x800, 0 and 0x400 in vertex order, with the x of each
 *           corner the 0x801782FC sample of its phase scaled by the radius and
 *           shifted right twelve and the y the 0x801783C8 sample scaled the same
 *           way. Its uv pair is 1,1 on corners 0 and 2, u 0x1F with v 1 on corner
 *           1 and u 1 with v 0x1F on corner 2, and 0x1F,0x1F on corner 3, with
 *           the u and v halves each written as their own byte store. Its texture
 *           page is GetTPage(0, 1, 0x340, 0x100) and its clut GetClut(0, 0x1FA).
 *           All twelve colour bytes are the tint halfword, each stored from its
 *           own read of the cell because the primitive stores may alias it.
 *           Finally it links the finished 0x34-byte polygon at the same work
 *           record words with kind 2.
 *           The 0x28-byte frame keeps ra at 0x20, the primitive pointer at 0x1C,
 *           the tint base at 0x18 and the fifth draw-mode argument at 0x10, and
 *           the body writes no state outside the four scratchpad cells, the
 *           cursor and the two primitives it links; the work record offsets and
 *           the cell roles stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30WithRadiusSixTimesWorkByte9PlusOneOrTwiceWorkByte9Plus32AndTintTwentyTimesWorkByteABmagicMagic04603_801EF7E8(
    void) {
  POLY_GT4 *primitive;
  s32 graph;
  u16 *tint;
  u8 *work;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0xC);

  primitive = (POLY_GT4 *)g_PrimCursor;
  func_8017A9E0(primitive);
  SetSemiTrans(primitive, 1);

  if (g_battle_work[9] < 8) {
    D_1F800000 = (g_battle_work[9] + 1) * 6;
  } else {
    D_1F800000 = g_battle_work[9] * 2 + 0x20;
  }
  tint = &D_1F800008;
  work = g_battle_work;
  *tint = work[0xA] * 20;
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

  primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
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

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0x34);
}
