#include "bof3/bof3.h"

/* @source 0x801F2E88
 * @behavior Work-record claim scan: walks the 0x20 records of the 0x2C-byte-
 * stride table at 0x800E4800 (the main-RAM work area this overlay shares with
 * the world and scenario overlays) and returns the address of the first record
 * whose occupancy byte at offset 3 reads zero, or zero when all 0x20 records
 * are claimed. It reads only that byte, writes nothing and makes no call, so
 * its 0x3C bytes carry no frame and no saved register. The scan is a
 * counter-bounded loop whose bound test is the back edge: the unsigned halfword
 * counter is masked to 0xFFFF and compared below 0x20, the record advance (0x2C)
 * is scheduled into that back edge's delay slot, the exhausted path
 * materializes zero in $v0, and a found record leaves the loop through the
 * occupancy branch with its copy in that branch's delay slot. Both callers in
 * this payload, jal 0x801F2E88 at 0x801F2CCC inside func_801F2CA8 and at
 * 0x801F2D8C inside func_801F2D68, discard a zero result and otherwise pass the
 * returned pointer to func_801F3378, which writes 1 to that record's byte 3 and
 * seeds the record from the scratch work object published at pointer slot
 * 0x1F800044. The shape is the 0x20-entry/0x2C-stride/byte-3 sibling of
 * findFreeWorkRecordScenarioScena0800_801F8B14 in emi/scenario/scena08/00
 * (0x8 entries, 0x1C stride, lead byte), which differs from it in those four
 * immediates and is not byte-identical to it.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordByte3At800E4800World04Area17413_801F2E88(void) {
  u8* record;
  u16 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x20u; count++, record += 0x2Cu) {
    if (record[3] == 0) {
      return record;
    }
  }

  return 0;
}
