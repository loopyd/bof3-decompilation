#include "bof3/bof3.h"

/* @source 0x801F8394
 * @behavior Work-record claim scan: it walks the 0x8 entries of the
 * 0x1C-byte-stride record table at 0x800E4800 (the main-RAM work area this
 * overlay shares with the world and scenario overlays) and returns the address
 * of the first entry whose lead byte reads zero, or zero when all 0x8 entries
 * are claimed. It reads only each entry's lead byte, writes nothing, makes no
 * call and keeps no register, so its 0x3C bytes carry no frame. The scan is a
 * counter-bounded loop whose bound test is the back edge: the unsigned byte
 * counter is read back masked to 0xFF and compared below 0x8, the record pointer
 * advance (0x1C) is scheduled into that back edge's delay slot, the exhausted
 * path materializes zero in $v0, and a found record leaves the loop through the
 * lead-byte branch with its copy in that branch's delay slot. The lead byte is
 * the claim flag: both callers in this payload test the returned pointer for
 * zero and otherwise pass it in $a0 to an overlay record-seeding helper - jal
 * 0x801F8394 at 0x801F6EB4 inside func_801F6E90 followed by func_801F83D0, and
 * jal 0x801F8394 at 0x801F7AE8 inside func_801F7AB0 followed by func_801F8478.
 * Its 0x3C bytes are instruction-for-instruction identical (branch
 * displacements included) to findFreeWorkRecordScenarioScena0800_801F8B14 at
 * 0x801F8B14 in emi/scenario/scena08/00, which is the same 0x8-entry/0x1C-stride
 * claim scan; that target owns its own address, boundary, map row and source.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordScenarioScena0700_801F8394(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x8u; count++, record += 0x1Cu) {
    if (*record == 0) {
      return record;
    }
  }

  return 0;
}
