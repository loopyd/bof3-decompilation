#include "internal.h"

/* does: dispatches the current queued-result substate byte through its table.
 * @source: 0x801e4928 FUN_801e4928
 */
void NO_SIBLING_CALLS func_801e4928(void) {
  volatile u8* scratch;

  scratch = *(volatile u8**)0x1f800044u;
  (*(Battle03Handler const volatile*)((volatile u8*)0x801f0000u +
                                      ((u32)scratch[3] << 2) - 0x4bacu))();
}
