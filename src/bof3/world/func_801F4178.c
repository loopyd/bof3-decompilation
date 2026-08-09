#include "bof3/world/area01613_internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F51AC.
 * @source 0x801F4178
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801F4178(void) {
  D_801F51AC[WORLD00_AREA016_SCRATCH_PTR->mode]();
}
