#include "bof3/bof3.h"

/* @source 0x801F336C
 * @behavior Overlay step handler: loads the scratch work record cursor from
 *           scratchpad pointer-slot 0x1F800044, writes the halfword 0x100 to
 *           the record's +0x2E field, 0 to +0x30 and 0x18 to +0x32, then
 *           increments the record's +0x01 step byte; takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfwordsIncrementWorkByte1World02Area08213_801F336C(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u16*)(work + 0x2E) = 0x100;
  *(u16*)(work + 0x30) = 0;
  *(u16*)(work + 0x32) = 0x18;
  work[1]++;
}
