#include "bof3/bof3.h"

/* @source 0x801F7DAC
 * @behavior Work-record claim scan: it walks the 0x40 entries of the
 * 0x28-byte-stride record table at 0x800E4800 (the main-RAM work area this
 * overlay shares with the world and scenario overlays) and returns the address
 * of the first entry whose lead byte reads zero, or zero when all 0x40 entries
 * are claimed. It reads only each entry's lead byte, writes nothing, makes no
 * call and keeps no register, so its 0x3C bytes carry no frame. The scan is a
 * counter-bounded loop whose bound test is the back edge: the unsigned byte
 * counter is masked to 0xFF and compared below 0x40 (so entries 0 through 0x3F
 * are examined), the record pointer advances in the iteration expression and is
 * scheduled into that back edge's delay slot, the exhausted path materializes
 * zero in $v0, and a found record leaves the loop through the lead-byte branch
 * with its copy in that branch's delay slot. The lead byte is the claim flag:
 * the only caller in this payload, func_801F802C at 0x801F8038, tests the
 * returned pointer for zero and then writes 1 to byte 0 and 0 to byte 2 of that
 * record and fills its words at 0x04/0x08/0x0C from the scratch work object
 * published at pointer slot 0x1F800044 + 0x44.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordScenarioScena0800_801F7DAC(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x40u; count++, record += 0x28u) {
    if (*record == 0) {
      return record;
    }
  }

  return 0;
}
