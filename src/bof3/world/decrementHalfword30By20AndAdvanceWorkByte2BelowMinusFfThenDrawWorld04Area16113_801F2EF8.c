#include "bof3/bof3.h"

void func_801F30D4(void);

/* @source 0x801F2EF8
 * @behavior Byte-2 state handler entry 2 of the five-entry handler table based
 *           at 0x801F3424, the table the overlay dispatcher at 0x801F2C04
 *           indexes by the state byte of the work record published at the
 *           scratchpad pointer cell 0x1F800044. Subtracts 0x20 from the
 *           record unsigned halfword at offset 0x30, stores the truncated
 *           halfword, and, once that stored value read as a signed halfword
 *           falls below -0xFF, increments the record state byte at offset 0x02
 *           so the next dispatch takes entry 3 at 0x801F2F54. It then calls the
 *           overlay routine at 0x801F30D4, which reads the same halfword as a
 *           sprite screen offset. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps $ra at 0x10 across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By20AndAdvanceWorkByte2BelowMinusFfThenDrawWorld04Area16113_801F2EF8(
    void) {
  u8* work;
  u16 half;

  work = SPAD_PTR_SLOT(u8, 0x44);
  half = *(u16*)(work + 0x30) - 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half < -0xFF) {
    work[2]++;
  }
  func_801F30D4();
}
