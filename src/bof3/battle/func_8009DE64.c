#include "bof3/battle/battle15_internal.h"

/* @stores -40 to the halfword at byte +0x04 of the record pointed to by 0x801463A0.
 * @source 0x8009DE64
 * @behavior Stores -40 in the halfword at byte +0x04 of the record pointed to by
 * 0x801463A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009DE64(void) {
  s16* ptr;

  ptr = (s16*)D_8014639C.record;
  ptr[2] = -40;
}
