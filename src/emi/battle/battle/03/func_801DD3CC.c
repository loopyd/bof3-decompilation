#include "internal.h"

/* @behavior rotates or mirrors the second point pair at offsets `0x18/0x1c` based
 * on the selector byte at offset `8`.
 * @source 0x801DD3CC
 */
void func_801DD3CC(s32 arg0) {
  s32 mode;
  s32 value_18;
  s32 value_1c;

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
  goto block_3;

block_1:
  value_1c = -*(s32*)(arg0 + 0x1c);
  value_18 = *(s32*)(arg0 + 0x18);
  goto store;

block_2:
  value_1c = -*(s32*)(arg0 + 0x18);
  value_18 = -*(s32*)(arg0 + 0x1c);

store:
  *(s32*)(arg0 + 0x18) = value_1c;
  *(s32*)(arg0 + 0x1c) = value_18;
  return;

block_3:
  value_1c = *(s32*)(arg0 + 0x1c);
  value_18 = *(s32*)(arg0 + 0x18);
  *(s32*)(arg0 + 0x18) = value_1c;
  *(s32*)(arg0 + 0x1c) = -value_18;
}
