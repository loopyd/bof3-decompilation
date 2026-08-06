#include "internal.h"

/* @source 0x800AE014 */
/* @behavior Dispatches the selected local battle handler. */
void battle15_dispatch_local_handler_pair(void) {
  BattleSelectionHandler handlers[2];

  barrier();
  handlers[0] = battle15_set_work_byte9_advance;
  handlers[1] = func_800AE09C;
  handlers[BATTLE_SCRATCHPAD_PTR[1]]();
}
