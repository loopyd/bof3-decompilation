#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0A2C
 * @behavior Loads the pointer at 0x80148648, stores halfwords 0x005A at +0x04 and
 * 0xFFE9 at +0x06, and increments byte +0x03 modulo 256.
 * @status exact
 * @match 100.00
 * @residual none
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
