#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FAE4
 * @behavior Raises bit 0x4000 of the battle global halfword D_801462E8, copies
 *   the battle bytes D_80146374 and D_80146384 into D_801463CA and D_801463CB,
 *   then runs the selection-input refresh pass stateRefreshFlagStatus.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateFlagCopy4000(void) {
  volatile u16 *flags = &D_801462E8;
  u16 value;
  u8 first, second;

  value = *flags;
  first = D_80146374;
  second = D_80146384;
  *flags = value | 0x4000;
  D_801463CA = first;
  D_801463CB = second;
  stateRefreshFlagStatus();
}
