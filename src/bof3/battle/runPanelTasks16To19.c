#include "bof3/battle/battle15_internal.h"

/* @source 0x8009B20C
 * @behavior traverses PanelTask records 16 through 19 and calls func_80158E20 for each.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */

void runPanelTasks16To19(void) {
  u8  i;
  u8* base;
  u8* ptr;

  i = 0x10;
  base = D_80148330;
  do {
    ptr = (u8*)((u32)i * 0x24u + (u32)base);
    D_80148648 = (PanelTask*)ptr;
    i += 1;
    func_80158E20();
  } while (i < 0x14);
}
