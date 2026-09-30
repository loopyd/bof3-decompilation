#include "bof3/world/area03004_internal.h"

/* @behavior dispatches the AREA030 work-record stage byte at scratch offset 3
 * of the record cursor 0x1F800044 through the overlay stage-handler table at
 * 0x801E1D00, whose three code-pointer entries are the consecutive handlers
 * 0x801D130C, 0x801D1434 and 0x801D1538; the call takes no arguments and the
 * handler result is discarded.
 * @source 0x801D12C8
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801D12C8(void) {
    D_801E1D00[D_1F800044[3]]();
}
