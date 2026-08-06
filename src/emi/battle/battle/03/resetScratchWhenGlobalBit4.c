#include "internal.h"

/* @behavior conditionally runs the current scratch-object reset helper when global
 * bit `0x4` is set.
 * @source 0x801E1B2C
 */
void resetScratchWhenGlobalBit4(void) {
  if ((BATTLE_GLOBAL_HALF_62E8 & 4u) == 0u) {
    return;
  }

  func_801E1DD4();
}
