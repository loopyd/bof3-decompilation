#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2BC4
 * @behavior forwards the panel task's x and field-6 halfwords, the u16 entry of
 *           the D_801E5DD8 table selected by task byte 0xA, the constant 0x34,
 *           task byte 0xB and the constant 6 to func_801D8E2C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2BC4(void) {
  PanelTask* task;
  u32 index;

  task = D_80148648;
  index = *((u8*)task + 0xA);
  func_801D8E2C(task->x, task->field_06, D_801E5DD8[index], 0x34,
                *((u8*)task + 0xB), 6);
}
