#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D0C18
 * @behavior clears the five shared mode and handler bytes at 0x801D4285
 *           through 0x801D4289, each through its own fixed-address byte
 *           symbol; the region at 0x801D0C00 it follows is the EMI-local
 *           header data (the 0x1F count word and the three numeric-row
 *           sprintf format strings).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D0C18(void) {
  modeIndex = 0;
  D_801D4286 = 0;
  D_801D4287 = 0;
  D_801D4288 = 0;
  D_801D4289 = 0;
}
