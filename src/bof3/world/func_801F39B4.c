#include "bof3/world/area00813_internal.h"

/* @behavior Empty body: returns to the caller immediately without reading or
 *           writing any state; it is the no-op entry selected for area-state
 *           mode 13 through the 15-entry handler table at 0x801F46B0 (entry 13,
 *           table word 0x801F46E4).
 * @source 0x801F39B4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F39B4(void) {}
