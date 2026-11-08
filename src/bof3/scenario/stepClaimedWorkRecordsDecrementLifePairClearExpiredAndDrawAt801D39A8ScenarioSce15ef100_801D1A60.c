#include "bof3/bof3.h"

/* Table of 0x10 records of 0x18 bytes at 0x801D39A8 inside this overlay. The
 * lead byte of each record is its claim flag: the record claim at 0x801D19C8
 * sets it to 1 and fills the rest of the record from the scratchpad work object
 * at 0x1F800044, and this body gates its per-record update on it and clears it
 * when the record byte-2 countdown expires. The bytes sit inside the Splat
 * `textbin` row at payload offset 0x2D18 and are zero on disc, so the run is
 * only referenced here. */
extern u8 D_801D39A8[]; /* @source 0x801D39A8 @kind data */

/* Resident projection helper at 0x801AFE18: it fills the 0x20-byte scratch
 * block this body passes in $a0 and returns nothing. Resolved from this
 * target's generated undefined-function rows. */
void func_801AFE18(void* arg0);

/* In-payload draw helper at 0x801D1B10: it takes the record base in $a0 and
 * emits the record's quad strip from the halfword at record +0x14. */
void func_801D1B10(u8* record);

/* @source 0x801D1A60
 * @behavior Per-frame pass over this overlay's 0x10 work records (the
 * 0x18-byte-stride claim table at 0x801D39A8), one of the two helpers every
 * step handler of the overlay run at 0x801D3918 calls before it draws. It
 * first fills the 0x20-byte projection scratch through the resident helper
 * 0x801AFE18, then walks the table with an unsigned byte counter, gating on the
 * claim byte at +0x0 of each record. For every claimed record it raises the
 * return flag, subtracts 0x10 from the halfword at +0x14, subtracts 1 from the
 * countdown byte at +0x2, clears the claim byte of that record when the
 * countdown reaches zero, and hands the record base to the in-payload draw
 * helper 0x801D1B10. It returns 1 once any record was claimed and 0 when the
 * whole table is free; the five step handlers that call it ignore the result.
 * The 0x48-byte frame holds the 0x10-byte outgoing argument area, the 0x20-byte
 * projection scratch at +0x10 and the saved $ra with $s0-$s3 of the walk.
 *
 * Name evidence: the table is the work-record table this target already names
 * at 0x801D39A8 (the record claim at 0x801D19C8 seeds the same life pair, the
 * flag clear at 0x801D199C clears the same claim bytes), and byte 2 with the
 * halfword at 0x14 are the record life pair: the seed writes 0x10 and 0x100
 * there and this body retires both over the same 0x10 frames. The draw comes
 * from the in-payload helper 0x801D1B10, whose own body arms GetTPage and
 * SetDrawMode and walks the record halfword 0x14 up to 0x1000 in 0x80 steps.
 * The only in-image references to 0x801D1A60 are the five jal sites inside
 * func_801D0F10, func_801D0FEC, func_801D106C, func_801D12A0 and func_801D13F4,
 * so no table word and no script blob publishes this address.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the halfword at record +0x14 has to be written before the
 * countdown test. With the test written first and the halfword update after it,
 * the sweep measured 35/44 instructions at the same 176 bytes for the
 * count-local form and 33/45 at 180 bytes for the predecrement form, both first
 * differing at offset 0x0030; this order measured 44/44 at 176 bytes. A second
 * pointer local two bytes into the record is not part of the shape either: with
 * it the compiler kept a third induction register for that pointer plus 0x12,
 * spilled the return flag into $s4 and measured 18/44 at 196 bytes, while
 * addressing the countdown byte at +0x2 and the halfword at +0x14 off the record
 * base lets the compiler derive the +0x2 address itself, which is the $s0 the
 * original rotates its +0x12 accesses through.
 */
u32 stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60(
    void) {
  u8 scratch[0x20];
  u8* record;
  u8 index;
  u32 active;

  active = 0;
  func_801AFE18(scratch);
  record = D_801D39A8;
  index = 0u;
  do {
    if (*record != 0) {
      active = 1;
      FIELD_REF(u16, record, 0x14) -= 0x10;
      if (--FIELD_REF(u8, record, 0x2) == 0) {
        *record = 0;
      }
      func_801D1B10(record);
    }
    record += 0x18;
    index += 1u;
  } while (index < 0x10u);

  return active;
}
