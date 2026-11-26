#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3310
 * @behavior starts the selected master's +0x10 action through updateEntrySelectPanel,
 *           passing a nonzero second argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startEntryReleasePanel(void) {
  updateEntrySelectPanel((u16)(masterActionBaseTable[masterIndex] + 0x10), 1);
}
