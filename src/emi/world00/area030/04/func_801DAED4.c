#include "internal.h"

/* @behavior dispatches the byte at offset 3 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E2268.
 * @source 0x801DAED4
 */
void NO_SIBLING_CALLS func_801DAED4(void) {
    D_801E2268[D_1F800044[3]]();
}
