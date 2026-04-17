#include "internal.h"

/* does: emits one 16x32 vertical gradient quad at the supplied screen point.
 * @source: 0x801d1744 FUN_801d1744
 */
void func_801d1744(s16 arg0, s16 arg1, u8 arg2) {
  u32 primitive;
  s16 bottom_y;

  primitive = (u32)BOF3_WORLD00_AREA030_PRIMITIVE_PTR;
  SetPolyG4((POLY_G4*)primitive);

  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 0x10) = (s16)(arg0 + 0x10);
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile u8*)(primitive + 4) = 100u;
  *(volatile u8*)(primitive + 5) = 100u;
  *(volatile u8*)(primitive + 0x0cu) = 100u;
  *(volatile u8*)(primitive + 0x0du) = 100u;
  *(volatile u8*)(primitive + 6) = 0x80u;
  *(volatile u8*)(primitive + 0x0eu) = 0x80u;
  *(volatile s16*)(primitive + 0x12) = arg1;
  *(volatile u8*)(primitive + 0x14) = 0u;
  *(volatile u8*)(primitive + 0x15) = 0u;
  *(volatile u8*)(primitive + 0x16) = 0x50u;
  *(volatile u8*)(primitive + 0x1cu) = 0u;
  *(volatile u8*)(primitive + 0x1du) = 0u;
  *(volatile u8*)(primitive + 0x1eu) = 0x50u;

  bottom_y = (s16)(*(volatile u16*)(primitive + 10) + 0x20);
  *(volatile u16*)(primitive + 0x18) = *(volatile u16*)(primitive + 8);
  *(volatile u16*)(primitive + 0x20) = *(volatile u16*)(primitive + 0x10);
  *(volatile s16*)(primitive + 0x1a) = bottom_y;
  *(volatile s16*)(primitive + 0x22) = bottom_y;

  func_8014e5a0(arg2, 0x24u);
}
