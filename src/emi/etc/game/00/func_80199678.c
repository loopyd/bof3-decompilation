#include "internal.h"

/* @source 0x80199678
 * @behavior submits the current panel x and field-six coordinates to the panel helper.
 */
void func_80199678(void) {
  PanelTask* task = g_PanelTaskRoot;

  func_801647C4(task->x, task->field_06, 0);
}
