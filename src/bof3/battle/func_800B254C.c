#include "bof3/battle/battle15_internal.h"

/* @source 0x800B254C
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */

void func_800B254C(void) {
  PanelTask* task = g_PanelTaskRoot;
  s16        val;

  val = task->x - 0x20;
  task->x = val;
  if (val < 0x98) {
    task->x = 0x98;
    task->state = 0;
  }
}
