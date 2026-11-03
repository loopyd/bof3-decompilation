#include "bof3/world/area03004_internal.h"

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * local handler table at 0x801E1D8C.
 * @source 0x801D3358
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801D3358(void) {
  D_801E1D8C[D_1F800044[1]]();
}
