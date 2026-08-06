#include "internal.h"

/* @source 0x800B24CC
 * @behavior Advances the panel task x position and ends its state at the limit.
 */

void battle15_panel_step_right_clamp_98(void) {
  PanelTask* task = g_PanelTaskRoot;
  s16        value;

  value = task->x + 0x20;
  task->x = value;
  if (value >= 0x99) {
    task->x = 0x98;
    task->state = 0;
  }
}
