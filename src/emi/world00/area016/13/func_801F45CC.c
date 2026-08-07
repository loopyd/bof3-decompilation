#include "internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F51D0.
 * @source 0x801F45CC
 */
void NO_SIBLING_CALLS func_801F45CC(void) {
  WORLD00_AREA016_D_801F51D0[WORLD00_AREA016_SCRATCH_PTR->mode]();
}
