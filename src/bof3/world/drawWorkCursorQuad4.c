#include "bof3/bof3.h"

void renderCursorQuadAt(s32 *point); /* @source 0x801F2E68 @kind function */

/* @source 0x801F2E20
 * @behavior Entry 1 of the two-entry step dispatch table based at 0x801F3B00,
 *           whose word 0 names 0x801F2DD4 and whose word 1 names this span and
 *           which the overlay indexes with byte +1 of the work record published
 *           at the scratchpad pointer cell 0x1F800044: it loads that work-record
 *           pointer and copies the record's three cursor words at +0x34, +0x38
 *           and +0x3C into a local twelve-byte point, then passes the address of
 *           that point to the overlay quad renderer at 0x801F2E68, which
 *           projects the three words as a position through func_801AFF04 and
 *           emits one semi-transparent G4 through SetPolyG4 and SetSemiTrans, so
 *           the copy is local: the callee receives the address of this frame
 *           slot, not the work-object field. The cell pointer is loaded before
 *           the stack frame because nothing between it and the local stores
 *           needs the frame. It takes no arguments, returns nothing and reads no
 *           other state.
 * Shape evidence: this span differs from the already-exact sibling
 * drawWorkCursorQuadWorld00Area03613_801F2DA8 at 0x801F2DA8 in exactly one of
 * its eighteen words, the jal immediate at +0x30, and that word shifts by
 * 0x0C07CB7C to 0x0C07CB9A, which is the delta between the two spans (0x78)
 * carried into the two callee addresses 0x801F2DF0 and 0x801F2E68, so the pair
 * is one body and the sibling supplies the statement order, operand forms and
 * declaration style while this body's callee, address and literals are written
 * from this payload. The same twelve-byte copy shape also carries
 * drawWorkCursorQuadWorld02Area10013_801F2D18,
 * drawWorkCursorQuadWorld02Area10013_801F3D28 and
 * drawWorkCursorQuadWorld03Area14514_801F3938, plus the still-unlifted spans at
 * 0x801F3060 of WORLD01/AREA059 and 0x801F2F30 of WORLD03/AREA116. Because the
 * jal word differs, the span is not byte-identical to any of them and no clone
 * applies; the name comes from that sibling family and from the table position.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkCursorQuad(void) {
  s32 point[3];
  u8 *work;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  renderCursorQuadAt(point);
}
