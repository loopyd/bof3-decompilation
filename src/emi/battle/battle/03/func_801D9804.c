#include "internal.h"

/* @behavior emits one 24x8 numeric/icon strip selected by the low byte of `arg3`
 * and palette slot `arg2`.
 * @source 0x801D9804
 */
void func_801D9804(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  u32 packet;

  SetDrawMode((DR_MODE*)BATTLE_GLOBAL_WORD_598C, 0, 0, GetTPage(0, 0, 0x3c0, 0),
                0);
  func_8014E5A0(1, 0xc);

  packet = BATTLE_GLOBAL_WORD_598C;
  *(volatile u16*)(packet + 0xe) = GetClut((arg2 & 0xff) << 4, 0x1e0);
  *(volatile u8*)(packet + 4) = 0x80;
  *(volatile u8*)(packet + 5) = 0x80;
  *(volatile u8*)(packet + 6) = 0x80;
  *(volatile u8*)(packet + 0xc) = (u8)(((arg3 & 0xff) * 0x18) + 0x68);
  *(volatile u8*)(packet + 0xd) = 0xd0;
  *(volatile u16*)(packet + 0x10) = 0x18;
  *(volatile s16*)(packet + 8) = arg0;
  *(volatile s16*)(packet + 10) = arg1;
  *(volatile u16*)(packet + 0x12) = 8;
  ((void (*)(volatile void*))SetSprt)((volatile void*)packet);
  func_8014E5A0(1, 0x14);
}
