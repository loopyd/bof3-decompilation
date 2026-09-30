#include "bof3/world/area03213_internal.h"

/* @behavior Arms the scratch cursor record after the 0x801C2438 palette setup
 * call, setting its bytes 4 and 0x5C to 1 and advancing its 0x34 accumulator by
 * the 0xC step, then consumes two units of the shared countdown word at 0x122
 * of the current work record.
 * @source 0x801F3728
 * @status partial
 * @match 65.38
 * @residual same-size scheduler/allocator residual (17/26 insns): the original
 * emits the 0x34 accumulator load before the byte-4 store and the 0xC step load
 * after it (and keeps the constant in a0), while every clean-C statement order
 * measured here emits the step load first (constant a1); the countdown pointer
 * load is also hoisted above the accumulator store in the original, filling the
 * load delay slot and saving the trailing nop. Clean-C orders, local hoists and
 * declaration orders all reproduced the same 26-instruction stream; profile and
 * permuter rungs are not authorized for this lane.
 */
void func_801F3728(void) {
  u8* record;
  s32 step;

  func_801C2438();
  D_1F800044[0x5c] = 1;
  record = D_1F800044;
  record[4] = 1;
  step = *(s32*)(record + 0x0c);
  *(s32*)(record + 0x34) += step;
  *(u16*)(D_80146250 + 0x122) -= 2;
}
