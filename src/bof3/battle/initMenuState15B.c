#include "bof3/battle/battle15_internal.h"

/* @behavior Initializes battle menu state 0x15 and advances its update counter.
 * @source 0x800A5DE0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initMenuState15B(void) {
  u8 *counter;
  u8 count;

  func_80158DB8(0x15, 5);
  D_80148628 = -190;
  D_8014862A = 0x3f;
  D_80148626 = 0;
  D_80148627 = 0;
  D_8014862E = 1;
  counter = (u8 *)&D_801462E4;
  count = *counter;
  D_801485DF = 2;
  *counter = (u8)(count + 1);
}
