#include "bof3/world/area00813_internal.h"

/* @behavior sets state byte 2 and decrements the 16-bit count by 0x14.
 * @source 0x801F4534
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void retreatCounter1(void) {
  u16 count;

  count = counter1;
  D_80149333 = 2;
  counter1 = (u16)(count - 0x14);
}
