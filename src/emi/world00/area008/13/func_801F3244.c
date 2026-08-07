#include "internal.h"

/* @behavior dispatches the current area-state mode byte through the local
 * handler table at 0x801F46B0.
 * @source 0x801F3244
 */
void NO_SIBLING_CALLS func_801F3244(void) {
  D_801F46B0[g_areaWork->mode]();
}
