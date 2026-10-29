#include "bof3/battle/battle15_internal.h"

/* @behavior Stores 2 into byte D_80148627, reads byte D_801462E4, stores 1
 * into byte D_8014862E, then stores the previously read value plus 1 modulo
 * 256 into D_801462E4.
 * @source 0x800A59E8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A59E8(void) {
  u8 *counter = (u8 *)&D_801462E4;
  u8 value;

  D_80148627 = 2;
  value = *counter;
  D_8014862E = 1;
  *counter = value + 1;
}
