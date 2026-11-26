#include "bof3/bof3.h"

/* @source 0x80146920 @kind unknown */
extern u8 workRecordRow[];

/* @source 0x801F3C44
 * @behavior Saves the work pointer at scratchpad cell 0x1F800044 and publishes
 * the record at 0x80146920. Adds 0x4000 to that record's word at offset 0x38
 * twice, using a bottom-tested unsigned-byte counter. Sets byte 1 of the saved
 * record to mode 18, then restores the saved scratchpad pointer. Takes no
 * arguments and returns nothing. The role of the word at offset 0x38 is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceRowWord38Mode18(void) {
  u8** slots;
  u8* saved;
  u8* work;
  u8 i;

  i = 0u;
  slots = SPAD_PTR_TABLE(u8);
  saved = slots[0x11];
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = (u8*)&workRecordRow;
  work = (u8*)&workRecordRow;
  do {
    FIELD_REF(u32, work, 0x38) += 0x4000;
    i += 1u;
  } while (i < 2u);
  saved[0x01] = 18;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
}
