#include "internal.h"

/* @source 0x801E25CC
 * @behavior Decrements a frame counter; when it wraps to zero, increments
 *           a secondary counter.
 */
void shop_phase_timer_tick_2(void) {
  volatile u8* p = &SHOP_PHASE_TIMER;
  u8           val = *p - 1;
  *p = val;
  if (val == 0) {
    D_80148652 += 1;
  }
}
