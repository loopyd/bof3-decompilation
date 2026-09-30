#include "bof3/bof3.h"

/* @source 0x801F7FF8
 * @behavior Free-record claim scan of this overlay's work table: it walks the
 * 0x80 entries of the 0x28-byte-stride record table at 0x800E4800 (the main-RAM
 * work area this overlay shares with the world and the other scenario overlays)
 * and returns the address of the first entry whose byte at offset 0x25 reads
 * zero, or zero when every entry is claimed. It reads only that byte, writes
 * nothing, makes no call and keeps no register, so its 0x3C bytes carry no
 * frame. The scan is a counter-bounded loop whose bound test is the back edge:
 * the unsigned byte counter is masked to 0xFF and compared below 0x80 (so
 * entries 0 through 0x7F are examined), the record pointer advance (0x28) is
 * scheduled into that back edge's delay slot, the exhausted path materializes
 * zero in $v0, and a found record leaves the loop through the byte branch with
 * its copy in that branch's delay slot. Byte 0x25 is the claim marker: the three
 * in-payload claimers write it non-zero after a successful scan (0x80 at
 * 0x801F79D0 in func_801F78F4 and 0x801F7B6C in func_801F7B00, 0x40 at
 * 0x801F7C54 in func_801F7BC0), and this overlay's clear handler
 * clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC (0x801F7FCC) zeroes
 * byte 0x25 for all 0x80 entries, which is what frees them again. All three
 * callers jal this address (0x801F7934, 0x801F7B18 and 0x801F7BEC) and all three
 * test the returned pointer for zero before filling the record. Its 0x3C bytes
 * have the same scan shape as findFreeWorkRecordScenarioScena0800_801F7DAC
 * (0x801F7DAC, emi/scenario/scena08/00), which scans the lead byte of the same
 * 0x800E4800 table at the same 0x28 stride but over 0x40 entries; only the
 * claim-byte offset and the bound differ, so this target owns its own address,
 * boundary and source.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordByte25ScenarioScena0400_801F7FF8(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x80u; count++, record += 0x28u) {
    if (record[0x25] == 0) {
      return record;
    }
  }

  return 0;
}
