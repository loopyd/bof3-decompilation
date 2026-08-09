#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE8E8
 * @behavior Appends a fullscreen dim tile, then decrements the phase timer;
 *           when it reaches zero, advances the phase byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801DE8E8(void) {
  volatile u8* p = &phaseTimer;
  u8 val;

  appendFullscreenDimTileB();
  val = *p - 1;
  *p = val;
  if (val == 0) {
    D_80148652 += 1;
  }
}
