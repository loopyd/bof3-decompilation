#include "internal.h"

/* @behavior emits one fixed-size 16x8 marker primitive selected by the low byte of
 * the mode argument.
 * @source 0x801da078 FUN_801da078
 */
void func_801da078(s16 arg0, s16 arg1, s32 arg2) {
  u16 temp_v0;

  temp_v0 = func_8017a620(0, 1, 0x3c0, 0);
  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, temp_v0, 0);
  func_8014e5a0(1u, 0x0cu);
  {
    volatile u8* temp_a0;

    temp_a0 = (volatile u8*)BATTLE_GLOBAL_WORD_598C;
    *(volatile u16*)(temp_a0 + 0x10) = 0x10u;
    *(volatile u16*)(temp_a0 + 0x12) = 8u;
    *(volatile u16*)(temp_a0 + 0xe) = 0x7800u;
    temp_a0[4] = 0x80u;
    temp_a0[5] = 0x80u;
    temp_a0[6] = 0x80u;
    temp_a0[0xd] = 0xd8u;
    temp_a0[0xc] = (u8)arg2 << 4;
    *(volatile s16*)(temp_a0 + 8) = arg0;
    *(volatile s16*)(temp_a0 + 10) = arg1;
  }
  func_8017aa1c();
  func_8014e5a0(1u, 0x14u);
}
