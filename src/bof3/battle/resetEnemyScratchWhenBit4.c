#include "bof3/battle/battle03_internal.h"

/* @behavior conditionally runs the current enemy scratch reset helper when global
 * bit `0x4` is set.
 * @source 0x801E4F34
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void resetEnemyScratchWhenBit4(void) {
  if ((BATTLE_GLOBAL_HALF_62E8 & 4u) == 0u) {
    return;
  }

  resetEnemyScratchState();
}
