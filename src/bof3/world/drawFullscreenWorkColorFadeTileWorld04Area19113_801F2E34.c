#include "bof3/bof3.h"

/* Scratchpad pointer cell of the work record whose colour triplet this fade
 * tile shades with.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

extern u8* g_PrimCursor; /* @source 0x8014598C @kind unknown */

/* @source 0x8014E5A0 */
extern void appendRenderPrim(u32 ot_index, u32 primitive_size);

/* @source 0x801F2E34
 * @behavior Draws the full-screen wash of the state-3 overlay handler: selects
 *           the drawing mode for tpage GetTPage(0, 2, 0x3C0, 0), that is
 *           texture page 0 with subtract blending, dithering enabled and no
 *           texture window, queues the 0x0C-byte mode primitive on order-table
 *           head 5, then builds a 320x240 TILE at the screen origin whose red,
 *           green and blue channels are the work-record bytes at offsets 0x5D,
 *           0x5E and 0x5F, marks it semi-transparent with shading disabled and
 *           queues the finished 0x10-byte primitive on the same head. The
 *           scratchpad pointer cell 0x1F800044 is re-read for each of the three
 *           channels, so every channel comes from the record published at that
 *           point rather than from one cached pointer. The overlay dispatcher
 *           reaches this body through the entry-3 wrapper at 0x801F2E14. Takes
 *           no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFullscreenWorkColorFadeTileWorld04Area19113_801F2E34(void) {
  TILE* tile;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 2, 0x3C0, 0), 0);
  appendRenderPrim(5, 0xC);
  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  tile->r0 = D_1F800044[0x5D];
  tile->g0 = D_1F800044[0x5E];
  tile->b0 = D_1F800044[0x5F];
  tile->w = 0x140;
  tile->x0 = 0;
  tile->y0 = 0;
  tile->h = 0xF0;
  SetSemiTrans(tile, 1);
  SetShadeTex(tile, 0);
  appendRenderPrim(5, 0x10);
}
