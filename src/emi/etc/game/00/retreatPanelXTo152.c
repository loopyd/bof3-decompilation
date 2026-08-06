#include "internal.h"

/* @source 0x8019973C
 * @behavior retreats panel x by 32 and clamps it to 152.
 */
void retreatPanelXTo152(void) {
  PanelTask* task = g_PanelTaskRoot;
  s16        val;

  val = task->x - 0x20;
  task->x = val;
  if (val < 0x98) {
    task->x = 0x98;
    task->state = 0;
  }
}
