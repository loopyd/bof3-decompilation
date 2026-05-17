#include "internal.h"

/* does: emits one colored sprite primitive using the sprite offset table and a
 * packed 15-bit color argument.
 * @source: 0x801d9dbc FUN_801d9dbc
 */
void func_801d9dbc(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4) {
  u16 sprite_start;
  u16 sprite_offset;
  u32 primitive;
  u32 table_offset;
  u32 color;

  primitive = BATTLE_GLOBAL_WORD_598C;
  func_8017aa6c(primitive);
  table_offset = ((u32)arg2 & 0xffu) * 4;
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  sprite_start = *(const volatile u16*)(0x801eae50u + table_offset);
  *(volatile u16*)(primitive + 12) = sprite_start;
  color = (u32)arg3 & 0xffffu;
  sprite_offset = *(const volatile u16*)(0x801eae52u + table_offset);
  *(volatile u8*)(primitive + 4) = (color >> 7) & 0xf8u;
  *(volatile u8*)(primitive + 5) = (color >> 2) & 0xf8u;
  *(volatile u8*)(primitive + 6) = ((u32)arg3 & 0x1fu) << 3;
  *(volatile u16*)(primitive + 14) = sprite_offset;
  func_8017a904(primitive, arg4);
  func_8014e5a0(1u, 0x10u);
}
