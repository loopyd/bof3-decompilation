#include "bof3/battle/battle15_internal.h"

/* @source 0x8009B20C
 * @behavior traverses PanelTask records 16 through 19 and calls func_80158E20 for each.
 * @status exact
 * @match 100.00
 * @residual none
 */

void runPanelTasks16To19(void) {
  u8  taskIndex;
  u8* base;
  u8* ptr;

  taskIndex = 0x10;
  base = D_80148330;
  do {
    ptr = (u8*)((u32)taskIndex * 0x24u + (u32)base);
    D_80148648 = (PanelTask*)ptr;
    taskIndex += 1;
    func_80158E20();
  } while (taskIndex < 0x14);
}
