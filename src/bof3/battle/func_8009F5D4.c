#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F5D4
 * @behavior Stores 4 into byte D_80146375 and 0x57 into halfword D_801463C0, then
 * calls resetSelectionApplyInput with argument 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F5D4(void) {
  D_80146375 = 4;
  D_801463C0 = 0x57;
  resetSelectionApplyInput(0x10);
}
