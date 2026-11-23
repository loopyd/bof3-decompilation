#include "bof3/bof3.h"

/* @source 0x801F3348
 * @behavior Free-record scan of the shared work table at 0x800E4800: it walks
 *           the lead byte of each of the 0x10 records of 0x28-byte stride and
 *           returns the address of the first record whose lead byte reads zero,
 *           or zero when all 0x10 records are claimed. It reads only each
 *           record's lead byte, writes nothing, calls nothing and keeps no
 *           register, so its 0x3C bytes carry no prologue and no frame. The
 *           bound test is the loop's back edge: the unsigned byte counter is read
 *           back masked to 0xFF and compared below 0x10, the record pointer
 *           advance of 0x28 is scheduled into that back edge's delay slot, the
 *           exhausted path materialises zero in $v0, and a free record leaves the
 *           loop through the lead-byte branch with its copy in that branch's
 *           delay slot. The lead byte is the claim flag: this target's single
 *           in-image caller,
 *           claimFreeRingDirection,
 *           calls it at 0x801F31BC, returns without writing anything when the
 *           result is zero and otherwise stores 1 into that same lead byte.
 *
 * Name evidence: the role and the 0x800E4800 table are those of the exact
 *           siblings findFreeWorkRecordAt800E4800World01Area04913_801178C4 and
 *           findFreeWorkRecordAt800E4800World03Area13413_801F4938, which carry
 *           the same 0x3C-byte body and the same 0x28 stride with 0x10 and 0x20
 *           records. This span's 0x3C bytes equal the world01/area049 body's
 *           words exactly, and the world03/area134 body differs from it in the
 *           single bound immediate alone, so the count 0x10 is this target's own
 *           and agrees with its adjacent
 *           stateWorkTableAt800, which clears the
 *           lead byte of the same 0x10 records of this table. The scenario
 *           overlays hold the 0x40 and 0x8-entry forms of the same scan.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* queryFreeWorkRecord(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x10u; count++, record += 0x28u) {
    if (*record == 0) {
      return record;
    }
  }

  return 0;
}
