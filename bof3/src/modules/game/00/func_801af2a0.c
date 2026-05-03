#include "internal.h"

u8*  func_801af270(u8 sprite_id, u8 flags);
void func_8017aa1c(void);
void func_8017a904(u32 arg0, s32 arg1);
u16  func_8017a6f0(s32 arg0, s32 arg1);
void func_8014e5a0(u8 arg0, u8 arg1);

/* does: draws one sprite by filling a GT quad primitive from a rect-table
 * entry, selecting CLUT by the bit-1 flag, then appending to the OT.
 * @source: 0x801af2a0 FUN_801af2a0
 */
void func_801af2a0(s16 x, s16 y, u8 sprite_id, u8 flags) {
  u8*  rect;
  u8*  packet;

  rect = func_801af270(sprite_id, flags & 1);
  packet = (u8*)*(u32*)0x8014598c;

  func_8017aa1c();
  func_8017a904((u32)packet, 0);

  *(s16*)(packet + 8) = x;
  *(s16*)(packet + 10) = y;

  packet[12] = rect[0];
  packet[13] = rect[1];
  *(s16*)(packet + 16) = *(s16*)&rect[2];
  *(s16*)(packet + 18) = *(s16*)&rect[4];

  if (flags & 2) {
    *(u16*)(packet + 14) = func_8017a6f0(176, 481);
  } else {
    *(u16*)(packet + 14) = func_8017a6f0(128, 482);
  }

  packet[4] = 128;
  packet[5] = 128;
  packet[6] = 128;
  func_8014e5a0(1, 20);
}
