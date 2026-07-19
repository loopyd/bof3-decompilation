#include "internal.h"

/* @behavior conditionally runs the current enemy scratch reset helper when global
 * bit `0x4` is set.
 * @source 0x801E4F34
 */
void func_801E4F34(void) {
  if ((BATTLE_GLOBAL_HALF_62E8 & 4u) == 0u) {
    return;
  }

  func_801E567C();
}
