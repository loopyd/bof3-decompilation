#include "internal.h"

/* @behavior emits one 8x8 sprite marker at the supplied point using fixed UV/clut
 * values for the AREA030 HUD path.
 * @source 0x801d18cc FUN_801d18cc
 */
void func_801d18cc(s16 arg0, s16 arg1, u8 arg2) {
  u32 primitive;

  primitive = (u32)WORLD00_AREA030_PRIMITIVE_PTR;
  SetSprt8((SPRT_8*)primitive);
  *(volatile u8*)(primitive + 4) = 0x80u;
  *(volatile u8*)(primitive + 5) = 0x80u;
  *(volatile u8*)(primitive + 6) = 0x80u;
  *(volatile u16*)(primitive + 0x0eu) = 0x7a40u;
  *(volatile u8*)(primitive + 0x0cu) = 0xb8u;
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile u8*)(primitive + 0x0du) = 0x48u;
  func_8014e5a0(arg2, 0x10u);
}
