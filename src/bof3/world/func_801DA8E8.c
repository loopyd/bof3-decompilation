#include "bof3/world/area03004_internal.h"

/* @behavior dispatches the byte at offset 3 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E2258.
 * @source 0x801DA8E8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801DA8E8(void) {
    D_801E2258[D_1F800044[3]]();
}
