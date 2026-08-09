#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @increments the global counter at D_801462E4 if D_801485E0 equals 0xA3.
 * @source 0x800A60A8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
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
