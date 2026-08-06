#include "internal.h"

/* @source 0x801E25CC
 * @behavior Decrements a frame counter; when it wraps to zero, increments
 *           a secondary counter.
 */
void tickPhaseTimerB(void) {
  volatile u8* p = &phaseTimer;
  u8           val = *p - 1;
  *p = val;
  if (val == 0) {
    D_80148652 += 1;
  }
}
