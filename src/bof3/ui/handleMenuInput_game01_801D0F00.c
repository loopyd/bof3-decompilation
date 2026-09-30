#include "bof3/ui/game01_internal.h"

/* @behavior returns the overlay to state zero when the EXE gate closes;
 * otherwise accepts enabled pad input once loading/effects are idle, starts
 * the selected cue, advances state, and updates the frontend prompt.
 * @source 0x801D0F00
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleMenuInput(void) {
  if (D_80143B40 == 0u) {
    GAME_FRONT_STATE = 0u;
    return;
  }

  if (func_80162D00() && GAME_FRONT_EFFECT_BUSY == 0u &&
      (GAME_FRONT_PAD_STATE & 0x09ffu) != 0u) {
    u8  sel;
    u16 bank;
    /* Non-volatile views of the cue-flag store and the bank read: the plain
     * volatile accesses let cc1 hoist `li a0,4` / `li a2,8` away from the two
     * jal delay slots and reverse the lbu/lhu argument loads. */
    *(u8*)&D_80146874 = 1u;

    func_8014ECAC(4);

    sel = GAME_FRONT_SELECTION;
    bank = *(u16*)&D_80143F20;
    func_80161CD0(sel, bank, 8);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
  drawPrompt();
}
