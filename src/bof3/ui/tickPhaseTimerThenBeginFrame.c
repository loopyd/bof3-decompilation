#include "bof3/ui/shop00_internal.h"

/* @source 0x801E0F28
 * @behavior Decrements the per-frame phase timer and, when the decremented
 *           value wraps to zero, begins one shared front-end frame/update
 *           slice through func_80158E50 and arms the write-only byte
 *           D_80148650 with 1 (that byte has no loads in this target, so its
 *           role is unproven). Reached as entry 4 of the phase handler table
 *           D_801E5CE8, which func_801DF978 dispatches with the UI phase byte
 *           D_80148651 as index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickPhaseTimerThenBeginFrame(void) {
  volatile u8* p = &phaseTimer;
  u8           val = *p - 1;
  *p = val;
  if (val == 0) {
    func_80158E50();
    D_80148650 = 1;
  }
}
