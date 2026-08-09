#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A6488
 * @behavior Copies one four-byte battle-grid cell between coordinate-selected
 * buffers, then advances the battle state byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800A6488(void) {
  s16 *dst_row = &D_801485EC;
  u8 *state = &D_801462E4;

  D_80145518[*dst_row + D_801485EE][0] =
      D_80145500[D_801485C8 + D_801485CA][0];
  D_80145518[*dst_row + D_801485EE][1] =
      D_80145500[D_801485C8 + D_801485CA][1];
  D_80145518[*dst_row + D_801485EE][2] =
      D_80145500[D_801485C8 + D_801485CA][2];
  D_80145518[*dst_row + D_801485EE][3] =
      D_80145500[D_801485C8 + D_801485CA][3];
  *state = *state + 1;
}
