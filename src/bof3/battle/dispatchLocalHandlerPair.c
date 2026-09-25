#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE014
 * @behavior Dispatches the selected local battle handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchLocalHandlerPair(void) {
  BattleSelectionHandler handlers[2] = { setWorkByte9Advance, func_800AE09C };

  handlers[BATTLE_SCRATCHPAD_PTR[1]]();
}
