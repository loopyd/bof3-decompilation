#include "internal.h"

/* does: builds one two-corner primitive using the sprite offset table selected
 * by the shape byte and the palette-derived rgb triple selected by `arg3`.
 * @source: 0x801d9c80 FUN_801d9c80
 */
void func_801d9c80(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  u32 primitive;
  u32 table_offset;

  primitive = BATTLE_GLOBAL_WORD_598C;
  func_8017aa6c(primitive);

  table_offset = ((u32)arg2 & 0xffu) * 4u;
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile s16*)(primitive + 12) =
      *(const volatile u16*)(0x801eae50u + table_offset);
  *(volatile s16*)(primitive + 14) =
      *(const volatile u16*)(0x801eae52u + table_offset);

  arg3 = (u32)arg3 & 0xffu;
  *(volatile u8*)(primitive + 4) =
      (*(volatile u16*)(0x80033a08u +
                        (((u32)(BATTLE_GLOBAL_BYTE_4952 * 2) + (u32)arg3) *
                         0x20u)) &
       0x1fu)
      << 3;
  *(volatile u8*)(primitive + 5) =
      (*(volatile u16*)(0x80033a08u +
                        (((u32)(BATTLE_GLOBAL_BYTE_4952 * 2) + (u32)arg3) *
                         0x20u)) >>
       2) &
      0xf8u;
  *(volatile u8*)(primitive + 6) =
      (*(volatile u16*)(0x80033a08u +
                        (((u32)(BATTLE_GLOBAL_BYTE_4952 * 2) + (u32)arg3) *
                         0x20u)) >>
       7) &
      0xf8u;

  func_8017a904(primitive, arg3);
  func_8014e5a0(1u, 0x10u);
}
