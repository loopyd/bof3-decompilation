#include "bof3/world/area02414_internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F4200.
 * @source 0x801F2C14
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801F2C14(void) {
  D_801F4200[WORLD00_AREA024_SCRATCH_PTR->mode]();
}
