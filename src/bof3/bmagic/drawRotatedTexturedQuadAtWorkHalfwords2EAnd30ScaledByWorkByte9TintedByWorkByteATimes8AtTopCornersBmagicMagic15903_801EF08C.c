#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The original re-forms the
 * cell address with one lui 0x1F80 plus lw 0x44 before every read, because a
 * call sits between the reads, so the cell is bound as the shared pointer
 * variable the family siblings use and never kept across a call. The tint block
 * is the only site that binds the loaded pointer to a local, because it stores
 * two scratchpad halfwords between the two reads of work byte 0xA and the store
 * may alias the object, so a fresh cell read per byte would emit two extra
 * lui/lw pairs where the original keeps one base in $v1. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The original forms its address as one
 * lui 0x1F80 plus addiu 0 pair, keeps that base in $s1 across all eight rotator
 * calls and re-reads the cell with lh 0($s1) before each product, while the
 * single publish of the radius is a sh through the same base. The base is
 * therefore a live pointer to a signed halfword cell, not a folded constant. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad tint cells at 0x1F800002 and 0x1F800004. Each is published once,
 * as work byte 0xA shifted left 3 and as work byte 0xA itself, and each read
 * re-forms the address with one lui 0x1F80 plus lhu at its own offset, never
 * through a live base register, so the cells are named at their own addresses
 * rather than folded. Every read follows a byte store through the primitive
 * pointer, which may alias the cells, so each of the six reads of a cell is a
 * fresh load. */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair at 0x80159320/0x80159322, links the primitive at g_PrimCursor
 * with the given kind and advances the cursor by the given size. This body links
 * a 0xC-byte draw-mode packet at the work object projection words 0x34 and 0x38
 * before the quad and the 0x34-byte quad itself after it, both with kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared rotator pair at 0x801782FC and 0x801783C8, both returning the sample of
 * the 12-bit angle in the fixed-point scale this body shifts right 12 over the
 * published radius. This body takes the 0x801783C8 sample for every corner x and
 * the 0x801782FC sample for every corner y, the same split the family sibling
 * drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedBy
 * WorkByteATimesBytes5d5e5fBmagicMagic04403_801F06D8 uses. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed at 0x8017A9E0. It is the POLY_GT4 seeder: the object
 * this body builds is 0x34 bytes, the length the trailing func_80155A08 call
 * links, and every field it writes sits at the POLY_GT4 offsets (colours at
 * 0x4/0x10/0x1C/0x28, vertices at 0x8/0x14/0x20/0x2C, uv pairs at 0xC/0x18/0x24/
 * 0x30, clut 0xE, tpage 0x1A), while the seed sits in the SDK library slot
 * between SetPolyG4 (0x8017A9CC) and SetSprt8 (0x8017A9F4). The shared SDK map
 * carries no name for it, so it keeps its address-qualified prototype and binds
 * through this target's generated undefined-function table. */
void func_8017A9E0(POLY_GT4 *arg0);

/* @source 0x801EF08C
 * @behavior Quad emitter of this overlay's work-byte-2 handler group. Its only
 *           in-image caller is the dispatcher
 *           dispatchWorkByte2HandlerThenShiftMotionPairAndDrawWhenActive
 *           BmagicMagic15903_801EEE28 at 0x801EEE28, which reaches it with no
 *           arguments from the draw call at 0x801EEF40 once the work object
 *           byte 0x0 and the handler index byte 0x2 are both non-zero, and the
 *           body reads no incoming register, so it takes none.
 *           It reads the shared graphic type twice and selects the draw-mode
 *           texture page 0xA5 when either answer is 1 or 2, else 0x35, then
 *           links a 0xC-byte draw-mode packet through SetDrawMode at
 *           g_PrimCursor with dfe 0 and dtd 1 anchored at the work object
 *           projection words 0x34 and 0x38 with kind 2 through func_80155A08.
 *           It then publishes three scratchpad cells from the work object: work
 *           byte 9 shifted left 1 as the radius in the signed halfword at
 *           0x1F800000, work byte 0xA shifted left 3 in the halfword at
 *           0x1F800002 and work byte 0xA itself in the halfword at 0x1F800004.
 *           It takes a fresh g_PrimCursor POLY_GT4, seeds it through the shared
 *           seeder at 0x8017A9E0, marks it semi-transparent and writes four
 *           corners around the work object screen halfwords 0x2E (x) and 0x30
 *           (y): each corner x is that halfword plus the func_801783C8 sample of
 *           the corner angle scaled by the published radius and shifted right
 *           12, and each corner y is the halfword 0x30 plus the func_801782FC
 *           sample of the same angle. The corner angles are 0xE00, 0xA00, 0x200
 *           and 0x600, eighth turns of the 12-bit circle, which place the four
 *           corners of a half-diagonal-equals-radius diamond around the work
 *           screen pair in clockwise order.
 *           The texture is tpage GetTPage(0, 1, 0x340, 0x100) and clut
 *           GetClut(0, 0x1FA), and the uv rectangle is the 0x4F-by-0x40 window
 *           at (8, 8): u 8 on corners 0 and 2, u 0x57 on corners 1 and 3, v 8 on
 *           corners 0 and 1 and v 0x48 on corners 2 and 3. The quad tint is work
 *           byte 0xA for all four colours, read back from the scratchpad cell
 *           0x1F800002 at eight times that byte for the top corners 0 and 1 and
 *           from the cell 0x1F800004 at the unshifted byte for the bottom
 *           corners 2 and 3; since the primitive pointer may alias those cells
 *           every one of the twelve colour stores reloads its cell.
 *           Finally it links the 0x34-byte polygon at the same work projection
 *           words with kind 2 and returns, having stored nothing but the
 *           scratchpad cells and the primitive itself. The 0x28-byte frame holds
 *           $ra, $s1, $s0 and the fifth draw-mode argument at 0x10.
 *           No twin exists: the 0x394 payload bytes were searched across every
 *           extracted payload under out/binaries and appear only at this span's
 *           own offset 0x48C. It was written from the original bytes with the
 *           shape of the family sibling at
 *           emi/bmagic/magic044/03@0x801F06D8, whose prologue, draw-mode block
 *           and eight rotator corners are instruction-identical modulo this
 *           body's own constants, while the POLY_GT4 field set, the scratchpad
 *           tint publication, the uv window and the trailing link size follow
 *           this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRotatedTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimes8AtTopCornersBmagicMagic15903_801EF08C(
    void) {
  POLY_GT4 *primitive;
  s16 *scale;
  s32 graph;
  u8 *work;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0xC);

  work = g_battle_work;
  scale = &D_1F800000;
  primitive = (POLY_GT4 *)g_PrimCursor;
  *scale = work[9] << 1;
  D_1F800002 = work[0xA] << 3;
  D_1F800004 = work[0xA];
  func_8017A9E0(primitive);
  SetSemiTrans(primitive, 1);

  primitive->x0 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0xE00) * *scale) >> 12);
  primitive->y0 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0xE00) * *scale) >> 12);
  primitive->x1 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0xA00) * *scale) >> 12);
  primitive->y1 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0xA00) * *scale) >> 12);
  primitive->x2 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0x200) * *scale) >> 12);
  primitive->y2 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0x200) * *scale) >> 12);
  primitive->x3 =
      *(u16 *)(g_battle_work + 0x2E) +
      ((func_801783C8(0x600) * *scale) >> 12);
  primitive->y3 =
      *(u16 *)(g_battle_work + 0x30) +
      ((func_801782FC(0x600) * *scale) >> 12);

/* The clut store is the one access in this body the original commits straight
 * after its call. Written as a plain member store, the build hoists the whole uv
 * and colour group above it, keeps the GetClut answer live in $v0 and therefore
 * materialises the uv constants in $v1/$a0, which measured 165 of 229
 * instructions against this span. A volatile-qualified store keeps it at its
 * call, frees $v0 for the uv constants and measured 229 of 229 with no byte
 * difference; every tried grouping of the uv and colour statements, a local for
 * the GetClut answer and a second pointer alias measured the same 165. */
  primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
  *(volatile u16 *)&primitive->clut = GetClut(0, 0x1FA);

  primitive->u0 = 8;
  primitive->v0 = 8;
  primitive->u1 = 0x57;
  primitive->v1 = 8;
  primitive->u2 = 8;
  primitive->v2 = 0x48;
  primitive->u3 = 0x57;
  primitive->v3 = 0x48;

  primitive->r0 = D_1F800002;
  primitive->g0 = D_1F800002;
  primitive->b0 = D_1F800002;
  primitive->r1 = D_1F800002;
  primitive->g1 = D_1F800002;
  primitive->b1 = D_1F800002;
  primitive->r2 = D_1F800004;
  primitive->g2 = D_1F800004;
  primitive->b2 = D_1F800004;
  primitive->r3 = D_1F800004;
  primitive->g3 = D_1F800004;
  primitive->b3 = D_1F800004;

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0x34);
}
