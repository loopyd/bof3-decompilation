#include "internal.h"

/* does: dispatches the current local state-4 byte through its table.
 * @source: 0x801e1298 FUN_801e1298
 */
void BOF3_NO_SIBLING_CALLS func_801e1298(void) {
  volatile u8* scratch;

  scratch = *(volatile u8**)0x1f800044u;
  (*(Battle03Handler const volatile*)((volatile u8*)0x801f0000u +
                                      ((u32)scratch[4] << 2) - 0x4df0u))();
}
