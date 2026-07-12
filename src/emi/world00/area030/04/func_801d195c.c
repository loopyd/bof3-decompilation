#include "internal.h"

/* @behavior draws the AREA030 footer marker stack, including the fixed gradient
 * bar, the moving sprite marker, one flat quad, and an optional 8x8 icon.
 * @source 0x801d195c FUN_801d195c
 */
void func_801d195c(s16 arg0, s16 arg1) {
  s16 icon_y;
  s16 temp;
  u32 primitive;

  func_801d1744((s16)(arg0 + 0x58), (s16)(arg1 + 8), 4u);

  icon_y = arg1;
  if (WORLD00_AREA030_GLOBAL_BYTE_3FC9 != 0u) {
    temp = func_8015477c(WORLD00_AREA030_GLOBAL_HALF_3FFC,
                         WORLD00_AREA030_GLOBAL_HALF_4000);
    if (temp < 0) {
      temp = (s16)(temp + 0x1f);
    }
    icon_y = (s16)(arg1 - (temp >> 5));
  }
  if (icon_y < arg1) {
    icon_y = arg1;
  }

  func_801d1818((s16)(arg0 + 0x59), icon_y, 4u);

  primitive = (u32)WORLD00_AREA030_PRIMITIVE_PTR;
  SetPolyF4((POLY_F4*)primitive);
  *(volatile s16*)(primitive + 8) = (s16)(arg0 + 0x59);
  *(volatile s16*)(primitive + 0x0cu) = (s16)(arg0 + 0x69);
  *(volatile s16*)(primitive + 10) = (s16)(icon_y + 0x10);
  *(volatile s16*)(primitive + 0x0eu) = (s16)(icon_y + 0x10);
  *(volatile s16*)(primitive + 0x10) = *(volatile s16*)(primitive + 8);
  *(volatile s16*)(primitive + 0x14) = *(volatile s16*)(primitive + 0x0cu);
  *(volatile s16*)(primitive + 0x12) = (s16)(arg1 + 0x28);
  *(volatile s16*)(primitive + 0x16) = (s16)(arg1 + 0x28);
  *(volatile u8*)(primitive + 4) = 0x48u;
  *(volatile u8*)(primitive + 5) = 0x20u;
  *(volatile u8*)(primitive + 6) = 0u;
  func_8014e5a0(4u, 0x18u);

  if (WORLD00_AREA030_GLOBAL_BYTE_3FC9 == 4u) {
    icon_y = 0xb4;
    if (WORLD00_AREA030_GLOBAL_HALF_4006 < 1) {
      temp = WORLD00_AREA030_GLOBAL_HALF_4006;
      if (temp < 0) {
        temp = (s16)(temp + 0x1f);
      }
      icon_y = (s16)(0xb4 - (temp >> 5));
    }
    func_801d18cc((s16)(arg0 + 0x69), icon_y, 2u);
  }
}
