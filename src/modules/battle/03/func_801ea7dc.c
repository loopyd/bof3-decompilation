#include "internal.h"

/* does: dispatches the current panel-task byte-3 state through the four-entry
 * result-ring icon table.
 * @source: 0x801ea7dc FUN_801ea7dc
 */
void NO_SIBLING_CALLS func_801ea7dc(void) {
  Battle03Handler table[4];

  table[0] = *(Battle03Handler const volatile*)0x801d1004u;
  table[1] = *(Battle03Handler const volatile*)0x801d1008u;
  table[2] = *(Battle03Handler const volatile*)0x801d100cu;
  table[3] = *(Battle03Handler const volatile*)0x801d1010u;
  table[BATTLE_PANEL_TASK_BYTE_03]();
}
