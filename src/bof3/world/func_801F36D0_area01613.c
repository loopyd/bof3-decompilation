#include "bof3/world/area01613_internal.h"

/* @source 0x801F36D0
 * @behavior Resets the scratch height at 0x30 to 0xF0 and advances the local
 * state byte at 0x03 when the shared mode bytes permit it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F36D0(void) {
  World00Area016Scratch* scratch;

  if (D_801454F2 != 0 && WORLD00_AREA016_SCRATCH_PTR->unk_0b != 0) {
    return;
  }
  if (D_80143BB0 == 2) {
    return;
  }
  if ((D_80146258 & 0x100u) != 0) {
    return;
  }
  scratch = WORLD00_AREA016_SCRATCH_PTR;
  scratch->field_30 = 0xF0;
  scratch->state_03++;
}
