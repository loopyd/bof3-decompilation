#include "bof3/world/area01613_internal.h"

/* @behavior seeds the local scratchpad halfwords at `0x2e` and `0x30` with the
 * fixed area defaults.
 * @source 0x801F3400
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void seedScratchDefaults(void) {
  World00Area016Scratch* scratch;

  scratch = WORLD00_AREA016_SCRATCH_PTR;
  scratch->field_2e = 0xa0;
  scratch->field_30 = 0x50;
}
