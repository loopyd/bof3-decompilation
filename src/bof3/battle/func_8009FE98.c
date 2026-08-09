#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores 0x-F0 to offset 2 of the pointer loaded from D_801463A0.
 * @source 0x8009FE98
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8009FE98(void) {
  s16* ptr;

  ptr = D_801463A0;
  *(ptr + 2) = -240;
}
