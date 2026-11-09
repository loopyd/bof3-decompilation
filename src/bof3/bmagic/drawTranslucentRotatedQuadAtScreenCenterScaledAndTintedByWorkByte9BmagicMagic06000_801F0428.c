#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, the cell this target's
 * exact lifts already bind as g_battle_work. This body reads the cell five
 * times and never keeps one base across them: the radius seed and the first
 * corner angle share one load, each later corner angle takes its own, and the
 * tint ramp takes the last, because a call or a primitive store sits between
 * every pair. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The original forms its address as one
 * lui 0x1F80 plus addiu 0 pair, keeps that base in $s1 across all eight rotator
 * calls and re-reads the cell with lw 0($s1) before each product, while the
 * single publish of the radius is a sw through the same base. The base is
 * therefore a live pointer to a signed word cell, not a folded constant. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad tint-ramp cell at 0x1F800004, published once from work byte 9 and
 * read back for each of the twelve colour bytes. The original keeps the
 * absolute-address form, one lui $at plus the access at its own offset, and
 * re-loads the cell for every colour byte after the first because each of those
 * byte stores may alias it; the value the store just wrote reaches the first
 * byte as a register, with no second load. */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */

/* Scratchpad rotation-angle cell at 0x1F800008, published before each corner
 * and re-read as the second rotator argument of that corner. The first call of
 * a corner takes the register the store just wrote and only the second re-loads
 * the cell, so the cell stays unqualified. */
extern s32 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: both return the sample of
 * the 12-bit angle in the fixed-point scale the two bodies of this family shift
 * right 12 over the published radius. This body takes the 0x801782FC sample for
 * every corner x and the 0x801783C8 sample for its y. Neither address carries
 * an evidenced name, so both stay address-anchored. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query that selects the draw-mode texture page. */
s32 GetGraphType(void);

/* @source 0x801F0428
 * @behavior Rotated quad emitter of this overlay's work-byte-1 handler group.
 *           Its only in-image caller is the dispatcher
 *           dispatchWorkByte2Table0bccHandlerThenDrawWhenActiveBmagicMagic06000_801F0254
 *           at 0x801F0254, which reaches it with no arguments once the work
 *           object byte 0x0 and the handler index byte 0x2 are both non-zero,
 *           and the body reads no incoming register, so it takes none.
 *           It reads the shared graphic type twice and, when either answer is 1
 *           or 2, selects the draw-mode texture page 0x125, else 0x55; that
 *           pair is the family formula with the selector fixed at 2. It links a
 *           0xC-byte draw-mode packet through SetDrawMode at g_PrimCursor with
 *           dfe 0 and dtd 1 and through the shared primitive-append helper at
 *           0x8014E5A0 under OT slot 3.
 *           It then takes a fresh g_PrimCursor POLY_G4, seeds it with
 *           SetPolyG4, marks it semi-transparent and writes four corners around
 *           the fixed screen centre (0xA0, 0x78). Every corner angle is the
 *           scratchpad rotation cell 0x1F800008: work byte 9 plus one of the
 *           constants 6, 0xE, 0x1E and 0x16, masked to five bits - eighth turns
 *           of a 32-step circle - and scaled left 7 into the 12-bit angle the
 *           rotator pair takes. Each corner x is the centre 0xA0 plus the
 *           0x801782FC sample of that angle scaled by the radius cell and
 *           shifted right 12, and each corner y is the centre 0x78 plus the
 *           0x801783C8 sample of it, so the four points are a square rotated by
 *           work byte 9. The radius published once before the first corner is
 *           work byte 9 times three plus 0x12, and it is re-read from the
 *           scratchpad cell before every one of the eight products.
 *           The quad tint is one uniform grey ramp: work byte 9 divided by three
 *           as an unsigned value, truncated to eight bits and multiplied by six,
 *           published in the scratchpad word 0x1F800004 and re-read for the
 *           twelve colour bytes of the four vertices, so the cell reloads after
 *           every byte store while the first byte takes the stored register.
 *           The quad is then appended as a 0x24-byte packet under the same OT
 *           slot, the graphic type is queried a second time and selects 0x25,
 *           else 0x15, for a second draw-mode packet that is appended as 0xC
 *           bytes with no primitive written after it.
 *           It returns nothing and its 0x28-byte frame holds $ra, $s1, $s0 and
 *           the outgoing fifth draw-mode argument at 0x10.
 *           No twin exists: all seven call targets of this span resolve in the
 *           shared main executable and its data references are only the shared
 *           cursor cell at 0x8014598C and the scratchpad, so the span was
 *           searched across every extracted payload under out/binaries with
 *           each call and jump immediate masked, decoded in both byte orders,
 *           and no other copy exists. It was written from the original bytes
 *           with the statement order, operand forms and declaration style of the
 *           family quad emitters that this overlay already holds exact, whose
 *           x corners use halfword stores through the primitive pointer. The
 *           y corners use POLY_G4 fields, allowing the next work-cell load to
 *           fill the preceding multiply delay without moving the x store past
 *           the second rotator argument load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentRotatedQuadAtScreenCenterScaledAndTintedByWorkByte9BmagicMagic06000_801F0428(
    void) {
  POLY_G4 *primitive;
  s32 *scale;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x125;
  } else {
    graph = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);

  primitive = (POLY_G4 *)g_PrimCursor;
  SetPolyG4(primitive);
  SetSemiTrans(primitive, 1);

  scale = &D_1F800000;
  *scale = g_battle_work[9] * 3 + 0x12;

  D_1F800008 = ((g_battle_work[9] + 6) & 0x1F) << 7;
  *(u16 *)((u8 *)primitive + 0x08) = 0xA0 + ((func_801782FC(D_1F800008) * *scale) >> 12);
  primitive->y0 = 0x78 + ((func_801783C8(D_1F800008) * *scale) >> 12);

  D_1F800008 = ((g_battle_work[9] + 0xE) & 0x1F) << 7;
  *(u16 *)((u8 *)primitive + 0x10) = 0xA0 + ((func_801782FC(D_1F800008) * *scale) >> 12);
  primitive->y1 = 0x78 + ((func_801783C8(D_1F800008) * *scale) >> 12);

  D_1F800008 = ((g_battle_work[9] + 0x1E) & 0x1F) << 7;
  *(u16 *)((u8 *)primitive + 0x18) = 0xA0 + ((func_801782FC(D_1F800008) * *scale) >> 12);
  primitive->y2 = 0x78 + ((func_801783C8(D_1F800008) * *scale) >> 12);

  D_1F800008 = ((g_battle_work[9] + 0x16) & 0x1F) << 7;
  *(u16 *)((u8 *)primitive + 0x20) = 0xA0 + ((func_801782FC(D_1F800008) * *scale) >> 12);
  primitive->y3 = 0x78 + ((func_801783C8(D_1F800008) * *scale) >> 12);

  D_1F800004 = (u8)((u32)g_battle_work[9] / 3) * 6;
  primitive->r0 = D_1F800004;
  primitive->g0 = D_1F800004;
  primitive->b0 = D_1F800004;
  primitive->r1 = D_1F800004;
  primitive->g1 = D_1F800004;
  primitive->b1 = D_1F800004;
  primitive->r2 = D_1F800004;
  primitive->g2 = D_1F800004;
  primitive->b2 = D_1F800004;
  primitive->r3 = D_1F800004;
  primitive->g3 = D_1F800004;
  primitive->b3 = D_1F800004;
  GpuAppendPrim(3, 0x24);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);
}
