#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores byte 2 at offset 8 and halfword -0x28 at offset 6 of the pointer from D_801463A0.
 * @source 0x8009FED0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8009FED0(void) {
  ((volatile u8*)D_801463A0)[8] = 2;
  D_801463A0[3] = -0x28;
}
