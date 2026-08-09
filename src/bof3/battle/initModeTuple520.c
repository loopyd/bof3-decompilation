#include "bof3/battle/battle03_internal.h"

/* @source 0x801DDF00
 * @behavior initializes the battle state to substate 5,2,0.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initModeTuple520(void) {
  D_801462E0 = 5;
  D_801462E1[0] = 2;
  BATTLE_GLOBAL_BYTE_62E2 = 0;
}
