#include "bof3/bof3.h"

/* @source 0x801F4D80
 * @behavior Work-record claim scan: it walks the 0x10 entries of the
 *           0x1C-byte-stride record table at 0x800E4800 - the main-RAM work
 *           area this overlay shares with the world and scenario overlays, whose
 *           0x1C-stride byte-0 boundary this target's exact
 *           resetWorkTableAt800E4800AndRearmOneShotLatchesWorld02Area09813_801F4D44
 *           clears right before this span - and returns the address of the first
 *           entry whose lead byte reads zero, or zero once all 0x10 entries are
 *           taken. It reads only each entry's lead byte, writes nothing, makes no
 *           call and keeps no register, so its 0x3C bytes carry no frame. The
 *           scan is a counter-bounded loop whose bound test is the back edge: the
 *           unsigned byte counter is read back masked to 0xFF and compared below
 *           0x10, the record pointer advance (0x1C) is scheduled into that back
 *           edge's delay slot, the exhausted path materializes zero in $v0, and a
 *           found record leaves the loop through the lead-byte branch with its
 *           copy in that branch's delay slot.
 *
 *           The lead byte is the claim flag: the payload's only caller, the exact
 *           incrementPhaseFrameCountSpawnRecordEveryFourthTickRunRecordPassAndAdvanceStateOnExpiryWorld02Area09813_801F3EB0,
 *           reaches this address only when the phase countdown's low two bits read
 *           clear, tests the returned pointer for zero and otherwise hands it to
 *           the overlay record spawn 0x801F4DBC, which seeds the record. The
 *           payload holds no other reference to this address, so it takes no
 *           argument and returns the record pointer.
 *
 *           Shape evidence: the 0x3C-byte span is not byte-identical to any span
 *           of the corpus - a scan of every payload under out/binaries found only
 *           this copy - so no clone applied. Its nearest siblings are the same
 *           scan lowered with another bound or another stride: scena07/00's
 *           findFreeWorkRecordScenarioScena0700_801F8394 (payload offset 0x1794)
 *           and its byte-identical scena08/00 twin at 0x1F14 differ from this
 *           span only in the back-edge bound immediate (0x2C420008, sltiu 0x8,
 *           against 0x2C420010 here), while the copies at emi/world01/area049/13
 *           offset 0x8C4 and emi/world03/area125/13 offset 0x748 differ only in
 *           the pointer advance immediate (addiu 0x28 against 0x1C here). Every
 *           other word, relative branches included, is equal. The statement
 *           order, single record pointer, for-increment order (count++,
 *           record += 0x1Cu) and declaration style follow the exact scena07/00
 *           lift; this function's own bound and stride are written from this
 *           span's bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordWorld02Area09813_801F4D80(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x10u; count++, record += 0x1Cu) {
    if (*record == 0) {
      return record;
    }
  }

  return 0;
}
