#include "internal.h"

/* @behavior dispatches the current local state-2 event byte through its table.
 * @source 0x801e1b64 FUN_801e1b64
 */
void NO_SIBLING_CALLS func_801e1b64(void) {
  volatile u8* scratch;

  scratch = *(volatile u8**)0x1f800044u;
  (*(Battle03Handler const volatile*)((volatile u8*)0x801f0000u +
                                      ((u32)scratch[2] << 2) - 0x4d94u))();
}
