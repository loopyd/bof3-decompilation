#include "internal.h"

/* @behavior formats a short decimal string into the shared UI buffer, then emits a
 * run of 8x8 glyphs using palette slot `arg2` and the template selected by
 * `arg3`.
 * @source 0x801d9684 FUN_801d9684
 */
void func_801d9684(s16 arg0, u16 arg1, s32 arg2, u16 arg3) {
  u8 index;

  index = 0u;
  func_8017e3f4((void*)BATTLE_UI_CHAR_BUFFER, (const void*)0x801d0c78u, arg3);
  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, func_8017a620(0, 0, 0x3c0, 0),
                0);
  func_8014e5a0(1u, 0x0cu);

  while (BATTLE_UI_CHAR_BUFFER[index] != 0u) {
    if (BATTLE_UI_CHAR_BUFFER[index] != ' ') {
      BATTLE_UI_CHAR_BUFFER[index] -= 0x30u;
      *(volatile u16*)(BATTLE_GLOBAL_WORD_598C + 0xe) =
          func_8017a6f0((arg2 & 0xff) << 4, 0x1e0);
      *(volatile u8*)(BATTLE_GLOBAL_WORD_598C + 4) = 0x80u;
      *(volatile u8*)(BATTLE_GLOBAL_WORD_598C + 5) = 0x80u;
      *(volatile u8*)(BATTLE_GLOBAL_WORD_598C + 6) = 0x80u;
      *(volatile s16*)(BATTLE_GLOBAL_WORD_598C + 8) = arg0;
      *(volatile u16*)(BATTLE_GLOBAL_WORD_598C + 10) = arg1;
      *(volatile u8*)(BATTLE_GLOBAL_WORD_598C + 0xd) = 0xd0u;
      *(volatile u16*)(BATTLE_GLOBAL_WORD_598C + 0x10) = 8u;
      *(volatile u16*)(BATTLE_GLOBAL_WORD_598C + 0x12) = 8u;
      *(volatile u8*)(BATTLE_GLOBAL_WORD_598C + 0xc) =
          (BATTLE_UI_CHAR_BUFFER[index] * 8u) - 0x50u;
      func_8017aa1c();
      func_8014e5a0(1u, 0x14u);
    }
    arg0 += 8;
    index += 1u;
  }
}
