#include "internal.h"

/* @behavior clears the prompt gate once the EXE reaches its idle selection
 * state, then draws the active frontend prompt and its selection marker.
 * @source 0x801D11E4
 */
void func_801D11E4(void) {
  if (D_80143BB0 == 5u && D_80143B90 == 2u && GAME_FRONT_EFFECT_BUSY == 0u) {
    D_80143C30 = 0u;
  }

  if (D_8014832E != 0u && D_80143C30 != 0u) {
    SetDrawMode(
        (DR_MODE*)D_8014598C, 0, 0,
        GetGraphType() == 1 ? 557 : (GetGraphType() == 2 ? 557 : 157), 0);
    func_8014E5A0(2, 12);
    func_801D17D8(192, 4, 10, 2, 0);
  }
}
