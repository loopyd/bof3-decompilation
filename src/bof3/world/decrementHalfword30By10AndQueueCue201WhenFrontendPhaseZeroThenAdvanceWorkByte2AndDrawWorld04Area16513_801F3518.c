#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. The original
 * re-reads the cell once per access group, so it is named through this target's
 * map instead of a fixed-address accessor. */
extern u8* D_1F800044;

/* Front-end phase byte of the main executable, outside this overlay's payload.
 * The cue below only fires while that phase holds zero. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the main executable, outside this overlay's payload. */
void func_8015DF18(u16 cue);

/* Local drawing routine of this overlay (still an `asm` span of this target).
 * It reads the work record's halfword at +0x30 as a sprite screen offset. */
void func_801F30D4(void);

/* @source 0x801F3518
 * @behavior Byte-2 state handler, entry 2 of the four-entry handler table based
 *           at 0x801F35B0, the table the overlay dispatcher at 0x801F3418
 *           indexes by the state byte of the work record published at the
 *           scratchpad pointer cell 0x1F800044. Subtracts 0x10 from the
 *           record halfword at offset 0x30, stores the truncated halfword and
 *           tests the stored value as a signed halfword: while that value stays
 *           above zero the handler only draws. Once the countdown reaches zero
 *           or below, the front-end phase byte D_80143BB0 gates the sound cue
 *           0x201 through func_8015DF18, which fires only while the phase byte
 *           is zero, and the record's step byte at offset 0x02 is advanced by
 *           one so the next dispatch takes entry 3 at 0x801F30D4, this
 *           overlay's draw routine. The draw routine runs on every call,
 *           including the frame that expires the countdown, and reads the same
 *           halfword as a sprite screen offset. The pointer cell is re-read
 *           after the cue gate. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps $ra at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenAdvanceWorkByte2AndDrawWorld04Area16513_801F3518(
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
    work[2]++;
  }
  func_801F30D4();
}
