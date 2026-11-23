#include "bof3/bof3.h"

/* @source 0x801F3158
 * @behavior Free-record scan of the shared work table at 0x800E4800 (the
 *           main-RAM work area this overlay shares with the world and scenario
 *           overlays): it walks the occupancy byte 0x15 of each of the 0x10
 *           records of 0x18-byte stride and returns the address of the first
 *           record whose byte 0x15 reads zero, or zero when all 0x10 records are
 *           occupied. It reads only that byte, writes nothing, makes no call and
 *           keeps no saved register, so its 0x3C bytes carry no prologue and no
 *           frame. The bound test is the loop's back edge: the unsigned byte
 *           counter is read back masked to 0xFF and compared below 0x10, the
 *           record advance of 0x18 is scheduled into that back edge's delay slot,
 *           the exhausted path materialises zero in $v0, and a free record leaves
 *           the loop through the occupancy branch with its copy in that branch's
 *           delay slot. Its only in-image caller,
 *           claimFourRecordsAround,
 *           calls it at 0x801F30A4, skips the rest of that pass when the result
 *           is zero and otherwise stores 1 into byte 0x15 of the returned
 *           record, so the byte this scan tests is that caller's occupancy flag.
 *           The adjacent stateWorkTableByte15
 *           zeroes byte 0x15 of the same 0x10 records of the same 0x18-byte
 *           stride, releasing the block this scan fills.
 *
 * Name evidence: the 15-word body is that of the free-record scans lifted at
 *           findFreeWorkRecordAt800E4800World03Area12513_801F3348 and
 *           findFreeWorkRecordAt800E4800World01Area04913_801178C4. This span's
 *           words equal the world03/area125 body's words except the load offset
 *           (0x15 here, 0x00 there) and the record stride (0x18 here, 0x28
 *           there), and the world01/area049 body is word-equal to the world03
 *           one, so the two are shape evidence and not clone proof of this span.
 *           The non-lead occupancy byte is spelled as in
 *           findFreeWorkRecordByte3At800E4800World04Area17413_801F2E88 and as
 *           the adjacent clear helper spells the byte it clears. The 0x3C-byte
 *           boundary runs from this address to the next declared start,
 *           0x801F3194.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* queryFreeRecordByte15(void) {
  u8* record;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x10u; count++, record += 0x18u) {
    if (record[0x15] == 0) {
      return record;
    }
  }

  return 0;
}
