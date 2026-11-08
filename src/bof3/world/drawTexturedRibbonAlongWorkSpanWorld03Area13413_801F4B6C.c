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
typedef struct World03Area13413WorkSpan {
  s32 centreX;
  s32 centreY;
  s32 span;
  s32 unk_0C;
} World03Area13413WorkSpan;

/* One 8-byte projected screen entry: func_801AFF04 writes the halfword pair at
 * +0x00/+0x02 and the trailing four bytes stay unwritten. */
typedef struct World03Area13413ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} World03Area13413ScreenPoint;

/* @source 0x801F4B6C
 * @behavior Draws the textured band the work record sweeps along its own span
 *           and then links the trailing grid cell.
 *           The record is reached through the scratchpad pointer cell
 *           0x1F800044 by the byte-1 handler entries 1 and 2 of the table based
 *           at 0x801F5828,
 *           advanceWorkWord1CClampedToWord14AndRearmCountdownThenAdvanceWorkByte1World03Area13413_801F4400
 *           and retreatWorkWord24By100000AndSweepHalfword20ThenResetWorkareaAtCountdownEndWorld03Area13413_801F44C0,
 *           which own the step word at +0x24 and the phase halfword at +0x20
 *           this body reads and both call it with no argument register set.
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
 *           GetClut(0, 0x1E5), the flat 0xC0 grey is written to the three
 *           colour bytes, and SetSemiTrans is cleared. The trailing grid cell
 *           is linked through func_80155A08 with the last point pair, kind 2
 *           and size 0x28.
 *           Takes no arguments, returns nothing, and keeps a 0x90-byte frame
 *           for $ra at 0x88 and $s5..$s0 at 0x84..0x70 over its 0x10..0x6F
 *           local span: the copied span block at 0x10, the three-word point at
 *           0x20, the two projected rails at 0x30 and the 0x30-byte projection
 *           scratch at 0x40.
 * Name evidence: the only in-image references to this address are the jal at
 *           0x801F4454 in the entry at 0x801F4400 and the jal at 0x801F4524 in
 *           the entry at 0x801F44C0 of the byte-1 handler table based at
 *           0x801F5828 that this target's exact
 *           dispatchStateByte1Table5828World03Area13413_801F4344 indexes with
 *           the work record's state byte at +1. Neither caller sets an argument
 *           register, so the callee takes none, and each moves the record step
 *           word at +0x24 and the phase halfword at +0x20 immediately before
 *           the call, which are the two record fields this body reads. The body
 *           itself walks the record's own +0x14..+0x1C span and emits a
 *           textured FT4 strip along it, so the name states that span and the
 *           textured ribbon shape instead of an unproven effect role. The roles
 *           of the record's words at +0x14, +0x1C and +0x24 and of its
 *           halfword at +0x20 stay described by offset.
 * Shape evidence: world03/area132/13 holds this span at 0x801F3268 (file offset
 *           0x668), and the two spans are instruction-for-instruction identical
 *           at all 190 words except the six branch displacements and one
 *           immediate byte - that body's addiu $v0,$zero,0x80 against this
 *           body's addiu $v0,$zero,0xC0 at 0x801F4DC0 - so it is shape
 *           evidence, never clone eligibility: lift clone refuses it with
 *           "clone range is not byte-identical to the representative range".
 *           That sibling's exact source
 *           src/bof3/world/drawTexturedRibbonAlongWorkSpanWorld03Area13213_801F3268.c
 *           supplies the statement order, the 0x30-byte scratch declaration,
 *           the u16 phase spelling, the two projected rails and the
 *           (x0,y0,x1,y1)/(x2..y3)/u0..v3/colour/TPage/CLUT field order; this
 *           function's own colour constant and record offsets come from its
 *           original bytes. No payload among the 396 binaries under
 *           out/binaries carries a byte-identical copy of this 0x2F8-byte span
 *           and this target reports no data candidate over it, so the data,
 *           carve and clone levers are all declined on this range.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTexturedRibbonAlongWorkSpanWorld03Area13413_801F4B6C(void) {
  World03Area13413WorkSpan    span;
  s32                         point[3];
  World03Area13413ScreenPoint rail[2];
  u8                          scratch[0x30];
  POLY_FT4*                   prim;
  u16                         phase;

  func_801AFE18(scratch);
  phase = FIELD_REF(u16, D_1F800044, 0x20);
  span = *(const World03Area13413WorkSpan*)(D_1F800044 + 0xC);
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
    prim->r0 = 0xC0;
    prim->g0 = 0xC0;
    prim->b0 = 0xC0;
    prim->tpage = GetTPage(1, 0, 0x1C0, 0x100);
    prim->clut = GetClut(0, 0x1E5);
    func_80155A08(point[0], point[1], 2, 0x28);
  }
}
