#include "bof3/world/area02613_internal.h"

/* @behavior dispatches the scratch mode byte at offset 1 through the local
 * handler table at 0x801F33EC.
 * @source 0x801F2C04
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS dispatchArea026ScratchMode(void) {
  D_801F33EC[D_1F800044[1]]();
}
