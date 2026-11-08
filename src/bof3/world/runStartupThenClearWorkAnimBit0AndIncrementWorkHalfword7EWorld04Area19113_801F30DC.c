#include "bof3/bof3.h"

extern void* D_80146884; /* @source 0x80146884 @kind data */

void func_8015C088(void);

/* @source 0x801F30DC
 * @behavior Runs the shared front-end startup helper 0x8015C088, then clears
 * bit 0 of the byte at offset 0x74 of the work record published at 0x80146884
 * with an `andi 0xFE` read-modify-write, then increments that record's unsigned
 * halfword at offset 0x7E in place. The pointer cell 0x80146884 is re-read after
 * the byte store, so the increment goes through the record published then
 * rather than the one whose bit 0 was just cleared. Takes no arguments and
 * returns nothing; the 0x18-byte frame keeps only $ra because the record
 * pointer is never held across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runStartupThenClearWorkAnimBit0AndIncrementWorkHalfword7EWorld04Area19113_801F30DC(
    void) {
  u8* record;

  func_8015C088();
  record = D_80146884;
  record[0x74] &= ~1u;
  record = D_80146884;
  *(u16*)(record + 0x7E) += 1;
}
