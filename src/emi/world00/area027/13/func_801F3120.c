#include "internal.h"

/* @behavior dispatches the scratch mode byte at offset 2 through the local
 * handler table at 0x801F3ABC.
 * @source 0x801F3120
 */
void NO_SIBLING_CALLS func_801F3120(void) {
  D_801F3ABC[WORLD00_AREA027_SCRATCH_PTR[2]]();
}
