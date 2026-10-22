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
 * @status partial
 * @match 86.15
 * @residual same-size residual (260/260 bytes, 65/65 instructions): the
 * callee-saved roles are rotated. The original keeps the record walk in $s1,
 * the id cursor in $s0, the flag word in $s3, the 0xFF loop bound in $s2 and
 * masks the incoming flag argument in place ($a3); this build keeps the flag
 * word in $s2 and the bound in $s3 and masks the copied flag word through
 * $v0. Nine instructions differ, all in the prologue copy/save placement, the
 * page-select mask register and the loop-head scheduling. Measured neutral or
 * worse clean-C shapes: declaration order, s32/u32 flag, page and offset
 * widths, ternary page select, named terminator byte, staged word shifts,
 * const and non-const record walks, entry-pointer copy versus walking the
 * record_table parameter. Assembler profile, permuter and flag-search rungs
 * stay parent-owned.
 */
void drawSpriteRecordTable(s16 base_x, s16 base_y, const u8* record_table, u32 flags) {
  u16  dord;
  s32  tpage;
  u8   sprite_id;
  u8   x_offset;
  u8   y_offset;
  const u8* cursor;
  s16  x;
  s16  y;

  tpage = 0x340;
  if ((flags & 1) == 0) {
    tpage = 0x380;
  }
  dord = GetTPage(0, 0, tpage, 0x100);
  SetDrawMode((DR_MODE*)(*(u32*)0x8014598c), 0, 0, dord, 0);
  func_8014E5A0(1, 12);

  sprite_id = record_table[2];
  if (sprite_id == 0xff) {
    return;
  }

  cursor = record_table + 2;
  do {
    x_offset = record_table[0];
    y_offset = cursor[-1];
    cursor += 3;
    x = (s16)(base_x + (((s32)x_offset << 24) >> 21));
    y = (s16)(base_y + (((s32)y_offset << 24) >> 21));
    drawSprite(x, y, sprite_id, flags & 0xff);
    sprite_id = cursor[0];
    record_table += 3;
  } while (sprite_id != 0xff);
}
