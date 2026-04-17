#include "internal.h"

/* does: dispatches the current panel-task root byte through its eight-entry
 * table.
 * @source: 0x801e9074 FUN_801e9074
 */
void BOF3_NO_SIBLING_CALLS func_801e9074(void) {
  struct PanelTaskRootTable {
    Battle03Handler handlers[8];
  } handlers = *(struct PanelTaskRootTable const volatile*)
                   BOF3_BATTLE_PANEL_TASK_ROOT_TABLE;

  handlers.handlers[BOF3_BATTLE_PANEL_TASK_PTR[2]]();
}
