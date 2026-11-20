#include "bof3/bof3.h"

extern u8 D_80146920[];

/* @source 0x801F2D6C
 * @behavior Handler at index 4 of the byte-1 dispatch table based at
 * 0x801F45CC. Saves the work pointer published at the scratchpad cell
 * 0x1F800044 and publishes the record at 0x80146920, adds 0x400 to that record
 * word at offset 0x34 four times, using a bottom-tested unsigned-byte counter,
 * then arms the saved record state byte at offset 0x01 with mode 5 so the
 * dispatcher reaches the countdown handler at 0x801F2DC4 next, restores the
 * saved scratchpad pointer and sets the saved record countdown byte at offset
 * 0x09 to 4. Takes no arguments and returns nothing.
 * The shape is the same overlay handler at 0x801F2C88, which advances the
 * neighbouring word at offset 0x38 with the same counter, arms mode 2 and sets
 * the same countdown value, and the mirror at 0x801F2DFC, which retreats the
 * same word at offset 0x34 four times, arms mode 7 and clears that countdown
 * byte. The role of the word at offset 0x34 is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord34AndSetScratchStateToMode5WithCountdownWorld03Area11913_801F2D6C(void) {
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
    FIELD_REF(u32, work, 0x34) += 0x400;
    i += 1u;
  } while (i < 4u);
  saved[0x01] = 5;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
  saved[0x09] = 4;
}
