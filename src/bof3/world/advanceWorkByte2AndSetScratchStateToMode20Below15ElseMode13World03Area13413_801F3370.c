#include "bof3/bof3.h"

/* @source 0x801F3370
 * @behavior Overlay step handler: reads the work pointer published at the
 * scratchpad cell 0x1F800044, takes the record byte at offset 0x02, stores that
 * byte plus one back to offset 0x02, and then arms the record state byte at
 * offset 0x01 from the previous counter value. While that value was below 0x0F
 * the state byte becomes 0x14; once it has reached 0x0F the state byte becomes
 * 0x0D. The pointer cell is read a second time on either arm, so each state
 * store goes through a fresh load, and the two arms meet in one shared store
 * after the merge. The counter store fills the guard branch's delay slot in the
 * original. Takes no arguments, carries no frame and returns nothing.
 * This span is the word at byte offset 0x48 of the byte-1 handler table at
 * 0x801F5744 used by callStateTable5744World03, so
 * it runs as state 18; nothing in the payload calls it directly and the
 * dispatch table carries no prototype.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndSetScratchStateToMode20Below15ElseMode13World03Area13413_801F3370(void) {
  u8** slots;
  u8* work;
  u8 value;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  value = work[0x02];
  work[0x02] = value + 1;
  if (value >= 0x0F) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x0D;
  } else {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x14;
  }
}
