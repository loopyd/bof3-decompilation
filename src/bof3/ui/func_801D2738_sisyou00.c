#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D2738
 * @behavior when the phase byte D_80143BB0 is not 2, passes D_8014554F and
 *           masterIndex to the EXE-side flag helper func_8015B580, sets
 *           modeIndex to 3, then clears the handler index D_801D4286.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D2738(void) {
  if (D_80143BB0 == 2) {
    return;
  }
  func_8015B580(&D_8014554F, masterIndex);
  modeIndex = 3;
  D_801D4286 = 0;
}
