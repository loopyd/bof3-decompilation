#include "bof3/bof3.h"

extern u8 D_80146888[];

/* @source 0x801F2F3C
 * @behavior Overlay work-record handoff: saves the work-record pointer
 * published at the scratchpad pointer slot 0x1F800044 (cell 0x11), publishes
 * the record based at 0x80146888 in its place and keeps that same record
 * address as the loop cursor, ramps the record's 32-bit word at offset 0x34 by
 * 0x4000 four times (net +0x10000) with an unsigned-byte counter in a
 * bottom-tested loop, then arms the saved record's state byte at offset 0x01
 * with 7 and republishes the saved record, so the saved record is current again
 * with mode 7 selected. The body is the 0x54-byte shape of the exact
 * same-target sibling
 * advanceWorkWordAndSetScratchStateToMode5World03Area13413_801F2E68 and is
 * instruction-identical to it except for the immediate the state byte receives:
 * 7 here instead of 5. The byte armed is the record state byte the
 * dispatchStateByte1Table* siblings dispatch through; 7 is decimal here. The
 * record base 0x80146888 is the shared 0x98-byte work-record table this overlay
 * uses, and its offset 0x34 word is the record's first coordinate word. Takes
 * no arguments, carries no frame and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWordAndSetScratchStateToMode7World03Area13413_801F2F3C(void) {
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
    FIELD_REF(u32, work, 0x34) += 0x4000;
    i += 1u;
  } while (i < 4u);
  saved[0x01] = 7;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
}
