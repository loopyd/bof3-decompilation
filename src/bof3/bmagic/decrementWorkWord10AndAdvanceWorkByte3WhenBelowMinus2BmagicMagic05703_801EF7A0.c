#include "bof3/bof3.h"

/* The scratchpad work-object pointer cell 0x1F800044 is loaded once and the
 * signed word at object offset 0x10 is taken into a local, so the compare and
 * the store both read that one load; the byte store on the other arm needs no
 * reload because it does not follow a store through the same pointer.
 * Declaration style and the SPAD_PTR_TABLE spelling follow the exact sibling
 * advanceWorkByte1AndClearWorkByte2WhenWorkBytes9AEqualBmagicMagic05703_801EEFA4
 * (0x801EEFA4) in this same overlay, which reads the same pointer cell. */

/* @source 0x801EF7A0
 * @behavior First phase handler of the scratchpad overlay work object published
 *           through the pointer cell at 0x1F800044. The dispatcher at
 *           0x801EF620 materialises the addresses of this function and the
 *           handler at 0x801EF7E0 into a two-word local table and indexes that
 *           table with the work object's unsigned byte 0x03, so byte 0x03 holds
 *           zero while this body runs. This body loads the pointer cell once,
 *           reads the signed word at object offset 0x10 and, while that word is
 *           at least -2, stores the word minus one back (lw / slti -2 / addiu
 *           -1 / sw, the store landing in the jump delay slot). When the word is
 *           below -2 the word is left alone and work byte 0x03 is incremented by
 *           one at byte width (lbu / addiu 1 / sb), which hands the next frame
 *           to the second table entry; that handler ramps the same word in the
 *           opposite direction and takes byte 0x03 back below zero, so the pair
 *           oscillates the word between -3 and zero. Byte 0x03 selects only this
 *           two-entry table and the word at 0x10 stays offset-anchored. No jal
 *           or table word in this payload names the span; the dispatcher
 *           address is its only reference. Leaf with no frame: it takes no
 *           arguments, makes no call and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkWord10AndAdvanceWorkByte3WhenBelowMinus2BmagicMagic05703_801EF7A0(void) {
  u8** slots;
  u8* work;
  s32 word10;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  word10 = *(s32*)(work + 0x10);
  if (word10 >= -2) {
    *(s32*)(work + 0x10) = word10 - 1;
  } else {
    work[3]++;
  }
}
