#include "internal.h"

/* @source 0x801E3438
 * @behavior dispatches through D_801EB3F4 using byte 3 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 */
void dispatchByte3TableB3f4(void) {
  D_801EB3F4[battleWork[3]]();
}
