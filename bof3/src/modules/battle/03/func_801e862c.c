#include "internal.h"

/* does: dispatches the current result-ui aux state byte through one of two
 * fixed handlers.
 * @source: 0x801e862c FUN_801e862c
 */
void BOF3_NO_SIBLING_CALLS func_801e862c(void) {
  Battle03Handler handler;
  Battle03Handler table[2];

  handler = BOF3_BATTLE_RESULT_UI_AUX_HANDLER_0;
  table[0] = handler;
  handler = BOF3_BATTLE_RESULT_UI_AUX_HANDLER_1;
  table[1] = handler;
  table[BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_01]();
}
