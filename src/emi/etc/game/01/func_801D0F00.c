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
    u8  sel;
    u16 bank;
    D_80146874 = 1u;
    /* MATCHING_AID: keep a0=4 in the jal delay slot */
    CLOBBER_CALLER_REG(a0);
    func_8014ECAC(4);
    /* MATCHING_AID: stage the two arguments through ordered temporaries, then
     * barrier() before the call. This makes cc1 emit lbu a0 (GAME_FRONT_SELECTION)
     * then lhu a1 (D_80143F20) immediately before the jal and defer li a2,8 into
     * the jal delay slot, reproducing the original schedule. A direct call lets
     * cc1 hoist li a2,8 ahead of the loads, load a1 before a0, and leave a nop in
     * the delay slot. Removable if the call is ever compiled in a context whose
     * scheduler already defers the constant third argument. */
    sel = GAME_FRONT_SELECTION;
    bank = D_80143F20;
    barrier();
    func_80161CD0(sel, bank, 8);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
  func_801D11E4();
}
