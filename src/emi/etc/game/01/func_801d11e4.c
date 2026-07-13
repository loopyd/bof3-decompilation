#include "internal.h"

/* @behavior clears the prompt gate once the EXE reaches its idle selection
 * state, then draws the active frontend prompt and its selection marker.
 * @source 0x801d11e4 FUN_801d11e4
 */
void func_801d11e4(void) {
  if (DAT_80143bb0 == 5u && DAT_80143b90 == 2u &&
      GAME_FRONT_EFFECT_BUSY == 0u) {
    DAT_80143c30 = 0u;
  }

  if (DAT_8014832e != 0u && DAT_80143c30 != 0u) {
    func_8017c2d8(DAT_8014598c, 0, 0,
                  func_8017b2b4() == 1
                      ? 557
                      : (func_8017b2b4() == 2 ? 557 : 157),
                  0);
    func_8014e5a0(2, 12);
    func_801d17d8(192, 4, 10, 2, 0);
  }
}
