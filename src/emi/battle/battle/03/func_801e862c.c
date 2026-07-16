#include "internal.h"

/* @behavior dispatches the current result-ui aux state byte through one of two
 * fixed handlers.
 * @source 0x801E862C
 */
void NO_SIBLING_CALLS func_801E862C(void) {
  Battle03Handler handler;
  Battle03Handler table[2];

  handler = BATTLE_RESULT_UI_AUX_HANDLER_0;
  table[0] = handler;
  handler = BATTLE_RESULT_UI_AUX_HANDLER_1;
  table[1] = handler;
  table[BATTLE_LOCAL_SCRATCH_PTR->unk_01]();
}
