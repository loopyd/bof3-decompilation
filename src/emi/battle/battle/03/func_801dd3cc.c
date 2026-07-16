#include "internal.h"

/* @behavior rotates or mirrors the second point pair at offsets `0x18/0x1c` based
 * on the selector byte at offset `8`.
 * @source 0x801DD3CC
 */
void func_801DD3CC(s32 arg0) {
  s32 mode;
  s32 value_1c;
  s32 value_18;

  mode = *(u8*)(arg0 + 8);
  if (mode == 1) {
    goto mode_1;
  }
  if (mode < 2) {
    return;
  }
  if (mode == 2) {
    goto mode_2;
  }
  if (mode == 3) {
    goto mode_3;
  }
  return;

mode_1:
  value_1c = *(s32*)(arg0 + 0x1c);
  value_18 = *(s32*)(arg0 + 0x18);
  goto done;

mode_2:
  value_1c = *(s32*)(arg0 + 0x18);
  value_18 = -*(s32*)(arg0 + 0x1c);

done:
  *(s32*)(arg0 + 0x18) = -value_1c;
  *(s32*)(arg0 + 0x1c) = value_18;
  return;

mode_3:
  value_18 = *(s32*)(arg0 + 0x18);
  *(s32*)(arg0 + 0x18) = *(s32*)(arg0 + 0x1c);
  *(s32*)(arg0 + 0x1c) = -value_18;
}
