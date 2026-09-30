#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D2C24
 * @behavior starts the selected master's +0xF action through func_801D0DD4,
 *           passing a zero second argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D2C24(void) {
  func_801D0DD4((u16)(masterActionBaseTable[masterIndex] + 0xF), 0);
}
