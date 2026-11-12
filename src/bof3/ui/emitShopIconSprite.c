#include "bof3/ui/shop00_internal.h"

/* Graphics-type query of the SDK map (0x8017B2B4): it reports the display
 * graph type the current frame is drawn for. This emitter queries it for type 1
 * and again for type 2 to pick the draw-mode texture page, so the second query
 * runs only when the first did not answer 1.
 * @source 0x8017B2B4 @kind function */
extern s32 GetGraphType(void);

/* @source 0x801D8FC0
 * @behavior shop icon sprite emitter of this EMI, called by the shop icon
 *           wrapper func_801D910C with two coordinates, an icon kind byte and a
 *           mode byte: it first picks the draw-mode texture page from
 *           GetGraphType, taking 0x2E when the first query answers 1, 0x2E when
 *           a second query answers 2 and 0x1E otherwise, then arms SetDrawMode
 *           at g_PrimCursor with dithering enabled, no texture disable and no
 *           texture window, appends that 0xC-byte draw-mode packet at
 *           ordering-table slot 1 and initialises the 0x14-byte SPRT packet at
 *           the advanced cursor through SetSprt. It then picks the three colour
 *           bytes of the sprite from the mode byte (0 makes all three 0x80, 1
 *           makes them 0x30, any other value gives red 0x40, green 0x40 and
 *           blue 0x80), stores the two coordinates at 0x8 and 0xA, and takes
 *           the entry of the four-byte-stride icon tile table D_801E5850 that
 *           the kind byte indexes for the texture byte pair at 0xC and 0xD.
 *           It fixes the sprite size at 0x28 by 0x30 and assembles the clut
 *           word at 0xE as ((clutY + 0x1E0) << 6) | (clutX >> 4) from the last
 *           two bytes of that entry, then appends the sprite at slot 1 as a
 *           0x14-byte primitive. The 0x30-byte frame keeps $s0 through $s4 and
 *           $ra across those calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitShopIconSprite(s32 x, s32 y, s32 kind, s32 mode) {
  SPRT* sprite;
  ShopIconTile* tile;
  u32 colorMode;
  s32 tpage;

  tile = &D_801E5850[kind & 0xFF];

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

  colorMode = mode & 0xFF;
  if (colorMode == 0) {
    sprite->b0 = 0x80;
    sprite->g0 = 0x80;
    sprite->r0 = 0x80;
  } else if (colorMode == 1) {
    sprite->b0 = 0x30;
    sprite->g0 = 0x30;
    sprite->r0 = 0x30;
  } else {
    sprite->g0 = 0x40;
    sprite->r0 = 0x40;
    sprite->b0 = 0x80;
  }

  sprite->x0 = x;
  sprite->y0 = y;
  sprite->u0 = tile->u;
  sprite->v0 = tile->v;
  sprite->clut = ((tile->clutY + 0x1E0) << 6) | (tile->clutX >> 4);
  sprite->w = 0x28;
  sprite->h = 0x30;
  appendRenderPrim(1, 0x14);
}
