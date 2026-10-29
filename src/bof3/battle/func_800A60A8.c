#include "bof3/battle/battle15_internal.h"

/* @behavior Increments byte D_801462E4 modulo 256 only when the signed
 * halfword at D_801485E0 equals 0xA3.
 * @source 0x800A60A8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A60A8(void) {
  s16* state;
  u8*  counter;

  state = (s16*)&D_801485E0;
  if (*state == 0xA3) {
    counter = &D_801462E4;
    *counter = *counter + 1;
  }
}
