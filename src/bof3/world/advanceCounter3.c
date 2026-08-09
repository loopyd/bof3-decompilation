#include "bof3/world/area00813_internal.h"

/* @source 0x801F45E4
 * @behavior increments the area counter by 0x800
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advanceCounter3(void) {
  s32 *counter;
  s32 value;

  counter = &counter3;
  value = *counter;
  value += 0x800;
  *counter = value;
}
