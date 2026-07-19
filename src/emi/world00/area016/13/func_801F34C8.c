#include "internal.h"

/* @behavior dispatches through the local handler table selected by scratchpad
 * state byte `0x02`.
 * @source 0x801F34C8
 */
void func_801F34C8(void) {
  u32 index;

  index = (u32)WORLD00_AREA016_SCRATCH_PTR->state_02 << 2;
  WORLD00_AREA016_STATE_TABLE[index]();
}
