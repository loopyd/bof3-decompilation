#include "bof3/bof3.h"
#include "gpu/prim.h"

/* One cell of the three-record table this overlay draws: the cell's
 * upper-left corner x and y, then its width and height, all in 128-unit screen
 * steps. The body reads every field at its own use. */
typedef struct CellRecord {
  u8 x;
  u8 y;
  u8 w;
  u8 h;
} CellRecord;

/* The three cell records drawn below, twelve bytes in all. The original reads
 * each field where that use stands, so the fields stay behind one base
 * symbol. */
/* @source 0x801F3330 @kind table */
extern CellRecord cellRectTableWorld02Area10513_801F3330[];

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects. The
 * original keeps their base in a saved register and writes the halfword lanes
 * through that one pointer.
 * @source 0x1F800014 @kind data
 */
extern SVECTOR D_1F800014[];

/* Shared graph-type probe of the main executable. */
/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* @source 0x801F2D9C
 * @behavior Draws this overlay's three textured cell quads and brackets them in
 *           a draw-mode pair. Each draw-mode packet is an 8-byte RECT written
 *           into the primitive buffer at g_PrimCursor, which the cursor then
 *           skips, followed by the 0xC-byte packet linked under
 *           ordering-table slot 5: the opening RECT scopes the texture window
 *           to (0x60, 0x10, 0x10, 0x10) and the closing one resets it to
 *           (0, 0, 0x100, 0x100), and both select texture page 0x2B when
 *           GetGraphType returns 1 or 2 and 0x1B for every other type.
 *           Between them it walks the three four-byte cell records at
 *           0x801F3330 as three POLY_FT4 quads, each stamped at g_PrimCursor,
 *           unshaded, semi-transparent and flat 0x80 grey. A record's x and
 *           width give the left edge (x - 0x80) * 128 - 0x40 and the right edge
 *           (x + width - 0x81) * 128 + 0x40, its y and height give the top and
 *           bottom edges the same way, every edge carries 4 * the argument, and
 *           all four corners share the depth 6 * the argument - 0x380. The
 *           texture columns run from 0 to 16 * width - 1 and the rows from 0 to
 *           16 * height - 1 on clut 0x78C6, with texture page 0x1AB for graph
 *           types 1 and 2 and 0x7B otherwise. On the third record only, the
 *           top-left corner also moves by 8 * the argument in both x and y.
 *           RotTransPers4 projects the four scratchpad vectors at 0x1F800014
 *           straight into the primitive x0 to x3 slots, and the 0x28-byte quad
 *           is linked under ordering-table slot 5.
 *           The argument is a per-call displacement rather than a state: the
 *           state-1 and state-2 handlers at 0x801F2C94 and 0x801F2CDC pass 0,
 *           while the state-3 handler at 0x801F2D38 passes work byte 9 after
 *           incrementing it, so the cells and the third quad's slip advance
 *           once per call of that handler.
 * @status partial
 * @match 99.57
 * @residual One scheduled slot: the original materializes the scratchpad
 *           vector base in the loop-entry block before the 4 * argument shift,
 *           while this candidate lets the allocator rematerialize that base
 *           after it. The two instructions swap places; every other
 *           instruction, register and byte of the 932-byte span matches
 *           (232 of 233 instructions).
 */
void drawThreeCellTexturedQuadsShiftedByArgWorld02Area10513_801F2D9C(
    s32 shift) {
  POLY_FT4 *prim;
  SVECTOR *v;
  RECT *tw;
  long p;
  long flag;
  s32 tpage;
  s32 graph;
  s32 left;
  s32 top;
  s32 right;
  s32 bottom;
  s32 depth;
  s32 i;

  tw = (RECT *)g_PrimCursor;
  g_PrimCursor += 8;
  tw->x = 0x60;
  tw->y = 0x10;
  tw->h = 0x10;
  tw->w = 0x10;
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x2B;
  } else {
    graph = 0x1B;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, graph, tw);
  GpuAppendPrim(5, 0xC);

  v = D_1F800014;
  for (i = 0; i < 3; i++) {
    prim = (POLY_FT4 *)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    SetSemiTrans(prim, 1);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0x1AB;
    } else {
      tpage = 0x7B;
    }
    prim->tpage = tpage;
    prim->clut = 0x78C6;

    left = shift * 4 +
           (cellRectTableWorld02Area10513_801F3330[i].x - 0x80) * 128 - 0x40;
    v[0].vx = v[2].vx = left;
    right = shift * 4 +
            (cellRectTableWorld02Area10513_801F3330[i].x +
             cellRectTableWorld02Area10513_801F3330[i].w - 0x81) *
                128 +
            0x40;
    v[1].vx = v[3].vx = right;
    top = shift * 4 +
          (cellRectTableWorld02Area10513_801F3330[i].y - 0x80) * 128 - 0x40;
    v[0].vy = v[1].vy = top;
    bottom = shift * 4 +
             (cellRectTableWorld02Area10513_801F3330[i].y +
              cellRectTableWorld02Area10513_801F3330[i].h - 0x81) *
                 128 +
             0x40;
    v[2].vy = v[3].vy = bottom;
    depth = shift * 6 - 0x380;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = depth;
    if (i == 2) {
      v[0].vx = left + shift * 8;
      v[0].vy = top + shift * 8;
    }

    prim->u0 = 0;
    prim->v0 = 0;
    prim->u1 = cellRectTableWorld02Area10513_801F3330[i].w * 16 - 1;
    prim->v1 = 0;
    prim->u2 = 0;
    prim->v2 = cellRectTableWorld02Area10513_801F3330[i].h * 16 - 1;
    prim->u3 = cellRectTableWorld02Area10513_801F3330[i].w * 16 - 1;
    prim->v3 = cellRectTableWorld02Area10513_801F3330[i].h * 16 - 1;
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;

    RotTransPers4(v, &v[1], &v[2], &v[3], (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3, &p,
                  &flag);
    GpuAppendPrim(5, 0x28);
  }

  tw = (RECT *)g_PrimCursor;
  g_PrimCursor += 8;
  tw->x = 0;
  tw->y = 0;
  tw->h = 0x100;
  tw->w = 0x100;
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x2B;
  } else {
    graph = 0x1B;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, graph, tw);
  GpuAppendPrim(5, 0xC);
}
