#include "bof3/world/area00813_internal.h"

/* @behavior Empty body: returns to the caller immediately without reading or
 *           writing any state; it is the no-op entry selected for area-state
 *           mode 9 through the 15-entry handler table at 0x801F46B0 (entry 9,
 *           table word 0x801F46D4).
 * @source 0x801F37B4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F37B4(void) {}
