#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D2C24
 * @behavior starts the selected master's +0xF action through updateEntrySelectPanel,
 *           passing a zero second argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startEntryClaimPanel(void) {
  updateEntrySelectPanel((u16)(masterActionBaseTable[masterIndex] + 0xF), 0);
}
