#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @sets D_80148627 to 2 and increments the global counter at D_801462E4.
 * @source 0x800A4B08
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800A4B08(void) {
  u8 *counter;
  u8 value;

  counter = &D_801462E4;
  value = *counter;
  D_80148627 = 2;
  *counter = value + 1;
}
