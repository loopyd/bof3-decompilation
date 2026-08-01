#include "internal.h"

/* @source 0x801DDF00
 * @behavior initializes the battle state to substate 5,2,0.
 */
void func_801DDF00(void) {
  D_801462E0 = 5;
  D_801462E1[0] = 2;
  BATTLE_GLOBAL_BYTE_62E2 = 0;
}
