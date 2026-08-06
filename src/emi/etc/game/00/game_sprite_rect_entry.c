#include "internal.h"

/* @kind: table */
extern u8 game_sprite_rectTable[];
/* @kind: table */
extern u8 game_sprite_rectTable_alt[];

/* @behavior returns one four-byte sprite record from the table selected by
 * flags, indexed by the low byte of the sprite id.
 * @source 0x801AF270
 */
u8* game_sprite_rect_entry(u8 sprite_id, u8 flags) {
  if (flags & 0xff)
    return game_sprite_rectTable_alt + (sprite_id & 0xff) * 4;
  return game_sprite_rectTable + (sprite_id & 0xff) * 4;
}
