#include "bof3/bof3.h"

/* The statement order and the SPAD_PTR_TABLE cell spelling are the exact
 * sibling resetAdvanceScratchState (0x801F3460, world00/area016/13), which
 * clears two of these work bytes and then advances work byte 0x01 with the
 * same local pointer. Each store goes through the pointer loaded from the
 * cell, so it may alias the cell and the next evaluation re-reads it; the
 * final increment keeps one cell load for its read and its write, which is
 * why its store fills the return delay slot. */

/* @source 0x801EEE50
 * @behavior Clears work-record bytes 0x0B, 0x09 and 0x0A, then advances work
 *           byte 0x01 with a byte-width read-modify-write whose store is the
 *           return-delay-slot instruction. The scratchpad work-object pointer
 *           cell at 0x1F800044 is re-read before each of the three clears and
 *           once more for the increment. Leaf with no frame; takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytes9ABAndAdvanceWorkByte1BmagicMagic10703_801EEE50(void)
{
  u8** slots;
  u8*  ptr;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x0b] = 0;
  slots[0x11][0x09] = 0;
  slots[0x11][0x0a] = 0;
  ptr = slots[0x11];
  ptr[1] += 1;
}
