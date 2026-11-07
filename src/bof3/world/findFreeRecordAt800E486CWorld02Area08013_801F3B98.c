#include "bof3/bof3.h"

/* @source 0x801F44A4 @kind unknown — cursor into the 8-entry, 0x60-byte-stride
 * record table that starts at 0x800E486C. The lifted sibling at 0x801F3834
 * caches the table base in it, the lifted sibling at 0x801F3870 republishes it
 * as it walks the entries, and this function advances it one record at a time
 * while scanning for a free (lead-byte-zero) record, so the cursor ends on the
 * record it returns. */
extern u8* D_801F44A4;

/* @source 0x801F3B98
 * @behavior Claims the first free record of the eight-entry, 0x60-byte-stride
 *           record table whose base is 0x800E486C: it publishes the base as the
 *           shared cursor D_801F44A4, then reads the cursor back and walks it one
 *           record at a time, and returns the first record whose lead byte at
 *           +0x00 reads zero while leaving the cursor on that record. When all
 *           eight records carry a non-zero lead byte it returns zero and leaves
 *           the cursor one stride past the last entry. It takes no arguments,
 *           writes nothing but the cursor, makes no call and keeps no register,
 *           so its 0x5C bytes carry no stack frame. Its single caller, the
 *           handler at 0x801F395C, stores the result back into D_801F44A4,
 *           returns early when the result is zero and otherwise seeds that
 *           record's +0x18/+0x1C words from the scratchpad work record before
 *           calling rand, so the lead byte is the record's claim flag and the
 *           0x60 stride covers the payload's largest work record.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: one global store of the base literal
 * (`lui`/`ori`/`sw`), an unsigned byte counter seeded after that store, and a
 * bottom-tested loop whose bound test is the back edge: the counter is read back
 * masked to 0xFF and compared below 8 with `sltiu`, the next-record address is
 * the value `addiu`d into the lead-byte branch's delay slot, the free record
 * leaves through the branch-free path with its copy in the `j` delay slot, and
 * the exhausted path materialises zero in the back edge's delay slot. The loop
 * reloads D_801F44A4 from its own `lui`/`lw` pair on every iteration instead of
 * carrying a local record pointer, which is why the published cursor is the walk
 * state. Table base 0x800E486C is main RAM outside this payload's own 0x801F2C00
 * range and is not 0x28-stride aligned inside the 0x800E4800 work table, so it is
 * a distinct table here and the name stays anchored on its address.
 */
u8* findFreeRecordAt800E486CWorld02Area08013_801F3B98(void) {
  u8* record;
  u8 count;

  D_801F44A4 = PSX_PTR(u8, 0x800E486Cu);
  count = 0u;

  while (count < 8u) {
    record = D_801F44A4;
    if (*record == 0) {
      return record;
    }
    D_801F44A4 = record + 0x60u;
    count += 1u;
  }

  return 0;
}
