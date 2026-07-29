#include "internal.h"

/* @source 0x800AE014 */
/* @behavior Dispatches the selected local battle handler. */
void func_800AE014(void) {
  BattleSelectionHandler handlers[2];

  barrier();
  handlers[0] = func_800AE06C;
  handlers[1] = func_800AE09C;
  handlers[BATTLE_SCRATCHPAD_PTR[1]]();
}
