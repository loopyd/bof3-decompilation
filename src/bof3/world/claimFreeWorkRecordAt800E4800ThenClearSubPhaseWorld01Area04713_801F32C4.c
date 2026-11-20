#include "bof3/bof3.h"

/* @source 0x801F32C4
 * @behavior Overlay work-record claim: walks the 0x10 records of the
 *           0x18-byte-stride work table at 0x800E4800 (the main-RAM work area
 *           this overlay shares with the world and scenario overlays) with an
 *           unsigned byte counter, and at the first record whose state byte
 *           +0x14 reads zero it marks that record active by storing 1 into the
 *           same byte and clears the record's sub-phase byte +0x15. It stops at
 *           the first free record and the walk ends after 0x10 records, so a
 *           table with no free record is left untouched. It takes no arguments
 *           and returns nothing.
 *
 *           The role of the two bytes comes from the exact lifts beside it in
 *           this payload: clearWorkTableByte14At800E4800World01Area04713_801F3298
 *           zeroes byte +0x14 of the same block as the record release, and
 *           updateWorkTableRecordsAndReportActiveWorld01Area04713_801F330C
 *           skips every record whose +0x14 reads zero, arms a record whose
 *           sub-phase byte +0x15 is 0 and increments that byte, so a record
 *           claimed here enters the next step pass unarmed. Its only in-payload
 *           caller is func_801F2CC8 (`jal` at 0x801F2CF8), which reaches it
 *           whenever the shared frame word D_80143E6C is a multiple of 8, i.e.
 *           on a fixed spawn period; that caller ignores $v0, so the call site
 *           agrees with the no-argument void prototype. The prologue-less body
 *           and the `jr $ra` at 0x801F3304 prove the 0x48-byte boundary.
 *
 *           Shape notes from the original bytes: the counter is a byte (masked
 *           to 0xFF, compared below 0x10) in a loop whose entry test the
 *           compiler drops because the zero initial value makes it provably
 *           true, and the constant 1 is hoisted out of the body. A bottom-tested
 *           `do`/`while` carrying the same early exit instead peeled the first
 *           iteration and measured 8 of 18 instructions with a 116-byte body,
 *           while this top-tested loop and its `while` form both measured 18 of
 *           18 instructions at 72 bytes; the loop shape, not its spelling of
 *           the test, is what the original encodes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeWorkRecordAt800E4800ThenClearSubPhaseWorld01Area04713_801F32C4(
    void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);

  for (i = 0u; i < 0x10u; i += 1u) {
    if (work[0x14] == 0) {
      work[0x14] = 1;
      work[0x15] = 0;
      break;
    }
    work += 0x18u;
  }
}
