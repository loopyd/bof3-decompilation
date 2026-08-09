#include "bof3/battle/battle03_internal.h"

/* @behavior initializes the three-byte mode tuple at `0x801462e0` for one specific
 * queued branch.
 * @source 0x801DDF28
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initModeTuple530(void) {
  s32 new_var3;
  u32 new_var2;
  s32 new_var;

  new_var3 = 2;
  new_var = 0;
  BATTLE_MODE_TUPLE_62E0[new_var] = 5u;
  new_var2 = 1;
  BATTLE_MODE_TUPLE_62E0[new_var2] = 3u;
  BATTLE_MODE_TUPLE_62E0[new_var3] = 0u;
}
