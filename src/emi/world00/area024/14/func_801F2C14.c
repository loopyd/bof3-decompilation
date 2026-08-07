#include "internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F4200.
 * @source 0x801F2C14
 */
void NO_SIBLING_CALLS func_801F2C14(void) {
  WORLD00_AREA024_D_801F4200[WORLD00_AREA024_SCRATCH_PTR->mode]();
}
