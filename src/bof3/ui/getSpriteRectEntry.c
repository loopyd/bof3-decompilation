#include "bof3/ui/game00_internal.h"

/* @kind: table */
extern u8 spriteRectTable[];
/* @kind: table */
extern u8 spriteRectTableAlt[];

/* @behavior returns one four-byte sprite record from the table selected by
 * flags, indexed by the low byte of the sprite id.
 * @source 0x801AF270
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8* getSpriteRectEntry(u8 sprite_id, u8 flags) {
  if (flags & 0xff)
    return spriteRectTableAlt + (sprite_id & 0xff) * 4;
  return spriteRectTable + (sprite_id & 0xff) * 4;
}
