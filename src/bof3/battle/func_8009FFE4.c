#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FFE4
 * @behavior Stores -5 in the halfword at byte +0x04 through the pointer loaded from
 * 0x801463A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009FFE4(void) {
  ((s16*)D_8014639C.record)[2] = -5;
}
