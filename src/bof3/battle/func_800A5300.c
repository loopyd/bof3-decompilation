#include "bof3/battle/battle15_internal.h"

/* @behavior Stores 2 into byte D_80148597, reads byte D_801462E4, stores 1
 * into byte D_801483C3, then stores the previously read value plus 1 modulo
 * 256 into D_801462E4.
 * @source 0x800A5300
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A5300(void) {
  u8* counter;
  u8  value;

  counter = &D_801462E4;
  D_80148597 = 2;
  value = *counter;
  D_801483C3 = 1;
  *counter = value + 1;
}
