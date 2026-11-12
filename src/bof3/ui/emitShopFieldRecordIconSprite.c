#include "bof3/ui/shop00_internal.h"

/* Graphics-type query of the SDK map (0x8017B2B4): it reports the display
 * graph type the current frame is drawn for. This emitter queries it for type 1
 * and again for type 2 to pick the draw-mode texture page, so the second query
 * runs only when the first did not answer 1.
 * @source 0x8017B2B4 @kind function */
extern s32 GetGraphType(void);

/* @source 0x801E4FC8
 * @behavior shop field-record icon sprite emitter of this EMI, called only by
 *           the panel emitter func_801E4E9C once per field record whose flag
 *           byte is set and whose byte at +0x16 matches D_80148361, with the
 *           task's x/field-6 coordinates plus that walk's running icon offset,
 *           the record byte at +0 as the icon kind and the mode byte 0. It
 *           first substitutes the kind 0xB when the kind byte is 4 and the
 *           signed main-RAM byte D_80146870 is at least 8, then picks the
 *           draw-mode texture page from GetGraphType (0x2E when the first query
 *           answers 1, 0x2E when a second query answers 2, 0x1E otherwise),
 *           arms SetDrawMode at g_PrimCursor with dithering enabled, no texture
 *           disable and no texture window, appends that 0xC-byte draw-mode
 *           packet at ordering-table slot 1 and initialises the 0x14-byte SPRT
 *           packet at the advanced cursor through SetSprt. It then picks the
 *           three colour bytes of the sprite from the mode byte (0 makes all
 *           three 0x80, 1 makes them 0x30, any other value gives green 0x40,
 *           red 0x40 and blue 0x80), stores the two coordinates at 0x8 and 0xA,
 *           and takes the entry of the four-byte-stride icon tile table
 *           D_801E600C that the kind byte indexes for the texture byte pair at
 *           0xC and 0xD, stored offset by 3 and 5. It fixes the sprite size at
 *           0x24 by 0x28 and assembles the clut word at 0xE as
 *           ((clutY + 0x1E0) << 6) | (clutX >> 4) from the last two bytes of
 *           that entry, then appends the sprite at slot 1 as a 0x14-byte
 *           primitive. The 0x30-byte frame keeps $s0 through $s4 and $ra across
 *           those calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitShopFieldRecordIconSprite(u16 arg0, u16 arg1, u8 arg2, u8 arg3) {
  SPRT* sprite;
  ShopIconTile* tile;
  s32 tpage;

  if (arg2 == 4 && D_80146870 >= 8) {
    arg2 = 0xB;
  }
  tile = &D_801E600C[arg2];

  if (GetGraphType() == 1) {
    tpage = 0x2E;
  } else if (GetGraphType() == 2) {
    tpage = 0x2E;
  } else {
    tpage = 0x1E;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  appendRenderPrim(1, 0xC);

  sprite = (SPRT*)g_PrimCursor;
  SetSprt(sprite);

  if (arg3 == 0) {
    sprite->b0 = 0x80;
    sprite->g0 = 0x80;
    sprite->r0 = 0x80;
  } else if (arg3 == 1) {
    sprite->b0 = 0x30;
    sprite->g0 = 0x30;
    sprite->r0 = 0x30;
  } else {
    sprite->g0 = 0x40;
    sprite->r0 = 0x40;
    sprite->b0 = 0x80;
  }

  sprite->x0 = arg0;
  sprite->y0 = arg1;
  sprite->u0 = tile->u + 3;
  sprite->v0 = tile->v + 5;
  sprite->clut = ((tile->clutY + 0x1E0) << 6) | (tile->clutX >> 4);
  sprite->w = 0x24;
  sprite->h = 0x28;
  appendRenderPrim(1, 0x14);
}
