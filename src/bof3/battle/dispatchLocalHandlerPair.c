#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE014
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
/* @behavior Dispatches the selected local battle handler. */
void dispatchLocalHandlerPair(void) {
  BattleSelectionHandler handlers[2];

  handlers[0] = setWorkByte9Advance;
  handlers[1] = func_800AE09C;
  handlers[BATTLE_SCRATCHPAD_PTR[1]]();
}
