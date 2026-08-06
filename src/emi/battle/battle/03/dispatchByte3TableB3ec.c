#include "internal.h"

/* @source 0x801E32F0
 * @behavior dispatches through D_801EB3EC using byte 3 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 */
void dispatchByte3TableB3ec(void) {
  D_801EB3EC[battleWork[3]]();
}
