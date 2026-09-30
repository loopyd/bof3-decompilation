#include "bof3/battle/battle03_internal.h"

/* @behavior copies the visible state of battlers 0, 1 and 2 into their templates,
 * then runs one shared front-end frame slice and starts EMI stream slot 0x256
 * before advancing the battle state byte that selects the next handler of the
 * table entered by dispatchStateTableAd20.
 * @source 0x801D71E0
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The state byte is read and stored through a named non-volatile view: the
 * original keeps its address in one register for both accesses and schedules the
 * frame restore above the increment store, which a volatile pointee forbids. */
void snapshotBattlersAndAdvanceState(void) {
  u8* state;

  copyCurrentBattlerVisibleStateToTemplate(0u);
  copyCurrentBattlerVisibleStateToTemplate(1u);
  copyCurrentBattlerVisibleStateToTemplate(2u);
  func_80158E50();
  func_80161FDC(0x256u);
  state = (u8*)&BATTLE_GLOBAL_BYTE_62E2;
  *state += 1u;
}
