#include "internal.h"

/* @behavior returns the overlay to state zero when the EXE gate closes;
 * otherwise accepts enabled pad input once loading/effects are idle, starts
 * the selected cue, advances state, and updates the frontend prompt.
 * @source 0x801d0f00 FUN_801d0f00
 */
void func_801d0f00(void) {
  if (DAT_80143b40 == 0u) {
    GAME_FRONT_STATE = 0u;
    return;
  }

  if (func_80162d00() && GAME_FRONT_EFFECT_BUSY == 0u &&
      (GAME_FRONT_PAD_STATE & 0x09ffu) != 0u) {
    DAT_80146874 = 1u;
    func_8014ecac(4);
    func_80161cd0(GAME_FRONT_SELECTION, DAT_80143f20, 8);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
  func_801d11e4();
}
