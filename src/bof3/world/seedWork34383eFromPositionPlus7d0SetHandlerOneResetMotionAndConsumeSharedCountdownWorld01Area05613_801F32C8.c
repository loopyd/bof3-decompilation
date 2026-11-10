#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044 and the shared record
 * published through the global pointer cell 0x80146250. Both are bound through
 * the target symbol table: each read expands to the split-address form
 * `lui %hi` plus a folded `%lo` displacement. A literal cell address instead
 * makes one constant-address pseudo that the compiler hoists into a
 * call-saved register across the resident position call, which costs $s0 and
 * the wider frame. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind unknown */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* Resident shared position routine outside this payload. */
s32 func_80154F28(s32 x, s32 y);

/* @source 0x801F32C8
 * @behavior First handler of this overlay's work-byte-4 table at 0x801F3898.
 *           It clears bit 0x40 of the work record's flag byte at 0x00, seeds
 *           the record halfword at 0x3E from the shared position routine
 *           0x80154F28 over the record coordinate words at 0x34 and 0x38 plus
 *           0x7D0, then hands the record to the next handler by storing 1 in
 *           the handler byte at 0x04. It resets the record's motion words at
 *           0x0C, 0x10 and 0x14 to zero and its step word at 0x20 to -8, and
 *           finally consumes two units of the halfword at 0x122 of the record
 *           published through the global pointer cell 0x80146250. Takes no
 *           arguments and returns nothing; the value left in $v0 is the
 *           consumed countdown, which no caller reads.
 *
 *           The work pointer cell is read three times in the original: once
 *           for the flag byte, once for the two coordinate words the position
 *           routine takes (both folded onto one base register because the
 *           second read serves both arguments), and again after the call for
 *           the whole store group, which shares the single base register $v1
 *           across the halfword store at 0x3E, the handler byte at 0x04 and
 *           the four word stores. The reload after the call is not redundant:
 *           the callee may republish the cell, so the store group reads it a
 *           third time.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWork34383eFromPositionPlus7d0SetHandlerOneResetMotionAndConsumeSharedCountdownWorld01Area05613_801F32C8(
    void) {
  u8* work;
  u8* shared;
  s32 layer;

  D_1F800044[0] &= 0xBF;
  layer = func_80154F28(FIELD_REF(s32, D_1F800044, 0x34),
                        FIELD_REF(s32, D_1F800044, 0x38));
  work = D_1F800044;
  FIELD_REF(u16, work, 0x3E) = layer + 0x7D0;
  work[4] = 1;
  shared = D_80146250;
  FIELD_REF(s32, work, 0x0C) = 0;
  FIELD_REF(s32, work, 0x10) = 0;
  FIELD_REF(s32, work, 0x14) = 0;
  FIELD_REF(s32, work, 0x20) = -8;
  FIELD_REF(u16, shared, 0x122) -= 2;
}
