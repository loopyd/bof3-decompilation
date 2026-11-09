#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. The original
 * re-reads the cell once per access group on either side of the cue call, so it
 * is named through this target's map instead of a fixed-address accessor: the
 * fixed-address spelling keeps the cell address live across the call in a
 * saved register, while the named cell is rematerialised. */
extern u8* D_1F800044;

/* Front-end phase byte of the main executable, outside this overlay's payload;
 * Splat binds it through this target's generated undefined-syms map. The exact
 * area165 lift of the same cue gate reads it as the front-end phase, while the
 * plchar lifts leave its role open, so the symbol stays address-anchored. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the main executable, outside this overlay's payload. */
void func_8015DF18(u16 cue);

/* Record draw routine of this overlay, still an `asm` span of this target at
 * 0x801F40F0; it re-reads the same work record and builds the sprite primitive. */
void drawRouteAxisSplitTexturedQuadPairAtWorkCellScaledByHalfword30World04Area17013_801F40F0(
    void);

/* @source 0x801F4080
 * @behavior Byte-2 state handler, entry 4 of the five-entry handler table based
 *           at 0x801F588C, the table the overlay dispatcher at 0x801F3BE8
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044. Subtracts 0x10
 *           from the record halfword at offset 0x30, stores the truncated
 *           halfword and tests the stored value as a signed halfword: while that
 *           value stays above zero the handler only draws. Once the countdown
 *           reaches zero or below, the front-end phase byte D_80143BB0 gates the
 *           sound cue 0x201 through func_8015DF18, which fires only while that
 *           byte is zero, and the record's state byte at offset 0x02 is set to 1
 *           so the next dispatch takes entry 1 at 0x801F3DE8 of the same table.
 *           The draw routine at 0x801F40F0 runs on every call, including the
 *           frame that expires the countdown, and re-reads the same record. The
 *           pointer cell D_1F800044 is loaded before the frame is created and is
 *           re-read after the cue gate, both times as `lui` plus the offset load
 *           into $v1. Takes no arguments and returns nothing; the 0x18-byte frame
 *           only keeps $ra at 0x10, and the slots after the halfword load and
 *           after each branch stay nops.
 *
 * Name evidence: the identical three-statement body lifted exactly in
 * emi/world04/area165/13 at 0x801F3518 is published as
 * decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenAdvanceWorkByte2AndDraw,
 * which fixes the role of the 0x30 halfword countdown, the 0x201 cue and the
 * D_80143BB0 phase gate; only the state action differs here (this handler stores
 * 1 instead of advancing the byte), so that clause follows the
 * `...AndSetWorkByte2To1...` spelling of the exact world03/area118 handler at
 * 0x801F36B4. The tail call into 0x801F40F0 keeps the family's `...ThenDraw`
 * ending used by both exact neighbours 0x801F3F28 and 0x801F3518.
 *
 * Shape evidence: state entry 2 at 0x801F3F28 and entry 3 at 0x801F3F84 of the
 * same handler table are exact lifts of this target and fix the record accessor,
 * the halfword read-modify-write and the empty tail-call delay slot. The
 * area165 copy at 0x801F3518 is a 0x7C-byte span, not the 0x70 bytes here: it
 * re-reads and increments the state byte (lbu, addiu 1, sb) where this handler
 * stores a materialised 1 (addiu $v0, $zero, 0x1 then sb), and its tail call
 * targets its own 0x801F30D4. Relocation- and value-different, so it is shape
 * evidence only and never clone eligibility.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenSetWorkByte2To1AndDrawWorld04Area17013_801F4080(
    void) {
  u8* work;
  u16 half;

  work = D_1F800044;
  half = *(u16*)(work + 0x30) - 0x10;
  *(u16*)(work + 0x30) = half;
  if ((s16)half <= 0) {
    if (D_80143BB0 == 0) {
      func_8015DF18(0x201);
    }
    work = D_1F800044;
    work[2] = 1;
  }
  drawRouteAxisSplitTexturedQuadPairAtWorkCellScaledByHalfword30World04Area17013_801F40F0();
}
