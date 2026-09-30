#include "bof3/bof3.h"

/* @source 0x801F9DD4
 * @behavior Free-slot claim scan of the overlay's second work pool: it walks
 * the 0x40 entries of the 0x20-byte-stride table at 0x800E515C and returns the
 * address of the first entry whose lead byte reads zero, or zero when all 0x40
 * entries are claimed. It reads only each entry's lead byte, writes nothing,
 * makes no call and keeps no register, so its 0x3C bytes carry no frame. The
 * scan is a counter-bounded loop whose bound test is the back edge: the
 * unsigned byte counter is read back masked to 0xFF and compared below 0x40,
 * the entry pointer advance (0x20) is scheduled into that back edge's delay
 * slot, the exhausted path materializes zero in $v0, and a found entry leaves
 * the loop through the lead-byte branch with its copy in that branch's delay
 * slot. The lead byte is the claim flag the pool's seeder sets: the scan's
 * single in-image reference is the jal at 0x801F74FC inside func_801F7480,
 * which tests the returned pointer for zero and otherwise passes it in $a0 to
 * the seeding helper func_801F9E10 (that helper stores 1 in the entry's lead
 * byte, 0 in its byte at +0x01 and 0x10 in its countdown byte at +0x02, which
 * is the 0x20-byte record layout this scan strides over). The sibling clear at
 * 0x801F9DA8 empties the same 0x40x0x20 pool with the identical
 * counter-bounded stride loop, and the 0x3C-byte shape here is
 * instruction-for-instruction identical to findFreeWorkRecordScenarioScena0700_801F8394
 * at 0x801F8394 (same scan over the 0x8 entries of the 0x1C-byte-stride pool
 * at 0x800E4800); only the base, stride and bound immediates differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordAt800E515CScenarioScena0700_801F9DD4(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E515Cu);

  for (count = 0u; count < 0x40u; count++, record += 0x20u) {
    if (*record == 0) {
      return record;
    }
  }

  return 0;
}
