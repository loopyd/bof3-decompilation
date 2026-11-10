#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_801F0AD4[]; /* @source 0x801F0AD4 @kind data */

s32 GetGraphType(void);

/* @source 0x801F03F0
 * @behavior Queues a draw-mode packet on ordering-table index 2, selecting
 * texture page 0x25 when GetGraphType returns 1 or 2 and 0x15 otherwise.
 * Draws a grey 12-by-12 textured glyph at work halfwords 0x2E and 0x30.
 * The low byte of the argument selects the CLUT x coordinate from the byte
 * table at 0x801F0AD4 and the texture column (index + 15) * 12. The texture
 * rectangle spans v 0x30 to 0x3C on GetTPage(0, 0, 0x3C0, 0), with CLUT
 * y 0x1E0 and RGB 0x80. Queues the 0x28-byte polygon on index 2.
 * The caller at 0x801EEF6C supplies work byte 0xB as the glyph index;
 * the meaning of individual glyphs and palette entries is not established.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawGrey12PixelGlyphAtWorkScreenPairBmagicMagic00303_801F03F0(u8 index) {
  POLY_FT4 *primitive;
  u16 x;
  u16 y;
  s32 graph;
  s32 u;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(2, 0xC);

  primitive = (POLY_FT4 *)g_PrimCursor;
  x = *(u16 *)(g_battle_work + 0x2E);
  y = *(u16 *)(g_battle_work + 0x30);
  SetPolyFT4(primitive);

  primitive->x0 = x;
  primitive->y0 = y;
  primitive->x1 = x + 12;
  primitive->y1 = y;
  primitive->x2 = x;
  primitive->y2 = y + 12;
  primitive->x3 = x + 12;
  primitive->y3 = y + 12;

  primitive->tpage = GetTPage(0, 0, 0x3C0, 0);
  primitive->clut = GetClut(D_801F0AD4[index], 0x1E0);

  u = (index + 15) * 12;
  primitive->u0 = u;
  primitive->v0 = 0x30;
  primitive->u1 = u + 12;
  primitive->v1 = 0x30;
  primitive->u2 = u;
  primitive->v2 = 0x3C;
  primitive->u3 = u + 12;
  primitive->v3 = 0x3C;
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;

  GpuAppendPrim(2, 0x28);
}
