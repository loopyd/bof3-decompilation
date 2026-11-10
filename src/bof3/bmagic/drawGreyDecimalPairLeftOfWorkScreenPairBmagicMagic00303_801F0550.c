#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
s32 GetGraphType(void);

/* @source 0x801F0550
 * @behavior Queues a draw-mode packet on ordering-table index 2, selecting
 * texture page 0x25 for graph types 1 and 2 and 0x15 otherwise. Splits the
 * unsigned byte argument by ten and draws its remainder as a grey 12-by-12
 * glyph at work halfword 0x2E minus 66 and work halfword 0x30. If the
 * quotient is nonzero, draws its glyph 12 pixels to the left. Both use
 * texture columns 0x48 plus twelve times the glyph index, v 0x18 to 0x24,
 * texture page GetTPage(0, 0, 0x3C0, 0), CLUT (0, 0x1E0), and RGB 0x80.
 * The caller at 0x801EF0A8 passes work byte 0xA without a range check.
 * @status partial
 * @match 65.79
 * @residual Register allocation, narrow-value conversions and scheduling differ;
 * measured 100 of 152 instructions and 604 of 608 bytes.
 */
void drawGreyDecimalPairLeftOfWorkScreenPairBmagicMagic00303_801F0550(u8 value) {
  POLY_FT4 *primitive;
  u16 x;
  u16 y;
  s32 graph;
  u8 quotient;
  u8 remainder;
  s32 left;
  s32 right;
  s32 bottom;
  s32 u;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(2, 0xC);

  quotient = (u32)value / 10;
  remainder = value - quotient * 10;
  primitive = (POLY_FT4 *)g_PrimCursor;
  x = *(u16 *)(g_battle_work + 0x2E);
  y = *(u16 *)(g_battle_work + 0x30);
  right = x - 66;
  left = right;
  SetPolyFT4(primitive);

  primitive->x0 = left;
  primitive->y0 = y;
  primitive->x1 = x - 54;
  primitive->y1 = y;
  primitive->x2 = left;
  primitive->y2 = bottom = y + 12;
  primitive->x3 = x - 54;
  primitive->y3 = bottom;
  primitive->tpage = GetTPage(0, 0, 0x3C0, 0);
  primitive->clut = GetClut(0, 0x1E0);
  u = remainder * 12;
  primitive->u0 = u + 0x48;
  primitive->v0 = 0x18;
  primitive->u1 = u + 0x54;
  primitive->v1 = 0x18;
  primitive->u2 = u + 0x48;
  primitive->v2 = 0x24;
  primitive->u3 = u + 0x54;
  primitive->v3 = 0x24;
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  GpuAppendPrim(2, 0x28);

  left = x - 78;
  if (quotient != 0) {
    primitive = (POLY_FT4 *)g_PrimCursor;
    SetPolyFT4(primitive);
    primitive->x0 = left;
    primitive->y0 = y;
    primitive->x1 = right;
    primitive->y1 = y;
    primitive->x2 = left;
    primitive->y2 = bottom;
    primitive->x3 = right;
    primitive->y3 = bottom;
    primitive->tpage = GetTPage(0, 0, 0x3C0, 0);
    primitive->clut = GetClut(0, 0x1E0);
    u = quotient * 12;
    primitive->u0 = u + 0x48;
    primitive->v0 = 0x18;
    primitive->u1 = u + 0x54;
    primitive->v1 = 0x18;
    primitive->u2 = u + 0x48;
    primitive->v2 = 0x24;
    primitive->u3 = u + 0x54;
    primitive->v3 = 0x24;
    primitive->r0 = 0x80;
    primitive->g0 = 0x80;
    primitive->b0 = 0x80;
    GpuAppendPrim(2, 0x28);
  }
}
