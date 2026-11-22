#include "bof3/bof3.h"

/* Shared frame counter of the main executable: its low two bits count frames,
 * so a zero test on them fires once every fourth frame. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Screen background of this area, which adds the passed vertical offset to
 * every band it draws. */
extern void
drawScrolledScreenBackgroundQuadRightColumnAndEightTiledBandsWorld04Area19913_801F2E80(
    s32 offset);

/* @source 0x801F2C7C
 * @behavior Background scroll step of this overlay: takes no arguments and
 *           returns no value. It reads the shared frame counter at 0x80143E6C
 *           once with `lw`, masks its low two bits and tests them against zero
 *           (`andi` into `sltiu`), loads the overlay work-record pointer
 *           published at the scratchpad cell 0x1F800044, and reads that
 *           record's signed word at 0x34. It subtracts the zero test from that
 *           word, so the vertical background offset drops by one on the frame
 *           whose two low counter bits are both clear and holds on every other
 *           frame, stores the result back into the same word, and passes that
 *           computed value - copied out of the producing register rather than
 *           reloaded - as the vertical offset to the background drawer at
 *           0x801F2E80. It allocates the 0x18-byte frame that call requires,
 *           saving $ra at 0x10($sp), and has one exit through $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementBackgroundVerticalOffsetEveryFourthFrameAndDrawScrolledScreenBackgroundWorld04Area19913_801F2C7C(
    void) {
  s32 offset;
  u8* work;

  /* The consumed predicate and final offset share one local. Subtraction is
   * unsigned modulo 2^32; the signed view preserves the drawer's word ABI. */
  work = SPAD_PTR_SLOT(u8, 0x44);
  offset = (D_80143E6C & 3) == 0;
  offset = (s32)(*(u32*)(work + 0x34) - (u32)offset);
  *(s32*)(work + 0x34) = offset;
  drawScrolledScreenBackgroundQuadRightColumnAndEightTiledBandsWorld04Area19913_801F2E80(
      offset);
}
