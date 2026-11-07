#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared resident helpers, declared the way this overlay's exact siblings
 * declare them: func_801AFE18 fills the projection scratch, the projector
 * func_801AFF04 projects the three-word point handed as arg0 into the screen
 * halfword pair at arg1, func_801782FC/func_801783C8 return the sine and cosine
 * of a phase, and func_80155A08 links one grid-cell primitive at the passed
 * packed coordinate pair with its kind and size class. */
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Scratchpad work-object pointer cell: the original re-materialises the cell
 * address and reloads the published pointer once per access group, so the body
 * reads the cell directly at each group instead of holding one base. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* The four words the original copies as one block out of the work object at
 * +0x0C: the first two are the centre pair the angular helpers offset, the
 * third is the span word this body advances in place, and the fourth is copied
 * but never read here, which is why the original still stores it. */
typedef struct World03Area132WorkSpan {
  s32 centreX;
  s32 centreY;
  s32 span;
  s32 unk_0C;
} World03Area132WorkSpan;

/* One 8-byte projected screen entry: func_801AFF04 writes the halfword pair at
 * +0x00/+0x02 and the trailing four bytes stay unwritten. */
typedef struct World03Area132ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} World03Area132ScreenPoint;

/* @source 0x801F3268
 * @behavior Draws the textured band the work record sweeps along its own span
 *           and then links the trailing grid cell.
 *           The record is reached through the scratchpad pointer cell
 *           0x1F800044 by the byte-1 handler entry 1 of the 0x801F3E00 table,
 *           advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World03Area13213_801F312C,
 *           once per frame while that handler runs; that entry owns the step
 *           word at +0x24 and the phase halfword at +0x20 this body reads.
 *           It fills its projection scratch through func_801AFE18, copies the
 *           four words at +0x0C..+0x18 as one block, and seeds the three-word
 *           point at the copied centre offset by the sine and cosine of the
 *           +0x20 phase shifted left 5. The point's third word is the span
 *           position and is projected twice per step: first against the
 *           trailing position, the copied span word minus the +0x24 step
 *           clamped up to the +0x14 word, and then against the leading
 *           position, the copied span word clamped down to the +0x1C word. The
 *           walk starts from the +0x14 word and advances the copied span word
 *           by 0x100000 per quad while it stays below the +0x1C word plus
 *           0x1000000, and the phase halfword advances by 0x100 per quad. Each
 *           step emits a POLY_FT4 whose first two vertices are the previous
 *           pair of projected rails and whose last two are the freshly
 *           projected pair, so successive quads stitch into a band that twists
 *           as it advances; the four texels take u 0/0x1F against v 0xFF/0xE0
 *           of the texture page GetTPage(1, 0, 0x1C0, 0x100) with CLUT
 *           GetClut(0, 0x1E5), the flat 0x80 grey is written to the three
 *           colour bytes, and SetSemiTrans is cleared. The trailing grid cell
 *           is linked through func_80155A08 with the last point pair, kind 2
 *           and size 0x28.
 *           Takes no arguments, returns nothing, and keeps a 0x90-byte frame
 *           for $ra and $s5..$s0 over its 0x10..0x6F local span.
 * Name evidence: the caller above documents this address as the per-frame
 *           helper it runs while the record's byte-1 handler is active, and the
 *           body emits a textured FT4 strip walked along the record's own
 *           +0x14..+0x1C span, so the name states that span and the textured
 *           ribbon shape instead of an unproven effect role. The roles of the
 *           record's words at +0x14, +0x1C and +0x24 stay described by offset.
 * Shape evidence: the projection scratch, the three-word point handed to
 *           func_801AFF04, the two projected rails and the FT4 field order are
 *           this repository's lifted family, e.g.
 *           drawFadingTranslucentGouraudRingPairWorld01Area04713_801F2F60 and
 *           func_801F2DF8; those bodies have their own textures, phases and
 *           record layouts, so they are shape evidence only, and every offset,
 *           constant, clamp and helper binding here comes from this function's
 *           own original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTexturedRibbonAlongWorkSpanWorld03Area13213_801F3268(void) {
  World03Area132WorkSpan    span;
  s32                       point[3];
  World03Area132ScreenPoint rail[2];
  u8                        scratch[0x30];
  POLY_FT4*                 prim;
  u16                       phase;

  func_801AFE18(scratch);
  phase = FIELD_REF(u16, D_1F800044, 0x20);
  span = *(const World03Area132WorkSpan*)(D_1F800044 + 0xC);
  point[0] = span.centreX + (func_801783C8(phase) << 5);
  point[1] = span.centreY + (func_801782FC(phase) << 5);
  point[2] = span.span - FIELD_REF(s32, D_1F800044, 0x24);
  if (point[2] < FIELD_REF(s32, D_1F800044, 0x14)) {
    point[2] = FIELD_REF(s32, D_1F800044, 0x14);
  }
  func_801AFF04(point, &rail[0]);
  point[2] = span.span;
  if (FIELD_REF(s32, D_1F800044, 0x1C) < point[2]) {
    point[2] = FIELD_REF(s32, D_1F800044, 0x1C);
  }
  func_801AFF04(point, &rail[1]);
  while (span.span < FIELD_REF(s32, D_1F800044, 0x1C) + 0x1000000) {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetSemiTrans(prim, 0);
    prim->x0 = rail[0].x;
    prim->y0 = rail[0].y;
    prim->x1 = rail[1].x;
    prim->y1 = rail[1].y;
    phase += 0x100;
    span.span += 0x100000;
    point[0] = span.centreX + (func_801783C8(phase) << 5);
    point[1] = span.centreY + (func_801782FC(phase) << 5);
    point[2] = span.span - FIELD_REF(s32, D_1F800044, 0x24);
    if (point[2] < FIELD_REF(s32, D_1F800044, 0x14)) {
      point[2] = FIELD_REF(s32, D_1F800044, 0x14);
    }
    func_801AFF04(point, &rail[0]);
    point[2] = span.span;
    if (FIELD_REF(s32, D_1F800044, 0x1C) < point[2]) {
      point[2] = FIELD_REF(s32, D_1F800044, 0x1C);
    }
    func_801AFF04(point, &rail[1]);
    prim->x2 = rail[0].x;
    prim->y2 = rail[0].y;
    prim->x3 = rail[1].x;
    prim->y3 = rail[1].y;
    prim->u0 = 0x1F;
    prim->u1 = 0x1F;
    prim->v0 = 0xFF;
    prim->v1 = 0xE0;
    prim->u2 = 0;
    prim->v2 = 0xFF;
    prim->u3 = 0;
    prim->v3 = 0xE0;
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    prim->tpage = GetTPage(1, 0, 0x1C0, 0x100);
    prim->clut = GetClut(0, 0x1E5);
    func_80155A08(point[0], point[1], 2, 0x28);
  }
}
