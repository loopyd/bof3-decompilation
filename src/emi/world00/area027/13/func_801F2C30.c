#include "internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F3AB4.
 * @source 0x801F2C30
 */
void NO_SIBLING_CALLS func_801F2C30(void) {
  WORLD00_AREA027_HANDLER_TABLE[WORLD00_AREA027_SCRATCH_PTR[1]]();
}
