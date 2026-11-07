#include "bof3/bof3.h"

/* @source 0x801F3A9C
 * @behavior Work-record claim scan: it walks the 0x80 records of the
 *           0xE-byte-stride record table at 0x800E4800 (the main-RAM work area
 *           this overlay shares with the world and scenario overlays) and
 *           returns the address of the first record whose +0xC halfword counts
 *           zero (read as a signed halfword, the same `lh` the overlay's own
 *           ticker func_801F39D8 uses on that field), or zero when all 0x80
 *           records are busy. The countdown halfword is the free marker: the
 *           exact sibling clearWorkRecordCountdownsAt800E4800World01Area06714_801F39AC
 *           at 0x801F39AC clears that field over the same table and the re-arm
 *           helper func_801F3D54 stores 0x20 there. The scan reads each record's
 *           halfword, writes nothing, makes no call and keeps no frame, so its
 *           0x3C bytes carry no prologue. The walk is a bottom-tested loop whose
 *           bound test is the back edge: the unsigned byte counter is read back
 *           masked to 0xFF and compared below 0x80, the record pointer advance
 *           (0xE) and the counter increment are those two blocks, a free record
 *           leaves through the halfword branch with its address copied in that
 *           branch's delay slot, and the exhausted path materializes zero in
 *           $v0. The counter idiom is byte-for-byte the one in the exact sibling
 *           above; the 0x80-entry/0xE-stride countdown form differs from the
 *           lead-byte claim scans
 *           findFreeWorkRecordAt800E4800World03Area13413_801F4938 and
 *           findFreeWorkRecordAt800E4800World01Area04913_801178C4, so this
 *           target owns its own address, boundary, map row and source.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordByCountdownAt800E4800World01Area06714_801F3A9C(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);
  count = 0u;

  do {
    if (FIELD_REF(s16, record, 0xCu) != 0) {
      record += 0xEu;
      count += 1u;
    } else {
      return record;
    }
  } while (count < 0x80u);

  return 0;
}
