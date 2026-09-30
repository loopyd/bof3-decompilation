#include "bof3/world/area02414_internal.h"

/* @behavior projects one local sprite work entry and queues the matching local
 * effect. The projected point and the quad size pair live in the first two entries
 * of the address-taken local screen-point workspace whose four 8-byte entries
 * reproduce the original 0x40-byte frame: the shared projection helper
 * func_801AFF04 fills the first entry, the record's `+0x24` halfword is written
 * twice into the second entry as the size, and func_801AFFD8 converts that pair in
 * place. The translucent FT4 is centered on the projected point with the half size
 * derived from `+0x24` as its offset (the far edges carry the full size), textured
 * with one 32x32 texel cell at (0xe0, 0x30), coloured from the record's `+0x03`
 * byte and handed to func_80155A08 at 0x28 bytes.
 * @source 0x801F2DF8
 * @status partial
 * @match 89.08
 * @residual live audit: 106/119 instructions, 476 original bytes versus 472 current. Residual class is allocator/scheduler order inside two call-free regions: the func_801AFFD8 argument address (&screen[1]) is materialised one slot before the first size read where the original keeps it in that load's delay slot and reuses $v0 for both reads, and the tail keeps the GetTPage result plus the colour byte in $v0/$v1 swapped (the original stores tpage straight after the call and reuses its nop-filled colour load-delay pair, one instruction longer). Measured clean-C shapes: work-pointer volatility view, screen-workspace volatility, pointer and alias locals, separate colour locals, statement order, and the SDK setUV4/setUVWH uv forms; the profile and permuter rungs are opt-in and untried.
 */
void func_801F2DF8(const void* arg0) {
  World00Area024ScreenPoint       screen[4];
  const volatile World00Area024SpriteWork* work;
  const void*                     object;
  POLY_FT4*                       prim;
  u8                              color;

  work = (const World00Area024SpriteWork*)arg0;
  object = (const void*)((const u8*)arg0 + 4u);
  prim = (POLY_FT4*)(u32)WORLD00_AREA024_PRIMITIVE_PTR;

  SetPolyFT4(prim);
  SetSemiTrans(prim, 1);

  func_801AFF04(object, &screen[0]);
  screen[1].x = work->field_24;
  screen[1].y = work->field_24;
  func_801AFFD8(object, &screen[1], &screen[1]);

  prim->x0 = screen[0].x - ((s16)screen[1].x >> 1);
  prim->y0 = screen[0].y - ((s16)screen[1].y >> 1);
  prim->x1 = (screen[0].x - ((s16)screen[1].x >> 1)) + (s16)screen[1].x;
  prim->y1 = screen[0].y - ((s16)screen[1].y >> 1);
  prim->x2 = screen[0].x - ((s16)screen[1].x >> 1);
  prim->y2 = (screen[0].y - ((s16)screen[1].y >> 1)) + (s16)screen[1].y;
  prim->x3 = (screen[0].x - ((s16)screen[1].x >> 1)) + (s16)screen[1].x;
  prim->y3 = (screen[0].y - ((s16)screen[1].y >> 1)) + (s16)screen[1].y;
  setUVWH(prim, 0xe0, 0x30, 0x1f, 0x1f);
  prim->clut = GetClut(0xa0, 0x1e3);
  prim->tpage = GetTPage(0, 1, 0x2c0, 0x100);
  color = work->field_03;
  prim->r0 = color;
  color = work->field_03;
  prim->g0 = color;
  color = work->field_03;
  prim->b0 = color;

  func_80155A08(work->field_04, work->field_08, 2, 0x28);
}
