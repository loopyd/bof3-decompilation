#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FAE4
 * @behavior Raises bit 0x4000 of the battle global halfword D_801462E8, copies
 *   the battle bytes D_80146374 and D_80146384 into D_801463CA and D_801463CB,
 *   then runs the selection-input refresh pass func_800A3F28.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009FAE4(void) {
  D_801462E8 |= 0x4000;
  D_801463CA = D_80146374;
  D_801463CB = D_80146384;
  func_800A3F28();
}
