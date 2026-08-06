#include "internal.h"

extern u8*  game_sprite_rect_entry(u8 sprite_id, u8 flags);
extern void func_8014E5A0(u8 arg0, u8 arg1);

/* @behavior draws one sprite by filling a GT quad primitive from a rect-table
 * entry, selecting CLUT by the bit-1 flag, then appending to the OT.
 * @source 0x801AF2A0
 */
void game_sprite_draw(s16 x, s16 y, u8 sprite_id, u8 flags) {
  u8* rect;
  u8* packet;

  rect = game_sprite_rect_entry(sprite_id, flags & 1);
  packet = (u8*)*(u32*)0x8014598c;

  SetSprt((SPRT*)packet);
  SetSemiTrans((void*)packet, 0);

  *(s16*)(packet + 8) = x;
  *(s16*)(packet + 10) = y;

  packet[12] = rect[0];
  packet[13] = rect[1];
  *(s16*)(packet + 16) = rect[2];
  *(s16*)(packet + 18) = rect[3];

  if (!(flags & 2)) {
    *(u16*)(packet + 14) = GetClut(176, 481);
  } else {
    *(u16*)(packet + 14) = GetClut(128, 482);
  }

  packet[4] = 128;
  packet[5] = 128;
  packet[6] = 128;
  func_8014E5A0(1, 20);
}
