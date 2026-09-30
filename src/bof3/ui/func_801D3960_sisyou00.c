#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3960
 * @behavior starts the selected master's +5 action through func_80150224,
 *           writes the phase byte at 0x80143BB0 as 2, clears the
 *           0x801D428A step byte, then advances the handler index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3960(void) {
  func_80150224((s16)(masterActionBaseTable[masterIndex] + 5));
  D_80143BB0 = 2;
  D_801D428A = 0;
  D_801D4286 = D_801D4286 + 1;
}
