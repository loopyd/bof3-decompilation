#include "internal.h"

/* @source 0x800B22AC
 * @behavior advances the local panel task X position by 32 pixels, clamps it
 * to 320, and clears the preceding state byte when the clamp is reached.
 */
void func_800B22AC(void) {
  BattleLocalPanelTask* task_root;
  u16                   next_x;

  task_root = D_80148648;
  next_x = (u16)(task_root->x + 32u);
  task_root->x = next_x;
  if ((s16)next_x >= 321) {
    task_root->x = 320u;
    task_root->state = 0u;
  }
}
