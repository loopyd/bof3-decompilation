#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D2B64
 * @behavior when the phase byte D_80143BB0 is not 2, starts the selected
 *           master's +4 action through func_80150224, writes the phase byte as
 *           2, sets the mode index to 6, and clears the handler index
 *           D_801D4286.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D2B64(void) {
  u8* phase = &D_80143BB0;

  if (*phase == 2) {
    return;
  }
  func_80150224((s16)(masterActionBaseTable[masterIndex] + 4));
  *phase = 2;
  modeIndex = 6;
  D_801D4286 = 0;
}
