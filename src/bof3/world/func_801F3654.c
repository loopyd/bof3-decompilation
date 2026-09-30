#include "bof3/world/area00813_internal.h"

/* @behavior Empty body: returns to the caller immediately without reading or
 *           writing any state; it is the no-op entry selected for area-state
 *           mode 6 through the 15-entry handler table at 0x801F46B0 (entry 6,
 *           table word 0x801F46C8).
 * @source 0x801F3654
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3654(void) {}
