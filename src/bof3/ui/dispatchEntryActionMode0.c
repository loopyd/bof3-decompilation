#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D2C64
 * @behavior dispatches the selected entry's +0x11 action.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchEntryActionMode0(void) {
  func_801D10AC((u16)(masterActionBaseTable[masterIndex] + 0x11));
}
