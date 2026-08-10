#include "bof3/ui/game00_internal.h"

/* @behavior runs the second frontend bank's paired local updates, ticks the
 * shared paths, and advances to its final sub-state.
 * @source 0x80197EFC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void bank2FinalUpdates(void) {
  u16* state;

  func_801BF8E0();
  func_801BFAC4();
  func_801A06D8();
  runFrameFinalizationServices();
  state = &D_80143B92;
  (*state)++;
}
