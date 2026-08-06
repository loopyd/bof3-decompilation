#include "bof3/context.h"
#include "internal.h"

extern TILE* D_8014598C;

/* @behavior draws a full-screen 320x240 tile primitive whose gray shade is
 * the incoming fade value shifted down, then advances the fade value by the
 * given step and reports whether its 16-bit sign bit is set.
 * @source 0x8014F704
 */
u8 game_fade_draw_tile(s16* value, s32 arg, u8 arg2, u8 arg3, u8 arg4) {
  TILE* tile;
  u16   next;
  u16   shade;

  SetDrawMode((DR_MODE*)D_8014598C, 0, 0, GetTPage(1, arg4, 320, 0), NULL);
  render_append_prim(arg3, 0xC);
  tile = D_8014598C;
  SetTile(tile);
  shade = ((u16)*value) >> 7;
  tile->r0 = tile->g0 = tile->b0 = shade;
  tile->x0 = 0;
  tile->y0 = 0;
  tile->w = 320;
  tile->h = 240;
  SetSemiTrans(tile, arg2);
  render_append_prim(arg3, 0x10);
  next = (u16)(*value + arg);
  *value = next;
  return ((s16)next) < 0;
}
