#include "internal.h"

/* does: dispatches the current local presentation state-1 byte through its
 * table.
 * @source: 0x801e31c8 FUN_801e31c8
 */
void NO_SIBLING_CALLS func_801e31c8(void) {
  volatile u8**    scratch_root;
  volatile u8*     scratch;
  Battle03Handler* table;

  scratch_root = (volatile u8**)0x1f800000u;
  scratch = scratch_root[0x11];
  table = (Battle03Handler*)((u8*)0x801f0000u - 0x4c50u);
  table[scratch[1]]();
}
