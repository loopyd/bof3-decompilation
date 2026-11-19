#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

void func_801F2DF0(s32 *point); /* @source 0x801F2DF0 @kind function */

/* @source 0x801F2DA8
 * @behavior Overlay step handler for state 1 of the local dispatch table based
 * at 0x801F3304: it loads the work-object pointer published at the scratchpad
 * cell 0x1F800044 and copies that object's three cursor words at +0x34, +0x38
 * and +0x3C into a local twelve-byte point, then passes the address of that
 * point to this overlay's quad renderer func_801F2DF0. That renderer projects
 * the three words as a position through func_801AFF04 and emits one
 * semi-transparent G4 through SetPolyG4/SetSemiTrans from g_PrimCursor, so the
 * copy is local: the callee receives the address of this frame slot, not the
 * work-object field. The cell pointer is loaded before the stack frame because
 * nothing between it and the local stores needs the frame. It takes no
 * arguments, returns nothing and reads no other state.
 * Shape evidence: the same twelve-byte copy shape carries eight spans in this
 * tree (drawWorkCursorQuadWorld02Area10013_801F2D18 and
 * drawWorkCursorQuadWorld02Area10013_801F3D28,
 * drawWorkCursorQuadWorld02Area11213_801F3EA4,
 * drawWorkCursorQuadWorld03Area14313_801F3F94,
 * drawWorkCursorQuadWorld03Area14514_801F3938, plus the still-unlifted spans at
 * 0x801F3060 of BIN/WORLD01/AREA059, 0x801F2E20 of BIN/WORLD03/AREA146 and
 * 0x801F2F30 of BIN/WORLD03/AREA116), and each differs from this span in the
 * jal immediate alone, so those are shape evidence and no clone. The statement
 * order, operand forms and declaration style are transcribed from
 * drawWorkCursorQuadWorld02Area10013_801F2D18, whose span compared equal to
 * this one in seventeen of eighteen words with the differing word being its own
 * jal to a different callee; this body's callee and address are written from
 * this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkCursorQuadWorld00Area03613_801F2DA8(void) {
  s32 point[3];
  u8 *work;

  work = D_1F800044;
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  func_801F2DF0(point);
}
