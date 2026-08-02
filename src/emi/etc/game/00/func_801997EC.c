#include "internal.h"

/* @source 0x801997EC
 * @behavior retreats panel x with the template clamp behavior.
 */
void func_801997EC(void) {
  PanelTask* task_root;
  s16        next_x;
  task_root = D_80148648;
  next_x = (s16)((s32)task_root->x - 0x20);
  task_root->x = (u16)next_x;
  if (next_x < -0xAA) {
    next_x = -0xAA;
    task_root->x = (u16)next_x;
    task_root->state = 0u;
  }
}
