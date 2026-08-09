#include "bof3/world/area03213_internal.h"

/* @behavior dispatches the scratch work byte at offset 1 of the scratchpad
 * cursor record through the local handler table at 0x801F3F80.
 * @source 0x801F30A8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801F30A8(void) {
  D_801F3F80[D_1F800044[1]]();
}
