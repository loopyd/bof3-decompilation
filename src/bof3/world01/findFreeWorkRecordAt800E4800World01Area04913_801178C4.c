#include "bof3/bof3.h"

/* @source 0x801178C4
 * @behavior Work-record claim scan of this overlay's 0x800E4800 work table (the
 * 16 records of 0x28-byte stride whose lead byte is the record's claim flag and
 * whose byte 1 selects the record's per-frame handler): it returns the address of
 * the first record whose lead byte reads zero, or zero when all 16 records are
 * claimed. Both callers, func_801176B8 and func_80117768, store the result in the
 * shared record cursor D_801181DC, skip the rest of their frame when it is zero,
 * and otherwise pass it to func_80117864, which initialises that record and sets
 * its lead byte to 1. It reads only each record's lead byte, writes nothing,
 * makes no call and keeps no register, so its 0x3C bytes carry no frame. The scan
 * is a counter-bounded loop whose bound test is the back edge: the unsigned byte
 * counter is read back masked to 0xFF and compared below 0x10, the record pointer
 * advance (0x28) is scheduled into that back edge's delay slot, the exhausted path
 * materialises zero in $v0, and a free record leaves the loop through the lead-byte
 * branch with its copy in that branch's delay slot. Its shape, table and stride are
 * those of the exact sibling findFreeWorkRecordAt800E4800World03Area13413_801F4938
 * (0x20 records) and of this overlay's own clearWorkTable helper, and the adjacent
 * func_80117864 at 0x80117864 is the matching claim step.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordAt800E4800World01Area04913_801178C4(void) {
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
