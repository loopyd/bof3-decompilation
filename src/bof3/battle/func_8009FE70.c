#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FE70
 * @behavior Stores -1 in the halfword at byte +0x04 through the pointer loaded from
 * 0x801463A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009FE70(void) {
  s16* ptr;

  ptr = D_8014639C.record;
  *(ptr + 2) = -1;
}
