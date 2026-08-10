#include "bof3/context.h"
#include "bof3/core/slus_internal.h"

extern TILE* g_PrimCursor;

/* @behavior draws a full-screen 320x240 tile primitive whose gray shade is
 * the incoming fade value shifted down, then advances the fade value by the
 * given step and reports whether its 16-bit sign bit is set.
 * @source 0x8014F704
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 drawFadeTile(s16* value, s32 arg, u8 arg2, u8 arg3, u8 arg4) {
  TILE* tile;
  u16   next;
  u16   shade;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(1, arg4, 320, 0), NULL);
  appendRenderPrim(arg3, 0xC);
  tile = g_PrimCursor;
  SetTile(tile);
  shade = ((u16)*value) >> 7;
  tile->r0 = tile->g0 = tile->b0 = shade;
  tile->x0 = 0;
  tile->y0 = 0;
  tile->w = 320;
  tile->h = 240;
  SetSemiTrans(tile, arg2);
  appendRenderPrim(arg3, 0x10);
  next = (u16)(*value + arg);
  *value = next;
  return ((s16)next) < 0;
}
