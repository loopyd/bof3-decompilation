#include "internal.h"

void func_801af2a0(s16 x, s16 y, u8 sprite_id, u8 flags);
u16  func_8017a620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017c2d8(u32 arg0, s32 arg1, s32 arg2, u16 arg3, s32 arg4);
void func_8014e5a0(u8 arg0, u8 arg1);

/* does: iterates a packed sprite-record table (3 bytes per entry: x-offset,
 * y-offset, sprite_id, terminated by sprite_id == 0xff), applies signed
 * offsets shifted by 3 to the base coordinates, and draws each sprite via
 * func_801af2a0.
 * @source: 0x801af390 FUN_801af390
 */
void func_801af390(s16 base_x, s16 base_y, const u8* record_table, u8 flags) {
  s16 x;
  s16 y;
  u8  sprite_id;
  u8  x_offset;
  u8  y_offset;
  u16 dord;
  u8* s0;

  if (flags & 1) {
    dord = func_8017a620(0, 0, 832, 256);
  } else {
    dord = func_8017a620(0, 0, 896, 256);
  }

  func_8017c2d8(*(u32*)0x8014598c, 0, 0, dord, 0);
  func_8014e5a0(1, 12);

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

    func_801af2a0(x, y, sprite_id, flags & 0xff);

    if (sprite_id == 0xff) {
      break;
    }
  }
}
