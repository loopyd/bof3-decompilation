#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores halfword 0x5A at offset 4, halfword -0x17 at offset 6 of D_80148648,
 * and increments byte at offset 3.
 * @source 0x800B0A2C
 */
void func_800B0A2C(void) {
  BattlePanelTask* task;
  u8 state;

  task = (BattlePanelTask*)D_80148648;
  task->x = 0x5A;
  state = task->state;
  task->field_06 = -0x17;
  task->state = state + 1;
}
