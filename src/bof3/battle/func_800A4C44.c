#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets D_80148627 to 2, clears D_8014862E, increments counter at D_801462E4.
 * @source 0x800A4C44
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800A4C44(void) {
  u8 *counter = (u8 *)&D_801462E4;
  u8 count = *counter;

  D_80148627 = 2;
  D_8014862E = 0;
  *counter = (u8)(count + 1);
}
