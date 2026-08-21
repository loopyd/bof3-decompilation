#include "bof3/world/area02713_internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F3AB4.
 * @source 0x801F2C30
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchScratchMode1(void) {
  handlerTable[WORLD00_AREA027_SCRATCH_PTR[1]]();
}
