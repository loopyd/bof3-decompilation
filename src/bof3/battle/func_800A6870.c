#include "bof3/battle/battle15_internal.h"

/* @behavior Stores 4 into byte D_801485BB, 2 into byte D_801485DF, 3 into
 * byte D_801462E3 and 2 into byte D_801462E4.
 * @source 0x800A6870
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A6870(void) {
  D_801485BB = 4;
  D_801485DF = 2;
  D_801462E3 = 3;
  D_801462E4 = 2;
}
