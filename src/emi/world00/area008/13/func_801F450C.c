#include "internal.h"

/* @behavior sets state byte 2 and advances the 16-bit count by 0x14.
 * @source 0x801F450C
 */
void func_801F450C(void) {
  u16 count;

  count = D_80149328;
  D_80149333 = 2;
  D_80149328 = (u16)(count + 0x14);
}
