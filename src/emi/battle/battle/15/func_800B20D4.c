#include "internal.h"

/* @source 0x800B20D4
 * @behavior UNKNOWN: exact behavior is not yet documented.
 */

void func_800B20D4(void) {
  PanelTask* task = g_PanelTaskRoot;

  func_801647C4(task->x, task->field_06, 0);
}
