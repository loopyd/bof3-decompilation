#include "bof3/world/area02713_internal.h"

/* @behavior dispatches the scratch mode byte at offset 2 through the local
 * handler table at 0x801F3ABC.
 * @source 0x801F3120
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801F3120(void) {
  D_801F3ABC[WORLD00_AREA027_SCRATCH_PTR[2]]();
}
