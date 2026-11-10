#include "bof3/bof3.h"

extern void func_801F3F04(s32 arg0); /* @source 0x801F3F04 */

/* @source 0x801F3E18
 * @behavior Third entry of the handler table based at 0x801F4634, which the
 * byte-2 dispatcher dispatchStateByte2Table4634WorldWorld03Area11913_801F3D08
 * indexes with the work record's state byte at offset 0x02. It first calls the
 * payload routine at 0x801F3F04 with the constant 7, then reads the work
 * record published at the scratchpad pointer cell 0x1F800044 and increments the
 * record's countdown byte at offset 0x09 at byte width. When that stored byte
 * has reached 0x1F the handler saturates it back to 0x1F and advances the
 * record's state byte at offset 0x02 by one, so the dispatcher moves on to the
 * entry at 0x801F3E94. The pointer cell is re-read before each access group, so
 * the increment, the saturated store and the state store each load the cell
 * again. Takes no arguments, carries a 0x18-byte frame that only keeps $ra at
 * 0x10 across the call, and returns nothing.
 * @status partial
 * @match 83.87
 * @residual The candidate is five instructions short at the original size; the
 *           first difference is at +0x0010, inside the countdown compare that
 *           the original folds into the loop test.
 */
void incrementCountdownByte9UpTo1FThenAdvanceWorkByte2World03Area11913_801F3E18(void) {
  u8** slots;
  u8* work;
  u32 count;

  func_801F3F04(7);
  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  work[0x09] += 1;
  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  count = work[0x09];
  if (count >= 0x1F) {
    work[0x09] = 0x1F;
    slots = SPAD_PTR_TABLE(u8);
    work = slots[0x11];
    work[0x02] += 1;
  }
}
