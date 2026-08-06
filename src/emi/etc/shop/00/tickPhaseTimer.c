#include "internal.h"

/* @source 0x801E0028
 * @behavior Decrements a frame counter; when it wraps to zero, increments
 *           a secondary counter.
 */
void tickPhaseTimer(void) {
  volatile u8* p = &phaseTimer;
  u8           val = *p - 1;
  *p = val;
  if (val == 0) {
    D_80148652 += 1;
  }
}
