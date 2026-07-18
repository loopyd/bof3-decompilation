#include "internal.h"

/* @behavior returns the overlay to state zero when the EXE gate closes;
 * otherwise accepts enabled pad input once loading/effects are idle, starts
 * the selected cue, advances state, and updates the frontend prompt.
 * @source 0x801D0F00
 */
void func_801D0F00(void) {
  if (D_80143B40 == 0u) {
    GAME_FRONT_STATE = 0u;
    return;
  }

  if (func_80162D00() && GAME_FRONT_EFFECT_BUSY == 0u &&
      (GAME_FRONT_PAD_STATE & 0x09ffu) != 0u) {
    D_80146874 = 1u;
    func_8014ECAC(4);
    func_80161CD0(GAME_FRONT_SELECTION, D_80143F20, 8);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
  func_801D11E4();
}
