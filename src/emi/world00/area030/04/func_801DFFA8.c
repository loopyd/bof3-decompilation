#include "internal.h"

/* @behavior dispatches the byte at offset 2 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E2340.
 * @source 0x801DFFA8
 */
void NO_SIBLING_CALLS func_801DFFA8(void) {
    D_801E2340[D_1F800044[2]]();
}
