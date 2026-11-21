#include "bof3/bof3.h"

void func_801F2F78(s32 *point); /* @source 0x801F2F78 @kind function */

/* @source 0x801F2F30
 * @behavior Entry 1 of the two-entry step dispatch table based at 0x801F34E8,
 *           whose word 0 names 0x801F2EE4 and whose word 1 names this span and
 *           which dispatchStateByte1Table34e8World03Area11613_801F2EA0 indexes
 *           with byte +1 of the work record published at the scratchpad pointer
 *           cell 0x1F800044. It loads that work-record pointer and copies the
 *           record's three cursor words at +0x34, +0x38 and +0x3C into a local
 *           twelve-byte point, then passes the address of that point to the
 *           overlay quad renderer at 0x801F2F78, which projects the three words
 *           as a position through func_801AFF04 and emits one semi-transparent
 *           G4 through SetPolyG4 and SetSemiTrans out of g_PrimCursor, so the
 *           copy is local: the callee receives the address of this frame slot,
 *           not the work-object field. The cell pointer is loaded before the
 *           stack frame because nothing between it and the local stores needs
 *           the frame. It takes no arguments, returns nothing and reads no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkCursorQuadWorld03Area11613_801F2F30(void) {
  s32 point[3];
  u8 *work;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  func_801F2F78(point);
}
