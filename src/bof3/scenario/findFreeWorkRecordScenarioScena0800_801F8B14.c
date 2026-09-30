#include "bof3/bof3.h"

/* @source 0x801F8B14
 * @behavior Work-record claim scan: it walks the 0x8 entries of the
 * 0x1C-byte-stride record table at 0x800E4800 (the main-RAM work area this
 * overlay shares with the world and scenario overlays) and returns the address
 * of the first entry whose lead byte reads zero, or zero when all 0x8 entries
 * are claimed. It reads only each entry's lead byte, writes nothing and makes
 * no call, so its 0x3C bytes carry no frame and no saved register. The scan is
 * a counter-bounded loop whose bound test is the back edge: the unsigned byte
 * counter is masked to 0xFF and compared below 0x8, the record pointer advance
 * (0x1C) is scheduled into that back edge's delay slot, the exhausted path
 * materializes zero in $v0, and a found record leaves the loop through the
 * lead-byte branch with its copy in that branch's delay slot. The lead byte is
 * the claim flag: the only caller in this payload, jal 0x801F8B14 at 0x801F7A34
 * inside func_801F79EC, discards a zero result and otherwise passes the
 * returned pointer to func_801F8B50, which writes 1 to that record's byte 0 and
 * seeds bytes 1/2/3/4 with 0/8/0/0x40 and its words at 0x08/0x0C/0x10/0x14 from
 * the scratch work object published at pointer slot 0x1F800044 + 0x44. The
 * shape is the 0x8-entry/0x1C-stride sibling of
 * findFreeWorkRecordScenarioScena0800_801F7DAC (0x40 entries, 0x28 stride) in
 * this same target, which is not byte-identical to it.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordScenarioScena0800_801F8B14(void) {
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
