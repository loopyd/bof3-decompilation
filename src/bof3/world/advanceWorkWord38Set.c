#include "bof3/bof3.h"

extern u8 D_80146920[];

/* @source 0x801F2C88
 * @behavior Second handler of the byte-1 dispatch table based at 0x801F45CC.
 * Saves the work pointer published at the scratchpad cell 0x1F800044 and
 * publishes the record at 0x80146920, adds 0x400 to that record's word at
 * offset 0x38 four times, using a bottom-tested unsigned-byte counter, then
 * arms the saved record's state byte at offset 0x01 with mode 2 so the
 * dispatcher reaches the countdown handler at 0x801F2CE0 next, restores the
 * saved scratchpad pointer and sets the saved record's countdown byte at
 * offset 0x09 to 4. Takes no arguments and returns nothing.
 * The shape is the same overlay's mode-7 handler at 0x801F2D18, which walks the
 * same word at offset 0x38 in the opposite direction with the same counter and
 * clears the same countdown byte, and the Area134 handler at 0x801F31FC, which
 * advances that word by 0x4000 twice against a different record, arms mode 14
 * and sets countdown 8. The role of the word at offset 0x38 is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord38Set(void) {
  u8** slots;
  u8* saved;
  u8* work;
  u8 i;

  i = 0u;
  slots = SPAD_PTR_TABLE(u8);
  saved = slots[0x11];
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = (u8*)&D_80146920;
  work = (u8*)&D_80146920;
  do {
    FIELD_REF(u32, work, 0x38) += 0x400;
    i += 1u;
  } while (i < 4u);
  saved[0x01] = 2;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
  saved[0x09] = 4;
}
