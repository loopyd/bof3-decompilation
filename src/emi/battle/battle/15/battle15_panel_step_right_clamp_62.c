#include "internal.h"

/* @source 0x800B2438
 * @behavior Advances the panel task x position and ends its state at the limit.
 */

void battle15_panel_step_right_clamp_62(void) {
  PanelTask* task = g_PanelTaskRoot;
  s16        value;

  value = task->x + 0x20;
  task->x = value;
  if (value >= 0x63) {
    task->x = 0x62;
    task->state = 0;
  }
}
