#include "bof3/bof3.h"

extern u8 D_80146888[];

/* @source 0x801F331C
 * @behavior Saves the work pointer at scratchpad cell 0x1F800044 and publishes
 * the record at 0x80146888. Adds 0x4000 to that record's word at offset 0x38
 * twice, using a bottom-tested unsigned-byte counter. Sets byte 1 of the saved
 * record to mode 18, then restores the saved scratchpad pointer. Takes no
 * arguments and returns nothing. The role of the word at offset 0x38 is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord38AndSetScratchStateToMode18World03Area13413_801F331C(void) {
  u8** slots;
  u8* saved;
  u8* work;
  u8 i;

  i = 0u;
  slots = SPAD_PTR_TABLE(u8);
  saved = slots[0x11];
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = (u8*)&D_80146888;
  work = (u8*)&D_80146888;
  do {
    FIELD_REF(u32, work, 0x38) += 0x4000;
    i += 1u;
  } while (i < 2u);
  saved[0x01] = 18;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
}
