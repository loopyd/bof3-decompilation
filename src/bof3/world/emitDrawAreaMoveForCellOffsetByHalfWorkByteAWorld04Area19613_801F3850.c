#include "bof3/bof3.h"

/* Shared primitive cursor published by the main executable at 0x8014598C. It
 * already has a generated undefined-symbol row for this target, so only the
 * declaration is needed here. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Game primitive-append helper at 0x8014E5A0, outside this overlay, taking the
 * ordering-table index and the primitive size in bytes. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801F3850
 * @behavior Emits one draw-area move packet for a single 0x10 by 0x20 cell.
 *           The rect is built on this 0x20-byte frame at +0x10: the x is the
 *           overlay's fixed source column 0x1A8, the width is 0x10 and the
 *           height is 0x20, and the y is 0x140 (the 320-line frame-buffer mid
 *           height) minus half of the unsigned byte 0xA of the shared scratch
 *           work record reached through the scratchpad pointer cell 0x1F800044,
 *           which is loaded once and only for that byte. The rect is handed to
 *           the LIBGPU draw-area mover SetDrawMove with the primitive published
 *           at g_PrimCursor and the fixed destination origin (0x198, 0x120), and
 *           the resulting 0x18-byte packet is queued through func_8014E5A0(2,
 *           0x18). Takes no arguments, returns nothing, and the frame keeps only
 *           $ra at +0x18. The role of the work byte at 0xA (an overlay-local
 *           counter elsewhere in this binary) and the meaning of the copied cell
 *           remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitDrawAreaMoveForCellOffsetByHalfWorkByteAWorld04Area19613_801F3850(void) {
  RECT rect;

  rect.x = 0x1A8;
  rect.y = 0x140 - (SPAD_PTR_SLOT(u8, 0x44)[0xA] >> 1);
  rect.w = 0x10;
  rect.h = 0x20;
  SetDrawMove((DR_MOVE*)g_PrimCursor, &rect, 0x198, 0x120);
  func_8014E5A0(2, 0x18);
}
