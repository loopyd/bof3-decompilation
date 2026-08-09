#include "bof3/battle/battle03_internal.h"

/* @source 0x801E32AC
 * @behavior dispatches through D_801EB3E4 using byte 2 of the object addressed
 * by the non-volatile scratchpad pointer cell battleWork.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchByte2TableB3e4(void) {
  D_801EB3E4[battleWork[2]]();
}
