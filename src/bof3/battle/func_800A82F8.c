#include "bof3/battle/battle15_internal.h"

/* @source 0x800A82F8
 * @behavior Zeroes the byte accumulator at D_801463B8, then adds the mapped
 *   value of every active list byte into it, once per entry of the active
 *   byte count.
 * @status partial
 * @match 77.78
 * @residual GCC loop-preheader emission order, pointer copy and missing bound copy differ; current body is 108 bytes versus 104 original.
 */
void func_800A82F8(void)
{
  u8 i;
  u8 *dst;
  u8 *p;

  dst = &D_801463B8;
  *dst = 0;
  p = dst;
  for (i = 0; i < D_801463C7; i++) {
    *p += D_800B4C7C[D_801463C4[i]];
  }
}
