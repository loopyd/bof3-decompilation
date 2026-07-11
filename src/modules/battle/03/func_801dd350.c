#include "internal.h"

/* @behavior rotates or mirrors the first point pair at offsets `0x0c/0x10` based on
 * the selector byte at offset `8`.
 * @source 0x801dd350 FUN_801dd350
 */
void func_801dd350(s32 arg0) {
  s32 mode;
  s32 value_0c;
  s32 value_10;

  mode = *(u8*)(arg0 + 8);
  if (mode == 1) {
    goto block_1;
  }
  if (mode < 2) {
    return;
  }
  if (mode == 2) {
    goto block_2;
  }
  if (mode != 3) {
    return;
  }

  value_10 = *(s32*)(arg0 + 0x10);
  value_0c = *(s32*)(arg0 + 0x0c);
  *(s32*)(arg0 + 0x0c) = value_10;
  *(s32*)(arg0 + 0x10) = -value_0c;
  return;

block_1:
  value_10 = *(s32*)(arg0 + 0x10);
  value_0c = *(s32*)(arg0 + 0x0c);
  *(s32*)(arg0 + 0x0c) = -value_10;
  *(s32*)(arg0 + 0x10) = value_0c;
  return;

block_2:
  value_10 = *(s32*)(arg0 + 0x0c);
  value_0c = *(s32*)(arg0 + 0x10);
  *(s32*)(arg0 + 0x0c) = -value_10;
  *(s32*)(arg0 + 0x10) = -value_0c;
}
