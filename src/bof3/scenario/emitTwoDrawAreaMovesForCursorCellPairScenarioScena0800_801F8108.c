#include "bof3/bof3.h"
#include "bof3/core/slus_variables.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801F8108
 * @behavior Issues the two DR_MOVE packets that position this overlay's cursor
 * cell pair for the drawn frame. Each packet carries an eight-byte (x, y, w, h)
 * rect built at the projected cursor: w is 0x40 and h is 0x80 in both, the y
 * is the cursor y plus the frame-buffer index byte at 0x80143D44 scaled by
 * 240 (0xF0, the height of one 320x240 display buffer) minus 0x40, and the x
 * is the cursor x minus 0x40 for the first packet and the cursor x for the
 * second, so the two 0x40-wide cells sit side by side. The rect is handed to
 * the LIBGPU draw-area mover SetDrawMove with the primitive published at
 * g_PrimCursor and the fixed draw-area origins (0x340, 0x100) and
 * (0x340, 0x180), and each 0x18-byte packet is queued through
 * func_8014E5A0(2, 0x18). The primitive pointer and the buffer index byte are
 * re-read for each packet, so the two call groups share no loaded value.
 * Takes the projected cursor as two words and returns nothing; the 0x30-byte
 * frame keeps $ra plus $s0-$s3, and the cursor arguments and the 0x40/0x80
 * cell size stay in $s0-$s3 across both calls. The cells' wider role and the
 * draw-area origins' meaning remain unknown.
 * Retained clean-C shape: the row offset is bound to the `offset` local and
 * added to the cursor y as a variable, reassigned in each call group. Folding
 * the constant into one expression (`cursorY + (D_80143D44 * 0xF0 - 0x40)`)
 * lets GCC reassociate the -0x40 onto the argument, which hoists one
 * `addiu a1,-0x40` into the entry block and drops the per-group bias
 * (56/59, first=+0x0000); the unparenthesized spelling measured 50/59.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitTwoDrawAreaMovesForCursorCellPairScenarioScena0800_801F8108(s32 cursorX, s32 cursorY) {
  RECT rect;
  s32 offset;

  rect.x = cursorX - 0x40;
  rect.w = 0x40;
  rect.h = 0x80;
  offset = D_80143D44 * 0xF0 - 0x40;
  rect.y = cursorY + offset;
  SetDrawMove((DR_MOVE*)g_PrimCursor, &rect, 0x340, 0x100);
  func_8014E5A0(2, 0x18);

  rect.x = cursorX;
  rect.w = 0x40;
  rect.h = 0x80;
  offset = D_80143D44 * 0xF0 - 0x40;
  rect.y = cursorY + offset;
  SetDrawMove((DR_MOVE*)g_PrimCursor, &rect, 0x340, 0x180);
  func_8014E5A0(2, 0x18);
}
