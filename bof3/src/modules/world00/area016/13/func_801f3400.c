#include "internal.h"

/* does: seeds the local scratchpad halfwords at `0x2e` and `0x30` with the
 * fixed area defaults.
 * @source: 0x801f3400 FUN_801f3400
 */
void func_801f3400(void) {
  World00Area016Scratch* scratch;

  scratch = BOF3_WORLD00_AREA016_SCRATCH_PTR;
  scratch->field_2e = 0xa0;
  scratch->field_30 = 0x50;
}
