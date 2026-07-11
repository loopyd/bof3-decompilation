#include "internal.h"

/* @behavior builds one icon/tile primitive using the byte-sized clut table at
 * `0x801d0c64`, then queues it through the standard primitive path.
 * @source 0x801d7d10 FUN_801d7d10
 */
void func_801d7d10(u8 arg0, s16 arg1, s16 arg2, u16 arg3, u8 arg4, u8 arg5) {
  u16 primitive_id;
  u32 primitive;
  s16 x1;

  primitive_id = func_8017a620(0, 0, 0x100, 0x100);
  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, primitive_id, 0);
  func_8014e5a0(1u, 0x0cu);

  func_8017a9b8(BATTLE_GLOBAL_WORD_598C);
  primitive = BATTLE_GLOBAL_WORD_598C;

  x1 = arg1 + (s16)(arg3 & 0xffu);

  *(volatile s16*)(primitive + 10) = arg2;
  *(volatile s16*)(primitive + 18) = arg2;
  *(volatile s16*)(primitive + 8) = arg1;
  *(volatile s16*)(primitive + 24) = arg1;
  *(volatile u8*)(primitive + 12) = arg0 * 0x18u;
  *(volatile u8*)(primitive + 28) = arg0 * 0x18u;
  *(volatile u8*)(primitive + 4) = arg5;
  *(volatile u8*)(primitive + 5) = arg5;
  *(volatile u8*)(primitive + 6) = arg5;
  *(volatile s16*)(primitive + 16) = x1;
  *(volatile u16*)(primitive + 26) = (u16)arg4 + (u16)arg2;
  *(volatile s16*)(primitive + 32) = x1;
  *(volatile u16*)(primitive + 34) = (u16)arg4 + (u16)arg2;
  *(volatile u8*)(primitive + 13) = 0xe0u;
  *(volatile u8*)(primitive + 20) = (arg0 * 0x18u) + 0x18u;
  *(volatile u8*)(primitive + 21) = 0xe0u;
  *(volatile u8*)(primitive + 29) = 0xf8u;
  *(volatile u8*)(primitive + 36) = (arg0 * 0x18u) + 0x18u;
  *(volatile u8*)(primitive + 37) = 0xf8u;
  *(volatile u16*)(primitive + 22) = func_8017a620(0, 0, 0x100, 0x100);
  *(volatile u16*)(primitive + 14) =
      (BATTLE_ICON_CLUT_TABLE_0C64[arg0] & 0x3fu) | 0x7800u;
  func_8014e5a0(1u, 0x28u);
}
