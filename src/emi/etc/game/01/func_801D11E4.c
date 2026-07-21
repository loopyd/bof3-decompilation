#include "internal.h"

/* @behavior clears the prompt gate once the EXE reaches its idle selection
 * state, then draws the active frontend prompt and its selection marker.
 * @source 0x801D11E4
 */
void func_801D11E4(void) {
  /* MATCHING_AID: read the gate/mode bytes through a non-volatile view.
   * cc1 (gcc-2.7.2-psx) emits an explicit zero-extension (andi 0xff after
   * lbu, andi 0xffff after lhu) whenever a *volatile* narrow load feeds a
   * non-zero comparison; the original binary has no such mask (lbu/li/bne
   * directly, load in $v1, constant in $v0). Casting away volatile lets cc1
   * absorb the zero-extension into the lbu/lhu and reproduces that register
   * allocation. GAME_FRONT_EFFECT_BUSY stays volatile: its == 0u compare is a
   * bnez that never triggers the mask. Remove if these symbols are ever
   * re-declared non-volatile. */
  if (*(u8*)&D_80143BB0 == 5u && *(u16*)&D_80143B90 == 2u &&
      GAME_FRONT_EFFECT_BUSY == 0u) {
    D_80143C30 = 0u;
  }

  if (D_8014832E != 0u && D_80143C30 != 0u) {
    SetDrawMode((DR_MODE*)D_8014598C, 0, 0,
                GetGraphType() == 1 ? 557 : (GetGraphType() == 2 ? 557 : 157),
                0);
    func_8014E5A0(2, 12);
    func_801D17D8(192, 4, 10, 2, 0);
  }
}
