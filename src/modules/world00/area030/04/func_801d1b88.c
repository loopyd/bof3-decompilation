#include "internal.h"

/* @behavior emits two short vertical gradient quads stacked four pixels apart.
 * @source 0x801d1b88 FUN_801d1b88
 */
void func_801d1b88(s16 arg0, s16 arg1, s16 arg2, u8 arg3) {
  u32 primitive;
  s16 bottom_y;

  primitive = (u32)WORLD00_AREA030_PRIMITIVE_PTR;
  SetPolyG4((POLY_G4*)primitive);
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 0x10) = (s16)(arg0 + arg2);
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile s16*)(primitive + 0x12) = arg1;
  *(volatile u8*)(primitive + 4) = 0u;
  *(volatile u8*)(primitive + 5) = 0u;
  *(volatile u8*)(primitive + 6) = 0u;
  *(volatile u8*)(primitive + 0x0cu) = 0u;
  *(volatile u8*)(primitive + 0x0du) = 0u;
  *(volatile u8*)(primitive + 0x0eu) = 0u;
  *(volatile u8*)(primitive + 0x14) = 0u;
  *(volatile u8*)(primitive + 0x15) = 0x80u;
  *(volatile u8*)(primitive + 0x16) = 0u;
  *(volatile u8*)(primitive + 0x1cu) = 0u;
  *(volatile u8*)(primitive + 0x1du) = 0x80u;
  *(volatile u8*)(primitive + 0x1eu) = 0u;
  bottom_y = (s16)(arg1 + 4);
  *(volatile s16*)(primitive + 0x18) = *(volatile s16*)(primitive + 8);
  *(volatile s16*)(primitive + 0x20) = *(volatile s16*)(primitive + 0x10);
  *(volatile s16*)(primitive + 0x1a) = bottom_y;
  *(volatile s16*)(primitive + 0x22) = bottom_y;
  func_8014e5a0(arg3, 0x24u);

  primitive = (u32)WORLD00_AREA030_PRIMITIVE_PTR;
  SetPolyG4((POLY_G4*)primitive);
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 0x10) = (s16)(arg0 + arg2);
  *(volatile s16*)(primitive + 10) = (s16)(arg1 + 4);
  *(volatile s16*)(primitive + 0x12) = (s16)(arg1 + 4);
  *(volatile u8*)(primitive + 4) = 0u;
  *(volatile u8*)(primitive + 5) = 0x80u;
  *(volatile u8*)(primitive + 6) = 0u;
  *(volatile u8*)(primitive + 0x0cu) = 0u;
  *(volatile u8*)(primitive + 0x0du) = 0x80u;
  *(volatile u8*)(primitive + 0x0eu) = 0u;
  *(volatile u8*)(primitive + 0x14) = 0u;
  *(volatile u8*)(primitive + 0x15) = 0u;
  *(volatile u8*)(primitive + 0x16) = 0u;
  *(volatile u8*)(primitive + 0x1cu) = 0u;
  *(volatile u8*)(primitive + 0x1du) = 0u;
  *(volatile u8*)(primitive + 0x1eu) = 0u;
  bottom_y = (s16)(arg1 + 8);
  *(volatile s16*)(primitive + 0x18) = *(volatile s16*)(primitive + 8);
  *(volatile s16*)(primitive + 0x20) = *(volatile s16*)(primitive + 0x10);
  *(volatile s16*)(primitive + 0x1a) = bottom_y;
  *(volatile s16*)(primitive + 0x22) = bottom_y;
  func_8014e5a0(arg3, 0x24u);
}
