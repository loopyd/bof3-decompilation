#include "internal.h"

/* @behavior dispatches the byte at offset 3 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E221C.
 * @source 0x801D9CF4
 */
void NO_SIBLING_CALLS func_801D9CF4(void) {
    D_801E221C[D_1F800044[3]]();
}
