#include "bof3/ui/game00_internal.h"

/* @behavior Marks the current palette range's source colors as high-bit colors.
 * @source 0x80196B9C
 * @status partial
 * @match 58.33
 * @residual First mismatch +0x0C: original lbu a1,0x28(a2), current lbu v0,0x28(a2) (table-index allocation). Supported -O2 -fno-delayed-branch scored 60.0% but still omitted the original bnez/nop/break 7 divisor-zero trap; gcc-2.6.3 -O2 also scored 60.0% and omitted that trap, so neither was retained. Next evidence: recover a clean-C division spelling/profile that emits the original divisor-zero break sequence before further allocation tuning.
 */
void func_80196B9C(void) {
  volatile u8*  work;
  u8            table_index;
  u32           value;
  u32           divisor;
  u32           quotient;
  u32           remainder;
  u32           start;
  u32           stride;
  volatile u16* colors;
  u32           color_index;
  u32           count;

  work = SPAD_PTR_SLOT(volatile u8, 0x44u);
  table_index = work[0x28];
  value = work[0x27];
  divisor = D_801C7AE0[table_index];
  quotient = value / divisor;
  remainder = value % divisor;
  stride = D_801C7AE8[table_index];
  start = quotient;
  if ((work[0x24] & 4u) != 0u) {
    start += 0x10u;
  }

  count = D_801C7AD8[table_index] << 4;
  color_index = 1u;
  if (color_index < count) {
    colors = &PSX_PTR(volatile u16, 0x80037800u)[(start & 0xffu) * 256u];
    do {
      colors[remainder * stride + color_index] |= 0x8000u;
      color_index++;
    } while (color_index < (D_801C7AD8[work[0x28]] << 4));
  }
  paletteStageSerial = 1u;
}
