#include "bof3/ui/game00_internal.h"

extern void drawSprite(s16 x, s16 y, u8 sprite_id, u8 flags);
extern void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801AF390
 * @behavior sets the draw mode for the sprite page selected by the low flag
 * bit (0x340 when that bit is set, 0x380 when it is clear), appends 12 bytes of
 * OT work, then iterates a packed sprite-record table (3 bytes per entry:
 * x-offset, y-offset, sprite_id, terminated by sprite_id == 0xff), applies
 * signed offsets shifted by 3 to the base coordinates, and draws each sprite
 * via drawSprite.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpriteRecordTable(s16 base_x, s16 base_y, const u8* record_table, u32 flags) {
  u8   sprite_flags;
  u16  dord;
  s32  tpage;
  u8   sprite_id;
  u8   x_offset;
  u8   y_offset;
  s16  x;
  s16  y;

  tpage = 0x340;
  sprite_flags = flags;
  flags &= 1;
  if (flags == 0) {
    tpage = 0x380;
  }
  dord = GetTPage(0, 0, tpage, 0x100);
  SetDrawMode((DR_MODE*)(*(u32*)0x8014598c), 0, 0, dord, 0);
  func_8014E5A0(1, 12);

  do {
    for (;;) {
      sprite_id = record_table[2];
      if (sprite_id == 0xff) {
        break;
      }
      x_offset = record_table[0];
      y_offset = record_table[1];
      x = (s16)(base_x + ((s32)((u32)x_offset << 24) >> 21));
      y = (s16)(base_y + ((s32)((u32)y_offset << 24) >> 21));
      drawSprite(x, y, sprite_id, sprite_flags);
      record_table += 3;
    }
  } while (0);
}
