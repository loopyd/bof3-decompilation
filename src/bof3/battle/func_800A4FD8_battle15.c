#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A4FD8
 * @behavior Returns whether the selected battler's 0x140-stride halfword above
 * 0x80145F18 is at least the sum of the mapped lookup bytes of the four-byte
 * battle-grid cell selected by the two work counters, where a cell byte of
 * 0xFF gates the whole test and each of the two trailing cell bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_800A4FD8(void) {
  u8 sum;
  u16 value;
  u8 battler_index;

  if (D_80145500[D_801485C8 + D_801485CA][3] == 0xFF) {
    return 0;
  }

  sum = D_800B4C7C[D_80145500[D_801485C8 + D_801485CA][0]];
  if (D_80145500[D_801485C8 + D_801485CA][1] != 0xFF) {
    sum += D_800B4C7C[D_80145500[D_801485C8 + D_801485CA][1]];
  }
  if (D_80145500[D_801485C8 + D_801485CA][2] != 0xFF) {
    sum += D_800B4C7C[D_80145500[D_801485C8 + D_801485CA][2]];
  }

  battler_index = D_801EBF08[5];
  value = *(u16*)(D_80145F18 + battler_index * 0x140 + 2);
  return value >= (u32)sum;
}
