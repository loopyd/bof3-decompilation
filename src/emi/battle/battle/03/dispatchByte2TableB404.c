#include "internal.h"

/* @source 0x801E3A00
 * @behavior dispatches through D_801EB404 using byte 2 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 */
void dispatchByte2TableB404(void) {
  D_801EB404[battleWork[2]]();
}
