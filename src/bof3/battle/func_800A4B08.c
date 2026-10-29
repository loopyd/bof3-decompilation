#include "bof3/battle/battle15_internal.h"

/* @behavior Reads byte D_801462E4, stores 2 into byte D_80148627, then stores
 * the previously read value plus 1 modulo 256 into D_801462E4.
 * @source 0x800A4B08
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A4B08(void) {
  u8 *counter;
  u8 value;

  counter = &D_801462E4;
  value = *counter;
  D_80148627 = 2;
  *counter = value + 1;
}
