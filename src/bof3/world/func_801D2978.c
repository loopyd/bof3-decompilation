#include "bof3/world/area03004_internal.h"

/* @behavior dispatches the scratch cursor mode byte at offset 1 through the
 * third local handler table at 0x801E1D78.
 * @source 0x801D2978
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801D2978(void) {
  D_801E1D78[D_1F800044[1]]();
}
