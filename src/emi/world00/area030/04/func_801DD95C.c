#include "internal.h"

/* @behavior dispatches the byte at offset 3 of the scratch work-record
 * cursor (0x1F800044) through the local handler table at 0x801E22FC.
 * @source 0x801DD95C
 */
void NO_SIBLING_CALLS func_801DD95C(void) {
    D_801E22FC[D_1F800044[3]]();
}
