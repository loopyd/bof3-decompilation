#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets D_801485BB, D_801485DF, and initializes counters at D_801462E3/E4.
 * @source 0x800A6870
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800A6870(void) {
  D_801485BB = 4;
  D_801485DF = 2;
  D_801462E3 = 3;
  D_801462E4 = 2;
}
