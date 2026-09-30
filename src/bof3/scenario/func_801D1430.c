#include "bof3/scenario/sce10eff_internal.h"

/* @source 0x801D1430
 * @behavior Advances byte 11 of the scratchpad work object, increments its word
 * counter at offset 0x0C and publishes the counter's low seven bits shifted left
 * 5 as the angle offset in the volatile scratchpad cell at 0x1F800004 together
 * with the fixed 0x20 block size in the cell at 0x1F800000, then scales the
 * local rotation results func_801783C8/func_801782FC by that block size, adds
 * them to the 0x34/0x38 coordinate pair cached in the object at 0x801D2744 and
 * stores those sums in the work object, and finally counts down byte 9 and
 * advances byte 3 when that countdown reaches zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1430(void) {
  ScenarioSce10effScratch* work;
  volatile s32* slot;
  s32 counter;
  s32 phase;
  s32 delta;

  ((u8*)D_1F800044)[0x0b]++;

  counter = D_1F800044->unk_0c + 1;
  phase = (counter & 0x7f) << 5;
  D_1F800044->unk_0c = counter;
  slot = &D_1F800004;
  *slot = phase;
  D_1F800000 = 0x20;

  delta = func_801783C8(phase) * D_1F800000;
  D_1F800044->unk_34 = *(s32*)(D_801D2744 + 0x34) + delta;
  delta = func_801782FC(*slot) * D_1F800000;
  work = D_1F800044;
  work->unk_38 = *(s32*)(D_801D2744 + 0x38) + delta;
  if (--((u8*)work)[0x09] == 0u) {
    ((u8*)D_1F800044)[0x03]++;
  }
}
