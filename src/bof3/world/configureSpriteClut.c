#include "bof3/world/area03004_internal.h"

/* @behavior Select draw mode 1/1 and draw sprite 2 at x and y. Unless packedSelector is
 * 0xFF, derive its CLUT halfword and UV bytes from the selector bits.
 * @source 0x801E0F4C
 * @status exact
 * @match 100.00
 * @residual none
 */
void configureSpriteClut(s16 x, s16 y, u8 packedSelector) {
  u16 clut;
  u32 sprite_index;
  u8* sprite;

  submitTpageDrawMode(1, 1);
  sprite = appendIndexedSpriteWorld00Area03004(2, 1, x, y);
  sprite_index = packedSelector & 0xff;
  if (sprite_index != 0xff) {
    clut = (u16)(((packedSelector >> 4) + 0x1eb) << 6);
    clut |= packedSelector & 0x0f;
    *(u16*)(sprite + 0x0e) = clut;
    sprite[0x0c] = (u8)((sprite_index & 3) << 6);
    sprite[0x0d] = (u8)((sprite_index >> 2) * 0x28);
  }
}
