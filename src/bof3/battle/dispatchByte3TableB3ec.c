#include "bof3/battle/battle03_internal.h"

/* @source 0x801E32F0
 * @behavior dispatches through D_801EB3EC using byte 3 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchByte3TableB3ec(void) {
  D_801EB3EC[battleWork[3]]();
}
