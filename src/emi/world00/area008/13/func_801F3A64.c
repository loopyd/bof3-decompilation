#include "internal.h"

/* @behavior dispatches the current area-state mode byte through the local
 * handler table at 0x801F46EC.
 * @source 0x801F3A64
 */
void func_801F3A64(void) {
  D_801F46EC[g_areaWork->mode]();
}
