#include "bof3/ui/game00_internal.h"

/* @behavior initializes the second frontend callback bank, ticks its shared
 * update paths, and advances to the next sub-state.
 * @source 0x80197A60
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void bank2Init(void) {
  u16* state;

  func_801BEDD0();
  func_801A06D8();
  runFrameFinalizationServices();
  state = &D_80143B92;
  (*state)++;
}
