#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3714
 * @behavior when the phase byte D_80143BB0 is not 2, starts the selected
 *           master's +0xD action through func_80150224, writes the phase byte
 *           as 2, then advances the handler index D_801D4286.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3714(void) {
  u8* phase = &D_80143BB0;

  if (*phase == 2) {
    return;
  }
  func_80150224((s16)(masterActionBaseTable[masterIndex] + 0xD));
  *phase = 2;
  D_801D4286 = D_801D4286 + 1;
}
