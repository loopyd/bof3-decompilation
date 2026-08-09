#include "bof3/world/area03213_internal.h"

/* @behavior dispatches the scratch work byte at offset 4 of the scratchpad
 * cursor record through the local handler table at 0x801F4908.
 * @source 0x801F3800
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801F3800(void) {
  D_801F4908[D_1F800044[4]]();
}
