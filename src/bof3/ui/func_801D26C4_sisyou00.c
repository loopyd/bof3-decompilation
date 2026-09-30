#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D26C4
 * @behavior passes D_8014554C and masterIndex to the EXE-side flag helper
 *           func_8015B580, starts the selected master's unoffset action
 *           through func_80150224, writes the phase byte D_80143BB0 as 2,
 *           then advances the handler index D_801D4286.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D26C4(void) {
  func_8015B580(&D_8014554C, masterIndex);
  func_80150224((s16)masterActionBaseTable[masterIndex]);
  D_80143BB0 = 2;
  D_801D4286 = D_801D4286 + 1;
}
