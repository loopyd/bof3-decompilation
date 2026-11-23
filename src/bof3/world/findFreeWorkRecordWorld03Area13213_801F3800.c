#include "bof3/bof3.h"

/* The overlay's own record-table cursor. The original materialises the cell
 * address and reloads the published pointer once per record, and this target's
 * reset stateTableClaimBytes and per-frame pass
 * tickDrawActiveRecords walk the same
 * 0x18-byte-stride table at 0x800E4800 through this same cell. */
extern u8* D_801F4A88; /* @source 0x801F4A88 @kind data */

/* @source 0x801F3800
 * @behavior Work-record free-entry scan: publishes the base 0x800E4800 of the
 *           0x18-byte-stride work table in main RAM into this overlay's own
 *           cursor cell 0x801F4A88, then walks at most 0x20 records, reloading
 *           the published cursor and reading the claim byte at offset 0 of the
 *           record it names. The first record whose claim byte reads zero is
 *           returned with the cursor left pointing at it; each claimed record
 *           advances the published cursor by 0x18 and an unsigned byte counter,
 *           so an exhausted walk returns zero with the cursor one stride past
 *           the last record. It takes no arguments, makes no call and reads no
 *           state beyond the table it publishes.
 *
 * Field evidence: the lead byte this scan tests for zero is the record claim
 * byte. The reset helper stateTableClaimBytes
 * clears that same byte in every record, and the seeding helper
 * stateRecordDefaultsPosition writes the
 * same lead byte to 1 when it takes the record this scan returned, so zero is
 * the unclaimed value.
 *
 * Caller evidence: the two in-payload call sites - jal 0x801F3800 at 0x801F2EAC
 * inside func_801F2E88 and at 0x801F2FD0 inside func_801F2F98 - both test the
 * returned pointer for zero and otherwise pass it in $a0 to
 * stateRecordDefaultsPosition, so the scan
 * feeds the overlay's claim path and its result is a record pointer.
 *
 * Shape evidence: this is the repository's free-record scan family
 * (findFreeWorkRecordScenarioScena0800_801F8B14 and
 * findFreeWorkRecordScenarioScena0700_801F8394 are its 0x1C-stride
 * counter-pointer members), but those walk the table from a local pointer and
 * write nothing, while this span reloads and republishes the overlay cursor cell
 * on the claimed path; the table base, stride, bound, tested field and returned
 * value here are written from this span's own original bytes, and the cell form
 * follows this target's own exact reset sibling.
 *
 * Representation: the record the cell publishes is loaded once per iteration
 * into a local and reused for both the claim-byte read and the returned value,
 * which is the single reload the original shows; the cursor update of the claimed
 * path is the sum of that local and the stride. The scan is a top-tested counter
 * loop, the form the free-record scan family uses; spelling it as the reset
 * sibling's bottom-tested do-while measured 20/23 instructions at 92->96 bytes
 * with the first difference on that zero test's polarity, while the form below
 * measured 23/23 at 92->92 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecordWorld03Area13213_801F3800(void) {
  u8* record;
  u8 index;

  D_801F4A88 = PSX_PTR(u8, 0x800E4800u);
  for (index = 0u; index < 0x20u; index++) {
    record = D_801F4A88;
    if (*record == 0) {
      return record;
    }
    D_801F4A88 = record + 0x18;
  }

  return 0;
}
