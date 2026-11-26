#include "bof3/bof3.h"

/* @source 0x80146920 @kind unknown */
extern u8 workRecordRow[];

/* @source 0x801F3BB4
 * @behavior Saves the work pointer at scratchpad cell 0x1F800044 and publishes
 * the record at 0x80146920. Subtracts 0x4000 from that record's word at offset
 * 0x38 four times, using a bottom-tested unsigned-byte counter. Sets byte 1
 * of the saved record to mode 16, restores the saved scratchpad pointer, then
 * sets the saved record's countdown byte at offset 9 to 8. Takes no arguments
 * and returns nothing. The role of the word at offset 0x38 is unknown.
 * The 0x58-byte body is the same walk as the sibling at 0x801F328C: comparing
 * the two payload spans leaves one differing word of the 22, the low half of
 * the published record address that body spells 0x80146888, so the loop count
 * of 4, the 0x4000 retreat, state 16 and the trailing countdown of 8 here were
 * each read back from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatWorkWord38Set(void) {
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
    FIELD_REF(u32, work, 0x38) -= 0x4000;
    i += 1u;
  } while (i < 4u);
  saved[0x01] = 16;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
  saved[0x09] = 8;
}
