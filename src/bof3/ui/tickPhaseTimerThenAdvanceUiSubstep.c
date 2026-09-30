#include "bof3/ui/shop00_internal.h"

/* @source 0x801D43F0
 * @behavior shop UI sub-step handler at entry 3 of the sub-step handler table
 *           D_801E52F0, which the dispatcher func_801D41B0 selects with the UI
 *           sub-step byte D_80148652: appends the fullscreen dim tile through
 *           appendFullscreenDimTile, then counts the shared frame timer
 *           phaseTimer down. When the timer is already zero, or its decrement
 *           reaches zero, it polls the main-exe predicate func_80163EA0 and,
 *           only when that reports a nonzero low byte, advances the UI
 *           sub-step byte D_80148652 by one and switches the frontend-local
 *           mode through func_8014ECAC with the bare constant 1. A timer that
 *           is still running returns without either side effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickPhaseTimerThenAdvanceUiSubstep(void) {
  volatile u8* timer;
  u8           value;

  /* The original materializes the timer address once after the dim-tile call
   * and reuses that base for the countdown load and store (lui+addiu / lbu /
   * sb through one register that is not live across a call). */
  appendFullscreenDimTile();
  timer = &phaseTimer;
  value = *timer;
  if (value != 0) {
    value = value - 1;
    *timer = value;
    if (value != 0) {
      return;
    }
  }
  if ((u8)func_80163EA0() != 0) {
    D_80148652 += 1;
    func_8014ECAC(1);
  }
}
