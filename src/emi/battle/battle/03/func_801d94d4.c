#include "internal.h"

/* @behavior formats a short decimal string into the shared UI buffer, then emits a
 * run of 6x5 glyphs using palette slot `arg2` and the alternate template chosen
 * by `arg3`.
 * @source 0x801D94D4
 */
void func_801D94D4(s16 arg0, u16 arg1, s32 arg2, s16 arg3) {
  u8  index;
  u8  temp;
  u8* packet;
  u16 value;
  u16 arg3_u16;

  index = 0;
  arg3_u16 = (u16)arg3;
  if (arg3_u16 == 0xffff) {
    func_8017E3F4((void*)((u8*)0x80140000 + 0x5ad4),
                  (const void*)((u8*)0x801d0000 + 0xc70));
  } else {
    func_8017E3F4((void*)((u8*)0x80140000 + 0x5ad4),
                  (const void*)((u8*)0x801d0000 + 0xc74), arg3_u16);
  }
  func_8017C2D8(*(u32*)0x8014598c, 0, 0, func_8017A620(0, 0, 0x3c0, 0), 0);
  func_8014E5A0(1, 0xc);

  for (;;) {
    temp = *(u8*)((u32)index + (0x80140000 + 0x5ad4));
    if (temp == 0) {
      break;
    }

    if (temp != ' ') {
      *(u8*)((u32)index + (0x80140000 + 0x5ad4)) = temp - 0x30;
      packet = (u8*)*(u32*)0x8014598c;
      value = func_8017A6F0((arg2 & 0xff) << 4, 0x1e0);
      *(u16*)(packet + 0xe) = value;
      packet[4] = 0x80;
      packet[5] = 0x80;
      packet[6] = 0x80;
      *(s16*)(packet + 8) = arg0;
      *(u16*)(packet + 0xa) = arg1;
      temp = *(u8*)((u32)index + (0x80140000 + 0x5ad4));
      packet[0xd] = 0xd8;
      *(u16*)(packet + 0x10) = 6;
      *(u16*)(packet + 0x12) = 5;
      packet[0xc] = (temp * 6) - 0x50;
      func_8017AA1C();
      func_8014E5A0(1, 0x14);
    }

    arg0 += 5;
    index += 1;
  }
}
