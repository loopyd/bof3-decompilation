#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

extern u8 D_8014832E;   /* @source 0x8014832E @kind data */
extern u8 D_801F44D4[]; /* @source 0x801F44D4 @kind data */
extern u8 D_801F44D6[]; /* @source 0x801F44D6 @kind data */
extern u8 D_801F44D8[]; /* @source 0x801F44D8 @kind data */
extern u8 D_801F44DA[]; /* @source 0x801F44DA @kind data */
extern u8 D_801F44DC[]; /* @source 0x801F44DC @kind data */
extern u8 D_801F44DD[]; /* @source 0x801F44DD @kind data */
extern u8 D_801F44DE[]; /* @source 0x801F44DE @kind data */
extern u8 D_801F44DF[]; /* @source 0x801F44DF @kind data */
extern u8 D_801F44E0[]; /* @source 0x801F44E0 @kind data */
extern u8 D_801F454C;   /* @source 0x801F454C @kind data */

/* @source 0x801F2E80
 * @behavior Screen background of the area: when the shared graph gate byte
 *           0x8014832E holds bit 2 it first takes an eight-byte texture-window
 *           rectangle out of the shared primitive cursor, seeds it with the
 *           16x16 window 16,0,16,16 and arms the cursor's drawing mode with the
 *           graph-type page 0x229 on types 1 and 2 and 0x99 otherwise, links
 *           that 0xC-byte mode and fills one full-screen textured quad over
 *           0..256 in both axes whose texture covers 0..255, then links it and
 *           fills a second quad over x 256..320, y 0..256 sampling texture
 *           columns 0..0x40. It then takes a second window of 0,0,256,256 out
 *           of the cursor and arms another drawing mode with the same page
 *           choice, and walks the eight fourteen-byte records that start at
 *           0x801F44D4 until the byte cursor reaches 0x801F454C, drawing for
 *           each one a textured quad at the record's x/y with the record's
 *           width and height added along each axis, the passed vertical offset
 *           added to all four y values, the record's texture origin and extents
 *           as its four corner texels, a page built from the record's page byte
 *           as ((byte << 7) + 0x140) masked to 0x3FF, shifted down six and
 *           combined with 0x220 on graph types 1 and 2 and 0x90 otherwise, and
 *           a palette at (page byte + 0x1E5) << 6. Every quad is stamped with
 *           SetPolyFT4 and SetShadeTex(kind 1) and linked at ordering table
 *           index 7 with its own size. Takes the vertical offset and returns no
 *           value.
 * @status partial
 * @match 96.63
 * @residual The two fixed-quad texture pages use $a2 instead of $v0, and
 *           the second quad materializes and stores x=320 before x=256.
 *           The record loop matches with an independent byte offset and an
 *           explicit backedge. Measured under the owning compiler profile.
 */
void drawScrolledScreenBackgroundQuadRightColumnAndEightTiledBandsWorld04Area19913_801F2E80(
    s32 arg0) {
  RECT* win;
  POLY_FT4* prim;
  s32 cell;
  s32 i;

  if (!(D_8014832E & 0x4)) {
    return;
  }

  win = (RECT*)g_PrimCursor;
  g_PrimCursor = (u8*)win + 8;
  win->x = 0x10;
  win->y = 0;
  win->h = 0x10;
  win->w = 0x10;
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0,
              GetGraphType() == 1 ? 0x229
                                  : (GetGraphType() == 2 ? 0x229 : 0x99),
              win);
  func_8014E5A0(7, 0xC);

  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);
  SetShadeTex(prim, 1);
  prim->x1 = 0x100;
  prim->y2 = 0x100;
  prim->x3 = 0x100;
  prim->y3 = 0x100;
  prim->x0 = 0;
  prim->y0 = 0;
  prim->y1 = 0;
  prim->x2 = 0;
  prim->u0 = 0;
  prim->v0 = 0;
  prim->u1 = 0xFF;
  prim->v1 = 0;
  prim->u2 = 0;
  prim->v2 = 0xFF;
  prim->u3 = 0xFF;
  prim->v3 = 0xFF;
  prim->tpage =
      GetGraphType() == 1 ? 0x229 : (GetGraphType() == 2 ? 0x229 : 0x99);
  prim->clut = 0x79C0;
  func_8014E5A0(7, 0x28);

  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);
  SetShadeTex(prim, 1);
  prim->x1 = 0x140;
  prim->x3 = 0x140;
  prim->x0 = 0x100;
  prim->x2 = 0x100;
  prim->y2 = 0x100;
  prim->y3 = 0x100;
  prim->y0 = 0;
  prim->y1 = 0;
  prim->u0 = 0;
  prim->v0 = 0;
  prim->u1 = 0x40;
  prim->v1 = 0;
  prim->u2 = 0;
  prim->v2 = 0xFF;
  prim->u3 = 0x40;
  prim->v3 = 0xFF;
  prim->tpage =
      GetGraphType() == 1 ? 0x229 : (GetGraphType() == 2 ? 0x229 : 0x99);
  prim->clut = 0x79C0;
  func_8014E5A0(7, 0x28);

  win = (RECT*)g_PrimCursor;
  g_PrimCursor = (u8*)win + 8;
  win->x = 0;
  win->y = 0;
  win->h = 0x100;
  win->w = 0x100;
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0,
              GetGraphType() == 1 ? 0x229
                                  : (GetGraphType() == 2 ? 0x229 : 0x99),
              win);
  func_8014E5A0(7, 0xC);

  cell = (s32)D_801F44DC;
  i = 0;
next_band:
  {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 1);
    prim->x0 = *(u16*)(D_801F44D4 + i);
    prim->y0 = *(u16*)(D_801F44D6 + i) + arg0;
    prim->x1 = *(u16*)(D_801F44D4 + i) + *(u16*)(D_801F44D8 + i);
    prim->y1 = *(u16*)(D_801F44D6 + i) + arg0;
    prim->x2 = *(u16*)(D_801F44D4 + i);
    prim->y2 = *(u16*)(D_801F44D6 + i) + arg0 + *(u16*)(D_801F44DA + i);
    prim->x3 = *(u16*)(D_801F44D4 + i) + *(u16*)(D_801F44D8 + i);
    prim->y3 = *(u16*)(D_801F44D6 + i) + arg0 + *(u16*)(D_801F44DA + i);
    prim->u0 = D_801F44DD[i];
    prim->v0 = D_801F44DE[i];
    prim->u1 = D_801F44DD[i] + D_801F44DF[i];
    prim->v1 = D_801F44DE[i];
    prim->u2 = D_801F44DD[i];
    prim->v2 = D_801F44DE[i] + D_801F44E0[i];
    prim->u3 = D_801F44DD[i] + D_801F44DF[i];
    prim->v3 = D_801F44DE[i] + D_801F44E0[i];
    prim->tpage =
        GetGraphType() == 1
            ? ((((*(u8*)cell << 7) + 0x140) & 0x3FF) >> 6) | 0x220
            : (GetGraphType() == 2
                   ? ((((*(u8*)cell << 7) + 0x140) & 0x3FF) >> 6) | 0x220
                   : ((((*(u8*)cell << 7) + 0x140) & 0x3FF) >> 6) | 0x90);
    prim->clut = (*(u8*)cell + 0x1E5) << 6;
    func_8014E5A0(7, 0x28);
    cell += 0xE;
    i += 0xE;
  }
  if (cell < (s32)&D_801F454C) {
    goto next_band;
  }
}
