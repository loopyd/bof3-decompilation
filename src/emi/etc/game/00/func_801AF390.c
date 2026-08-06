#include "internal.h"

extern void game_sprite_draw(s16 x, s16 y, u8 sprite_id, u8 flags);
extern void func_8014E5A0(u8 arg0, u8 arg1);

/* @behavior iterates a packed sprite-record table (3 bytes per entry: x-offset,
 * y-offset, sprite_id, terminated by sprite_id == 0xff), applies signed
 * offsets shifted by 3 to the base coordinates, and draws each sprite via
 * game_sprite_draw.
 * @source 0x801AF390
 */
void func_801AF390(s16 base_x, s16 base_y, const u8* record_table, u8 flags) {
  s16 x;
  s16 y;
  u8  sprite_id;
  u8  x_offset;
  u8  y_offset;
  u16 dord;
  u8* s0;

  if (flags & 1) {
    dord = GetTPage(0, 0, 832, 256);
  } else {
    dord = GetTPage(0, 0, 896, 256);
  }

  SetDrawMode((DR_MODE*)(*(u32*)0x8014598c), 0, 0, dord, 0);
  func_8014E5A0(1, 12);

  s0 = (u8*)record_table;

  if (s0[2] == 0xff) {
    return;
  }

  for (;;) {
    s0 = (u8*)record_table + 2;
    x_offset = s0[-2];
    y_offset = s0[-1];
    s0 += 3;

    x = base_x + (s16)(((s32)(s8)x_offset << 24) >> 21);
    y = base_y + (s16)(((s32)(s8)y_offset << 24) >> 21);

    sprite_id = s0[0];

    game_sprite_draw(x, y, sprite_id, flags & 0xff);

    if (sprite_id == 0xff) {
      break;
    }
  }
}
