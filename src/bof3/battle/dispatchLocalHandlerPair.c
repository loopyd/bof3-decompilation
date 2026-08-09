#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE014
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior Dispatches the selected local battle handler. */
void dispatchLocalHandlerPair(void) {
  BattleSelectionHandler handlers[2];

  barrier();
  handlers[0] = setWorkByte9Advance;
  handlers[1] = func_800AE09C;
  handlers[BATTLE_SCRATCHPAD_PTR[1]]();
}
