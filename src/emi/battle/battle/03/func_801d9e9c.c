#include "internal.h"

/* @behavior emits a paired horizontal UI bar using one of two tile templates,
 * depending on the final signed mode byte.
 * @source 0x801d9e9c FUN_801d9e9c
 */
void func_801d9e9c(s16 arg0, s16 arg1, u16 arg2, u16 arg3, s8 arg4) {
  u32 primitive;
  s16 x1;
  u8  tex_left;
  u8  tex_right;

  primitive = BATTLE_GLOBAL_WORD_598C;
  func_8017a9b8(primitive);
  *(volatile u16*)(primitive + 0x16) = func_8017a620(0, 0, 0x3c0, 0);
  *(volatile u16*)(primitive + 0xe) =
      func_8017a6f0((arg4 == 0) ? 0xe0 : 0xb0, 0x1e0);
  *(volatile u8*)(primitive + 4) = 0x80u;
  *(volatile u8*)(primitive + 5) = 0x80u;
  *(volatile u8*)(primitive + 6) = 0x80u;
  x1 = arg0 + (arg2 & 0xffu);
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile s16*)(primitive + 0x10) = x1;
  *(volatile s16*)(primitive + 0x12) = arg1;
  *(volatile s16*)(primitive + 0x18) = arg0;
  *(volatile s16*)(primitive + 0x1a) = arg1 + 7;
  *(volatile s16*)(primitive + 0x20) = x1;
  *(volatile s16*)(primitive + 0x22) = arg1 + 7;
  tex_left = 0x80u;
  tex_right = 0x87u;
  if (arg4 == 0) {
    tex_left = 0x90u;
    tex_right = 0x97u;
  }
  *(volatile u8*)(primitive + 0xd) = 0xd8u;
  *(volatile u8*)(primitive + 0x15) = 0xd8u;
  *(volatile u8*)(primitive + 0xc) = tex_left;
  *(volatile u8*)(primitive + 0x14) = tex_right;
  *(volatile u8*)(primitive + 0x1c) = tex_left;
  *(volatile u8*)(primitive + 0x1d) = 0xdfu;
  *(volatile u8*)(primitive + 0x24) = tex_right;
  *(volatile u8*)(primitive + 0x25) = 0xdfu;
  func_8014e5a0(1u, 0x28u);

  primitive = BATTLE_GLOBAL_WORD_598C;
  func_8017a9b8(primitive);
  *(volatile u16*)(primitive + 0x16) = func_8017a620(0, 0, 0x3c0, 0);
  *(volatile u16*)(primitive + 0xe) = func_8017a6f0(0xb0, 0x1e0);
  *(volatile u8*)(primitive + 4) = 0x80u;
  *(volatile u8*)(primitive + 5) = 0x80u;
  *(volatile u8*)(primitive + 6) = 0x80u;
  arg0 += arg2 & 0xffu;
  x1 = arg0 + (arg3 & 0xffu);
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 0x18) = arg0;
  *(volatile s16*)(primitive + 0x1a) = arg1 + 7;
  *(volatile s16*)(primitive + 0x22) = arg1 + 7;
  *(volatile s16*)(primitive + 0x10) = x1;
  *(volatile s16*)(primitive + 0x20) = x1;
  *(volatile u8*)(primitive + 0xd) = 0xd8u;
  *(volatile u8*)(primitive + 0x15) = 0xd8u;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile s16*)(primitive + 0x12) = arg1;
  *(volatile u8*)(primitive + 0xc) = 0x98u;
  *(volatile u8*)(primitive + 0x14) = 0x9fu;
  *(volatile u8*)(primitive + 0x1c) = 0x98u;
  *(volatile u8*)(primitive + 0x1d) = 0xdfu;
  *(volatile u8*)(primitive + 0x24) = 0x9fu;
  *(volatile u8*)(primitive + 0x25) = 0xdfu;
  func_8014e5a0(1u, 0x28u);
}
