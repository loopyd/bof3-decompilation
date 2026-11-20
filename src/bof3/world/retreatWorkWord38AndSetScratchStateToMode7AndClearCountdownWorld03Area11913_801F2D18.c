#include "bof3/bof3.h"

extern u8 D_80146920[];

/* @source 0x801F2D18
 * @behavior Third handler of the byte-1 dispatch table based at 0x801F45CC.
 * Saves the work pointer published at the scratchpad cell 0x1F800044 and
 * publishes the record at 0x80146920, subtracts 0x400 from that record's word
 * at offset 0x38 four times, using a bottom-tested unsigned-byte counter, then
 * arms the saved record's state byte at offset 0x01 with mode 7, restores the
 * saved scratchpad pointer and clears the saved record's countdown byte at
 * offset 0x09 to 0. Takes no arguments and returns nothing.
 * The shape is the same overlay's mode-7 handler at 0x801F2DFC, which retreats
 * the neighbouring word at offset 0x34 with the same counter and clears the
 * same countdown byte, and the Area134 handler at 0x801F328C, which retreats
 * the word at offset 0x38 by 0x4000 into a different record, arms mode 16 and
 * sets countdown 8. The role of the word at offset 0x38 is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatWorkWord38AndSetScratchStateToMode7AndClearCountdownWorld03Area11913_801F2D18(void) {
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
    FIELD_REF(u32, work, 0x38) -= 0x400;
    i += 1u;
  } while (i < 4u);
  saved[0x01] = 7;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
  saved[0x09] = 0;
}
