#include "bof3/world/area00813_internal.h"

/* @source 0x801F2D78
 * @behavior Overlay counter step: marks the shared counter active by storing 2
 * into the area status byte at 0x80149333, then retreats counter2 by 0x0A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By10World03Area13113_801F2D78(void) {
  u16 count_;

  count_ = counter2;
  D_80149333 = 2;
  counter2 = (u16)(count_ - 0x0A);
}
