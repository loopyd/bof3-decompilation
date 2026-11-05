#include "bof3/bof3.h"

/* @source 0x80146920 @kind unknown */
extern u8 workRecordAt80146920World03Area13413[];

/* @source 0x801F3898
 * @behavior Saves the scratchpad work pointer at 0x1F800044, publishes the
 * work record at 0x80146920, adds 0x4000 to its word at offset 0x34 four
 * times, sets the saved record's state byte at offset 1 to mode 7, and
 * restores the saved work pointer. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWordAndSetScratchStateToMode7World03Area13413_801F3898(void) {
  u8** slots;
  u8* saved;
  u8* work;
  u8 i;

  i = 0u;
  slots = SPAD_PTR_TABLE(u8);
  saved = slots[0x11];
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = (u8*)&workRecordAt80146920World03Area13413;
  work = (u8*)&workRecordAt80146920World03Area13413;
  do {
    FIELD_REF(u32, work, 0x34) += 0x4000;
    i += 1u;
  } while (i < 4u);
  saved[0x01] = 7;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
}
