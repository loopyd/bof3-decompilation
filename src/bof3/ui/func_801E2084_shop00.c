#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2084
 * @behavior shop sub-step handler reached as entry 0 of the sub-step table
 *           D_801E5D48, which func_801E2048 dispatches with the UI sub-step
 *           byte D_80148652 as index: initializes the shop UI state through
 *           initializeShopUiState, re-arms the frame timer phaseTimer with 6,
 *           advances the UI sub-step byte D_80148652 by one and then calls the
 *           main-exe sound cue dispatcher func_8015DF18 with the cue id 0x102.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2084(void) {
  /* Plain view of the volatile frame-timer cell: the original schedule keeps
   * the cue constant adjacent to the closing jal (li in its delay slot), which
   * the volatile store's ordering barriers defeat (they schedule the constant
   * into the sub-step load-delay slot and leave a nop at the call). */
  u8* timer = (u8*)&phaseTimer;

  initializeShopUiState();
  *timer = 6;
  D_80148652 += 1;
  func_8015DF18(0x102);
}
