#include "bof3/battle/battle15_internal.h"

/* @source 0x800A82F8
 * @behavior Zeroes the byte accumulator at D_801463B8, then adds the mapped
 *   value of every active list byte into it, once per entry of the active
 *   byte count.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A82F8(void)
{
  u8 i;
  u32 count;
  u8 *dst;
  u8 *p;

  i = 0;
  count = D_801463C7;
  dst = &D_801463B8;
  *dst = 0;
  if (count != 0) {
    p = dst;
    do {
      *p += D_800B4C7C[D_801463C4[i]];
      i++;
    } while (i < D_801463C7);
  }
}
