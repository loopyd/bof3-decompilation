#include "internal.h"

/* @behavior dispatches the byte at offset 3 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E22BC.
 * @source 0x801DC64C
 */
void NO_SIBLING_CALLS func_801DC64C(void) {
    D_801E22BC[D_1F800044[3]]();
}
