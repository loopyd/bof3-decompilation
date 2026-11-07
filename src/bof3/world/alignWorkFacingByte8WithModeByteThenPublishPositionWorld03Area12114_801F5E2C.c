#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;

void publishSharedPositionWordsToWorkThenDrawMeterWorld03Area12114_801F5C7C(
    void);
s32 func_801F5ECC(s32 facing, s32 mode);

/* @source 0x801F5E2C
 * @behavior Frame hook for the work record published at the scratchpad pointer
 * cell 0x1F800044. It compares the record facing byte 0x08 with the shared mode
 * byte at 0x80145E98 and, when the two differ, calls the turn helper at
 * 0x801F5ECC for that facing and then for the same facing with bit 0x04
 * toggled; a nonzero answer means the helper already stepped the record, so the
 * hook returns without a position publish. When both answers are zero the
 * helper found the facing close to the mode byte, so the hook republishes the
 * record facing at 0x80145E98 and runs the shared position publish at
 * 0x801F5C7C, which copies the words 0x80145EC4 and 0x80145EC8 and the halfword
 * 0x80145ECE into record offsets 0x34, 0x38 and 0x3E. That publish also runs
 * directly when the facing already equals the mode byte. The scratchpad pointer
 * cell is read again before the second turn call and before the republish,
 * matching the original's per-use reloads. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void alignWorkFacingByte8WithModeByteThenPublishPositionWorld03Area12114_801F5E2C(
    void) {
  u8* mode;
  u8 facing;

  mode = &D_80145E98;
  facing = area12114WorkCursor_1F800044[8];
  if (facing != *mode) {
    if ((func_801F5ECC(facing, *mode) & 0xFF) != 0) {
      return;
    }
    if ((func_801F5ECC(area12114WorkCursor_1F800044[8] ^ 4, *mode) & 0xFF) != 0) {
      return;
    }
    *mode = area12114WorkCursor_1F800044[8];
  }
  publishSharedPositionWordsToWorkThenDrawMeterWorld03Area12114_801F5C7C();
}
