#include "internal.h"

/* @behavior emits one flat-colored line/box primitive by writing rgb and two corner
 * points directly into the current primitive.
 * @source 0x801DA408
 */
void func_801DA408(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6) {
  u32 primitive;
  u8  color0;
  u8  color1;
  u8  color2;
  s16 x0;
  s16 y0;
  s16 x1;
  s16 y1;

  primitive = BATTLE_GLOBAL_WORD_598C;
  color0 = arg4;
  color1 = arg5;
  color2 = arg6;
  x0 = arg0;
  y0 = arg1;
  x1 = arg2;
  y1 = arg3;
  func_8017AA80(primitive);
  *(u8*)(primitive + 4) = color0;
  *(u8*)(primitive + 5) = color1;
  *(u8*)(primitive + 6) = color2;
  *(s16*)(primitive + 8) = x0;
  *(s16*)(primitive + 10) = y0;
  *(s16*)(primitive + 12) = x1;
  *(s16*)(primitive + 14) = y1;
  func_8014E5A0(1u, 0x10u);
}
