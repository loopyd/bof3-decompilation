#include "bof3/battle/battle15_internal.h"

/* @source 0x8009DE78
 * @behavior Stores -100 into the signed halfword at byte offset 4 of the pointer
 * loaded from D_801463A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009DE78(void) {
  s16* ptr;

  ptr = (s16*)D_8014639C.record;
  *(ptr + 2) = -100;
}
