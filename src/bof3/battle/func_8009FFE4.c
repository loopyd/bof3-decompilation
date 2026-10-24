#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores 0x-5 to offset 2 of the pointer loaded from D_8014639C.record.
 * @source 0x8009FFE4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009FFE4(void) {
  ((s16*)D_8014639C.record)[2] = -5;
}
