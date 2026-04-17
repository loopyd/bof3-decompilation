#include "internal.h"

/* does: dispatches the current default-class local byte through its table.
 * @source: 0x801e1e7c FUN_801e1e7c
 */
void BOF3_NO_SIBLING_CALLS func_801e1e7c(void) {
  Battle03Handler handler;
  u8              index;

  index = (*(volatile Battle03LocalWork**)0x1f800044u)->unk_02;
  handler = *(Battle03Handler const volatile*)((volatile u8*)0x801f0000u +
                                               ((u32)index << 2) - 0x4d84u);
  handler();
}
