#include "internal.h"

/* @behavior conditionally runs the current enemy scratch reset helper when global
 * bit `0x4` is set.
 * @source 0x801E4F34
 */
void battle03_reset_enemy_scratch_when_bit4(void) {
  if ((BATTLE_GLOBAL_HALF_62E8 & 4u) == 0u) {
    return;
  }

  battle03_reset_enemy_scratch_state();
}
