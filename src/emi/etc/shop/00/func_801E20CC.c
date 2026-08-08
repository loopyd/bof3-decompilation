#include "internal.h"

/* @source 0x801E20CC
 * @behavior Decrements the phase timer; on wrap, clears the sub-step and
 *           advances the phase.
 */
void func_801E20CC(void) {
  volatile u8* p = &phaseTimer;
  u8 val = *p - 1;

  *p = val;
  if (val == 0) {
    u8 phase = D_80148651;

    D_80148652 = 0;
    D_80148651 = phase + 1;
  }
}
