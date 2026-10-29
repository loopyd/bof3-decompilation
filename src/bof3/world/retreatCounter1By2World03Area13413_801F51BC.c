#include "bof3/bof3.h"

extern u16 D_80149328;
extern u8 D_80149333;

/* @source 0x801F51BC
 * @behavior Overlay counter step: marks the shared counter active by storing 2
 * into the area status byte at 0x80149333, then retreats the halfword counter
 * at 0x80149328 by 0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter1By2World03Area13413_801F51BC(void) {
  u16 count_;

  count_ = D_80149328;
  D_80149333 = 2;
  D_80149328 = (u16)(count_ - 0x02);
}
