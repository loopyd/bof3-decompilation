#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x801EF47C
 * @behavior Appends two primitives at g_PrimCursor under the 0x3c0 texture
 *           page: first, at ordering-table index 1, an opaque black 0xa8 x 0x18
 *           tile at (x + 0x20, 0x30), the same 168x24 panel rectangle the
 *           sibling emitTintedPanelSprite fills at (0x4c, 0x30); then, also at
 *           index 1, a 0x22 x 0x18 Gouraud quad spanning (x, 0x30) to
 *           (x + 0x22, 0x48) whose left corners are black and right corners
 *           0xff/0xff/0xff, marked semi-transparent, so the quad ramps from
 *           black to white across the panel's left edge; the second draw mode
 *           selects the same page with semi-transparency mode 2. The caller
 *           func_801EEF10 passes the battle halfword D_801463B0, which it
 *           raises by 4 up to 0xF4 before each call, so the panel and its ramp
 *           move right together; D_801463B0 starts at 0x2c, where the tile
 *           lands on the sibling's 0x4c anchor. The panel's screen role is not
 *           established.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The left-edge x is written as the duplicated expression `x + 0x22` in the two
 * right-edge stores rather than through a named right-edge temporary or an
 * in-place `x += 0x22`. The original copies the parameter into $v0 for the two
 * left-edge stores and then adds 0x22 to the parameter's own register; the
 * duplicated expression reproduces that pair, while the named temporary (89/95
 * instructions) and the in-place increment (77/95, one extra saved register)
 * measured worse. */
void drawSlidingPanelWithGradientEdge(s16 x) {
  TILE* tile;
  POLY_G4* quad;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  GpuAppendPrim(1, 0xc);

  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  tile->x0 = x + 0x20;
  tile->y0 = 0x30;
  tile->w = 0xa8;
  tile->h = 0x18;
  tile->r0 = 0;
  tile->g0 = 0;
  tile->b0 = 0;
  GpuAppendPrim(1, 0x10);

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 2, 0x3c0, 0), 0);
  GpuAppendPrim(1, 0xc);

  quad = (POLY_G4*)g_PrimCursor;
  SetPolyG4(quad);
  quad->r1 = 0xff;
  quad->g1 = 0xff;
  quad->b1 = 0xff;
  quad->r3 = 0xff;
  quad->g3 = 0xff;
  quad->b3 = 0xff;
  quad->x0 = x;
  quad->x2 = x;
  quad->r0 = 0;
  quad->g0 = 0;
  quad->b0 = 0;
  quad->r2 = 0;
  quad->g2 = 0;
  quad->b2 = 0;
  quad->y0 = 0x30;
  quad->x1 = x + 0x22;
  quad->y1 = 0x30;
  quad->y2 = 0x48;
  quad->x3 = x + 0x22;
  quad->y3 = 0x48;
  SetSemiTrans(quad, 1);
  GpuAppendPrim(1, 0x24);
}
