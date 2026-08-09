#include "bof3/world/area00813_internal.h"

/* @source 0x801F45C8
 * @behavior clears the state halfword and sets the adjacent state byte to 2
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void resetCounter2(void) {
  counter2 = 0;
  D_80149333 = 2;
}
