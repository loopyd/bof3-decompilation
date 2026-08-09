#include "bof3/ui/game00_internal.h"

/* @source 0x80199678
 * @behavior submits the current panel x and field-six coordinates to the panel helper.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void submitPanelPosition(void) {
  PanelTask* task = g_PanelTaskRoot;

  func_801647C4(task->x, task->field_06, 0);
}
