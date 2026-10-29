#include "bof3/battle/battle15_internal.h"

/* @behavior Reads byte D_801462E5, clears bytes D_801462E4 and D_801462E6,
 * stores the original value plus 3 modulo 256 into D_801462E3, and stores the
 * original value with bit 0x80 set into D_801462E5.
 * @source 0x800A4AD0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A4AD0(void) {
  u8 value = D_801462E5;

  D_801462E4 = 0;
  D_801462E6 = 0;
  D_801462E3 = value + 3;
  D_801462E5 = value | 0x80;
}
