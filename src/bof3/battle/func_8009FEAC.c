#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores byte 2 at offset 8 and halfword -5 at offset 6 of the pointer from D_801463A0.
 * @source 0x8009FEAC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8009FEAC(void) {
  ((u8*)D_801463A0)[8] = 2;
  *(s16*)((u8*)D_801463A0 + 6) = -5;
}
