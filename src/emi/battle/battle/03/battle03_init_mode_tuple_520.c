#include "internal.h"

/* @source 0x801DDF00
 * @behavior initializes the battle state to substate 5,2,0.
 */
void battle03_init_mode_tuple_520(void) {
  D_801462E0 = 5;
  D_801462E1[0] = 2;
  BATTLE_GLOBAL_BYTE_62E2 = 0;
}
