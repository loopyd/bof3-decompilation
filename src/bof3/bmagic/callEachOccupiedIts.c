#include "base/types.h"

/* Overlay work-record table at 0x801F0B38: 32 slots of 0x78 bytes, the same
 * 0x78-stride record shape the family's reserveFirstFreeRecord bodies walk.
 * Byte 0 bit 0 is the in-use flag this sweep tests; the word at 0x74 holds the
 * work pointer stored when the record was claimed, as
 * claimRecordsForGlobalWorkByteASeedOrdinalFieldsAndWorkPointerIncrementByteB
 * BmagicMagic00303_801F0078 writes it. The base is bound by this target's map
 * row. */
extern u8 magic00303Records[]; /* @source 0x801F0B38 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row. The original loads it at the head of the body, before the frame is
 * created, for the byte-1 state dispatch, and reloads it inside the sweep to
 * save it across the record step. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared work-object pointer cell at 0x801EB4E0. It sits outside this overlay
 * image, which the generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0
 * already names, and every other claimed source of this target that reads the
 * cell declares it the same way. The sweep saves the cell, publishes the
 * record's own work pointer into it and restores it once the record's step
 * handler returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* In-image work byte-1 handler table at 0x801F0AD8: the ten code pointers
 * 0x801EED14, 0x801EEDAC, 0x801EEE28, 0x801EEEA4, 0x801EEF4C, 0x801EF04C,
 * 0x801EF0A8, 0x801EF12C, 0x801EF1FC and 0x801EF268, which are the overlay's
 * own byte-1 step bodies of the work object. The body indexes it with the work
 * object's unsigned byte 1 scaled by four and does not bound the index. It sits
 * inside the reviewed trailing `textbin` row labelled 0x801F0A78, whose base no
 * map row names, so the offset binds through this target's own map row. */
extern void (*magic00303WorkByte1StateTable[])(void); /* @source 0x801F0AD8 @kind table */

/* This target's byte-1 step dispatcher at 0x801EFA9C, exact through
 * src/bof3/bmagic/dispatchWorkByte1HandlerBmagicMagic00303_801EFA9C.c: it reads
 * the work object published at 0x1F800044 and calls the handler its byte 1
 * selects from the table at 0x801F0B00, so publishing a record there makes it
 * run that record's step. */
void dispatchWorkByte1HandlerBmagicMagic00303_801EFA9C(void);

/* @source 0x801EEC28
 * @behavior Per-frame entry of this overlay. It reads the scratchpad work-object
 *           pointer cell at 0x1F800044 once and calls the entry its unsigned
 *           byte 1 selects from the ten-code-pointer table at 0x801F0AD8, with
 *           no arguments and no bound on the index. It then walks the 32
 *           records of the pool at 0x801F0B38 with stride 0x78 and, for each
 *           record whose byte 0 has bit 0 set, saves the scratchpad work cell
 *           and 0x801EB4E0, loads the record's word 0x74 - the work pointer
 *           stored there when the record was claimed - publishes the record
 *           itself as the work object and that word into 0x801EB4E0, runs the
 *           record's byte-1 step through the dispatcher at 0x801EFA9C and
 *           restores both cells. The flags read re-materialises the pool base,
 *           while the published record pointer reuses one materialised base
 *           held in a saved register, so the original keeps both forms. Takes
 *           no arguments and returns nothing; its 0x28-byte frame only keeps
 *           $ra and $s0 to $s3 across the two calls. No jal and no payload word
 *           names 0x801EEC28, so the overlay loader is its only caller and the
 *           parameter roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callEachOccupiedIts(
    void) {
  u8 index;
  u32 offset;
  u8 *record;
  u8 *savedWork;
  u8 *savedRecord;
  u8 *recordWork;

  magic00303WorkByte1StateTable[g_battle_work[1]]();
  for (index = 0; index < 0x20; index++) {
    offset = index * 0x78;
    if (magic00303Records[offset] & 1) {
      record = &magic00303Records[offset];
      savedWork = g_battle_work;
      savedRecord = D_801EB4E0;
      recordWork = *(u8 **)&magic00303Records[offset + 0x74];
      g_battle_work = record;
      D_801EB4E0 = recordWork;
      dispatchWorkByte1HandlerBmagicMagic00303_801EFA9C();
      D_801EB4E0 = savedRecord;
      g_battle_work = savedWork;
    }
  }
}
