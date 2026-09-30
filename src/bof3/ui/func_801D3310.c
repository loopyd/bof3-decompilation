#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3310
 * @behavior starts the selected master's +0x10 action through func_801D0DD4,
 *           passing a nonzero second argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3310(void) {
  func_801D0DD4((u16)(masterActionBaseTable[masterIndex] + 0x10), 1);
}
