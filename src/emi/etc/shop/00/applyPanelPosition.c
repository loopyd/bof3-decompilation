#include "internal.h"

/* @source 0x801E2B94
 * @behavior passes the panel position and field six to the common UI helper.
 */
void applyPanelPosition(void) {
  PanelTask* task = g_PanelTaskRoot;

  func_801647C4(task->x, task->field_06, 0);
}
