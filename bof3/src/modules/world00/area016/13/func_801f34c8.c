#include "internal.h"

/* does: dispatches through the local handler table selected by scratchpad
 * state byte `0x02`.
 * @source: 0x801f34c8 FUN_801f34c8
 */
void func_801f34c8(void) {
  u32 index;

  index = (u32)WORLD00_AREA016_SCRATCH_PTR->state_02 << 2;
  (*(World00Area016Handler const volatile*)((u8*)0x801f0000 + 0x511c +
                                            index))();
}
