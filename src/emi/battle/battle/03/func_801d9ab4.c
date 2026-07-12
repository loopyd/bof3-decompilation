#include "internal.h"

/* @behavior builds one four-corner primitive using the offset table selected by the
 * shape byte and the palette-derived rgb triple selected by `arg3`.
 * @source 0x801d9ab4 FUN_801d9ab4
 */
void func_801d9ab4(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  u32 primitive;
  u32 table_offset;

  primitive = BATTLE_GLOBAL_WORD_598C;
  func_8017a9a4(primitive);

  table_offset = ((u32)arg2 & 0xffu) << 4;
  *(volatile s16*)(primitive + 8) =
      arg0 + *(volatile u16*)(0x801f0000u + table_offset - 0x52d0u);
  *(volatile s16*)(primitive + 10) =
      arg1 + *(volatile u16*)(0x801f0000u + table_offset - 0x52ceu);
  *(volatile s16*)(primitive + 12) =
      arg0 + *(volatile u16*)(0x801f0000u + table_offset - 0x52ccu);
  *(volatile s16*)(primitive + 14) =
      arg1 + *(volatile u16*)(0x801f0000u + table_offset - 0x52cau);
  *(volatile s16*)(primitive + 16) =
      arg0 + *(volatile u16*)(0x801f0000u + table_offset - 0x52c8u);
  *(volatile s16*)(primitive + 18) =
      arg1 + *(volatile u16*)(0x801f0000u + table_offset - 0x52c6u);
  *(volatile s16*)(primitive + 20) =
      arg0 + *(volatile u16*)(0x801f0000u + table_offset - 0x52c4u);
  *(volatile s16*)(primitive + 22) =
      arg1 + *(volatile u16*)(0x801f0000u + table_offset - 0x52c2u);

  arg3 = (u32)arg3 & 0xffu;
  *(volatile u8*)(primitive + 4) =
      (*(volatile u16*)(0x80030000u +
                        ((((u32)(BATTLE_GLOBAL_BYTE_4952 * 2) + (u32)arg3)
                          << 5) +
                         0x3a08u)) &
       0x1fu)
      << 3;
  *(volatile u8*)(primitive + 5) =
      (*(volatile u16*)(0x80030000u +
                        ((((u32)(BATTLE_GLOBAL_BYTE_4952 * 2) + (u32)arg3)
                          << 5) +
                         0x3a08u)) >>
       2) &
      0xf8u;
  *(volatile u8*)(primitive + 6) =
      (*(volatile u16*)(0x80030000u +
                        ((((u32)(BATTLE_GLOBAL_BYTE_4952 * 2) + (u32)arg3)
                          << 5) +
                         0x3a08u)) >>
       7) &
      0xf8u;

  func_8017a904(primitive, arg3);
  func_8014e5a0(1u, 0x18u);
}
