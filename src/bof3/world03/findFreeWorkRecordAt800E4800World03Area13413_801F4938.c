#include "bof3/bof3.h"

/* @source 0x801F4938
 * @behavior Work-record claim scan: it walks the 0x20 entries of the
 * 0x28-byte-stride record table at 0x800E4800 (the main-RAM work area this
 * overlay shares with the world and scenario overlays) and returns the address
 * of the first entry whose lead byte reads zero, or zero when all 0x20 entries
 * are claimed. It reads only each entry's lead byte, writes nothing, makes no
 * call and keeps no register, so its 0x3C bytes carry no frame. The scan is a
 * counter-bounded loop whose bound test is the back edge: the unsigned byte
 * counter is read back masked to 0xFF and compared below 0x20, the record
 * pointer advance (0x28) is scheduled into that back edge's delay slot, the
 * exhausted path materializes zero in $v0, and a found record leaves the loop
 * through the lead-byte branch with its copy in that branch's delay slot; the
 * lead byte is the claim flag. Its shape is the 0x20-entry/0x28-stride form of
 * the exact siblings findFreeWorkRecordScenarioScena0800_801F7DAC (0x40
 * entries, 0x28 stride) and findFreeWorkRecordScenarioScena0700_801F8394 (0x8
 * entries, 0x1C stride), and its table, stride and count are those of the exact
 * adjacent clearWorkTableAt800E4800World03Area13413_801F4974 at 0x801F4974,
 * which clears the lead byte of the same 0x20 entries; this target owns its own
 * address, boundary, map row and source.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordAt800E4800World03Area13413_801F4938(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x20u; count++, record += 0x28u) {
    if (*record == 0) {
      return record;
    }
  }

  return 0;
}
