#include "bof3/world/area01613_internal.h"

/**
 * @source 0x801F350C
 * @behavior Advances the scratch state when the global mode and input flag permit it.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F350C(void) {
  World00Area016Scratch* scratch;

  if (D_801454F2 == 2) {
    return;
  }
  if ((D_80146258 & 0x100u) != 0) {
    return;
  }
  scratch = WORLD00_AREA016_SCRATCH_PTR;
  scratch->field_2e = -0x30;
  scratch->state_02++;
}
