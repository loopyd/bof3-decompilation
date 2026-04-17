#include "internal.h"

/* does: emits one colored sprite primitive using the sprite offset table and
 * the three scratch rgb bytes captured in scratchpad.
 * @source: 0x801d99ac FUN_801d99ac
 */
void func_801d99ac(s16 arg0, s16 arg1, s32 arg2) {
  u16 primitive_id;
  u32 primitive;
  u32 table_offset;

  primitive_id = func_8017a620(0, 2, 0x3c0, 0);
  func_8017c2d8(BOF3_BATTLE_GLOBAL_WORD_598C, 0, 0, primitive_id, 0);
  func_8014e5a0(1u, 0x0cu);
  primitive = BOF3_BATTLE_GLOBAL_WORD_598C;
  func_8017aa6c(primitive);
  table_offset = ((u32)arg2 & 0xffu) << 2;
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile u16*)(primitive + 12) =
      *(const volatile u16*)(0x801f0000u + table_offset - 0x51b0u);
  *(volatile u16*)(primitive + 14) =
      *(const volatile u16*)(0x801f0000u + table_offset - 0x51aeu);
  *(volatile u8*)(primitive + 4) = BOF3_BATTLE_SCRATCH_BYTE_000;
  *(volatile u8*)(primitive + 5) = BOF3_BATTLE_SCRATCH_BYTE_001;
  *(volatile u8*)(primitive + 6) = BOF3_BATTLE_SCRATCH_BYTE_002;
  func_8017a904(primitive, 1);
  func_8014e5a0(1u, 0x10u);
}
