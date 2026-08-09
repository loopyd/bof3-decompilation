#include "bof3/world/area01613_internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F5114.
 * @source 0x801F341C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801F341C(void) {
  D_801F5114[WORLD00_AREA016_SCRATCH_PTR->mode]();
}
