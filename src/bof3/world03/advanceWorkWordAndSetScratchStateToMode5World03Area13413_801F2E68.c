#include "bof3/bof3.h"

extern u8 D_80146888[];

/* @source 0x801F2E68
 * @behavior Overlay work-record handoff: saves the work-record pointer
 * published at the scratchpad pointer slot 0x1F800044 (cell 0x11), publishes
 * the record based at 0x80146888 in its place and keeps that same record
 * address as the loop cursor, ramps the record's 32-bit word at offset 0x34 by
 * 0x4000 four times (net +0x10000) with an unsigned-byte counter in a
 * bottom-tested loop, then arms the saved record's state byte at offset 0x01
 * with 5 and republishes the saved record, so the saved record is current again
 * with mode 5 selected. The state byte it arms is the byte the exact
 * same-target siblings
 * setScratchStateToMode1WhenPadMatchesWorld03Area13413_801F3E50,
 * resetScratchStateToMode1World03Area13413_801F35A4 and
 * countdownScratchStateToMode15World03Area13413_801F3254 arm, and the byte the
 * dispatchStateByte1Table* siblings dispatch through; 5 is decimal here. The
 * record base 0x80146888 is the shared 0x98-byte work-record table this overlay
 * uses (the neighbouring func_801F2DE8 loads the same record's word at
 * 0x801468BC, i.e. record + 0x34, and compares it against 0x80145EC4), and its
 * offset 0x34 word is the record's first coordinate word - the scenario sources
 * read the same offset out of the published record as `*(s32*)(D_1F800044 +
 * 0x34)` for the `>> 9` - 0x4000 projection
 * (src/bof3/scenario/drawAdoptedRecordCursorBeam.c,
 * src/bof3/scenario/drawLinkedRecordCursorBeam.c). The slot-1 twins at 0x801F36F0,
 * 0x801F37C4, 0x801F3898, 0x801F396C, 0x801F3A40, 0x801F3B24, 0x801F3BB4 and
 * 0x801F3C44 are the same 0x54-byte shape against the next entry at 0x801F46920,
 * and 0x801F2D94, 0x801F2F3C, 0x801F3010, 0x801F30E4, 0x801F31FC, 0x801F328C
 * and 0x801F331C are the same shape against 0x80146888 with other modes. Takes
 * no arguments, carries no frame and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWordAndSetScratchStateToMode5World03Area13413_801F2E68(void) {
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
  saved[0x01] = 5;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11] = saved;
}
