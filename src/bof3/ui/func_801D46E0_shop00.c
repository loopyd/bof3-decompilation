#include "bof3/ui/shop00_internal.h"

/* @source 0x801D46E0
 * @behavior emits the shop panel strip through this EMI's panel strip emitter
 *           func_801DAB90 with the fixed panel arguments (0x14, 0x12, 0x118,
 *           0x13) and the main-RAM CLUT-bank byte D_80144952 as the fifth
 *           argument, then advances the UI sub-step byte D_80148652 by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D46E0(void) {
  u8 *cell = &D_80148652;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  *cell += 1;
}
