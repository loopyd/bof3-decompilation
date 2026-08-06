#include "internal.h"

/* @behavior Marks the current palette range's source colors as high-bit colors.
 * @source 0x80196B9C
 */
void func_80196B9C(void) {
  volatile u8*  work;
  u8            table_index;
  u32           value;
  u32           quotient;
  u32           remainder;
  u32           start;
  u32           stride;
  volatile u16* colors;
  u32           color_index;

  work = SPAD_PTR_SLOT(volatile u8, 0x44u);
  table_index = work[0x28];
  value = work[0x27];
  quotient = value / D_801C7AE0[table_index];
  remainder = value % D_801C7AE0[table_index];
  stride = D_801C7AE8[table_index];
  start = quotient;
  if ((work[0x24] & 4u) != 0u) {
    start += 0x10u;
  }

  if (1u < (D_801C7AD8[table_index] << 4)) {
    colors = &D_80037800[(start & 0xffu) * 256u];
    color_index = 1u;
    do {
      colors[remainder * stride + color_index] |= 0x8000u;
      color_index++;
    } while (color_index < (D_801C7AD8[work[0x28]] << 4));
  }
  GAME_FRONT_PALETTE_STAGE_SERIAL = 1u;
}
