#include "bof3/bof3.h"

/* @source 0x801EF7E0
 * @behavior Second phase handler of the scratchpad overlay work object
 *           published through the pointer cell at 0x1F800044. The dispatcher
 *           at 0x801EF620 materialises the addresses of the handler at
 *           0x801EF7A0 and of this function into a two-word local table and
 *           indexes that table with the work object unsigned byte 0x03, so
 *           byte 0x03 holds one while this body runs. This body loads the
 *           pointer cell once, reads the signed word at object offset 0x10 and,
 *           while that word is negative, stores the word plus one back at word
 *           width (addiu 1 in the branch delay slot, sw in the jump delay
 *           slot). When the word is not negative the word is left alone and
 *           work byte 0x03 is decremented by one at byte width (lbu / addiu -1
 *           / sb), which hands the next frame back to the first table entry;
 *           that handler ramps the same word in the opposite direction and
 *           advances byte 0x03, so the pair oscillates the word between -3 and
 *           zero. Byte 0x03 selects only this two-entry table and the word at
 *           0x10 stays offset-anchored. No jal names this span in the payload;
 *           the dispatcher frame slot that holds its address is its only
 *           reference. Leaf with no frame: it takes no arguments, makes no call
 *           and returns nothing through the jr $ra at 0x801EF814.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkWord10WhenNegativeOtherwiseDecrementWorkByte3BmagicMagic05703_801EF7E0(
    void) {
  u8** slots;
  u8* work;
  s32 word10;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  word10 = *(s32*)(work + 0x10);
  if (word10 < 0) {
    *(s32*)(work + 0x10) = word10 + 1;
  } else {
    work[3]--;
  }
}
