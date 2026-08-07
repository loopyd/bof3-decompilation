#include "internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F51AC.
 * @source 0x801F4178
 */
void NO_SIBLING_CALLS func_801F4178(void) {
  WORLD00_AREA016_D_801F51AC[WORLD00_AREA016_SCRATCH_PTR->mode]();
}
