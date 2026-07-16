#include "internal.h"

extern u8 D_801CCE84[];
extern u8 D_801CCF7C[];

/* @behavior returns one four-byte sprite record from the table selected by
 * flags, indexed by the low byte of the sprite id.
 * @source 0x801AF270
 */
u8* func_801AF270(u8 sprite_id, u8 flags) {
  if (flags & 0xff)
    return D_801CCF7C + (sprite_id & 0xff) * 4;
  return D_801CCE84 + (sprite_id & 0xff) * 4;
}
