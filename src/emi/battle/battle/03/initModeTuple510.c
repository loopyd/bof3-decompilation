#include "internal.h"

/* @source 0x801DDED8
 * @behavior initializes three battle-global state bytes to 5, 1, and 0.
 */
void initModeTuple510(void) {
  D_801462E0 = 5;
  D_801462E1[0] = 1;
  BATTLE_GLOBAL_BYTE_62E2 = 0;
}
