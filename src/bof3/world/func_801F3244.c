#include "bof3/world/area00813_internal.h"

/* @behavior dispatches the current area-state mode byte through the local
 * handler table at 0x801F46B0.
 * @source 0x801F3244
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801F3244(void) {
  areaStateModeHandlerTable[g_areaWork->mode]();
}
