#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DDFD4
 * @behavior Copies the current state byte to its saved slot, then updates the
 * state according to the masked controller input.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801DDFD4(void)
{
  D_801E31F4 = D_801E31F0;
  if ((D_80145AA8 & 0xE040) != 0) {
    D_801E31F0 = 1;
  } else {
    D_801E31F0 = 0;
  }
}
