#include "bof3/world/area03004_internal.h"

/* @behavior configures the requested AREA030 sprite and its CLUT entry after
 * selecting graphics mode 1.
 * @source 0x801E0F4C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void configureSpriteClut(s16 arg0, s16 arg1, u8 arg2) {
  u16 clut;
  u32 sprite_index;
  u8* sprite;

  submitTpageDrawMode(1, 1);
  sprite = func_801E0DCC(2, 1, arg0, arg1);
  sprite_index = arg2 & 0xff;
  if (sprite_index != 0xff) {
    /* MATCHING_AID: retain the original CLUT construction register order. */
    clut = (u16)(((arg2 >> 4) + 0x1eb) << 6);
    clut |= arg2 & 0x0f;
    *(u16*)(sprite + 0x0e) = clut;
    sprite[0x0c] = (u8)((sprite_index & 3) << 6);
    sprite[0x0d] = (u8)((sprite_index >> 2) * 0x28);
  }
}
