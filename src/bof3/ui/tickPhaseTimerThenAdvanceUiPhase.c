#include "bof3/ui/shop00_internal.h"

/* @source 0x801D65EC
 * @behavior shop UI phase step at entry 3 of the phase step table D_801E5360,
 *           which the dispatcher func_801D6184 selects with the UI phase byte
 *           D_80148651: appends the fullscreen dim tile through
 *           appendFullscreenDimTile, then counts the shared frame timer
 *           phaseTimer down. When the timer is already zero, or its decrement
 *           reaches zero, it polls the main-exe predicate func_80163EA0 and,
 *           only when that reports a nonzero low byte, advances the UI phase
 *           byte D_80148651 by one and switches the frontend-local mode
 *           through func_8014ECAC with the bare constant 1. A timer that is
 *           still running returns without either side effect. The 124 bytes
 *           are byte-identical to the reviewed exact sub-step twin
 *           tickPhaseTimerThenAdvanceUiSubstep (0x801D43F0) except for the two
 *           %lo immediates 0x8651/0x8652, i.e. this member advances the phase
 *           byte instead of the sub-step byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickPhaseTimerThenAdvanceUiPhase(void) {
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
    D_80148651 += 1;
    func_8014ECAC(1);
  }
}
