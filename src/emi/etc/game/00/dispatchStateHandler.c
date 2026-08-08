#include "internal.h"

/* @behavior dispatches through the indexed handler table at stateHandlerTable
 * using the s8 state selector at D_801448EA.
 * @source 0x801A7BF0
 */
void dispatchStateHandler(void) {
  stateHandlerTable[(s32)D_801448EA]();
}
