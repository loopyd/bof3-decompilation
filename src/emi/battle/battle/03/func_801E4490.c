#include "internal.h"

/* @behavior dispatches the current local byte-3 presentation state through its
 * table.
 * @source 0x801E4490
 */
void NO_SIBLING_CALLS func_801E4490(void) {
  volatile u8* scratch;

  scratch = *(volatile u8**)0x1f800044u;
  (*(Battle03Handler const volatile*)((volatile u8*)0x801f0000u +
                                      ((u32)scratch[3] << 2) - 0x4bd0u))();
}
