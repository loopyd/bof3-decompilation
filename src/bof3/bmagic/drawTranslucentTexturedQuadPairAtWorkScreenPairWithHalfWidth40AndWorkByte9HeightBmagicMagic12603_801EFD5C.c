#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F80000E; /* @source 0x1F80000E @kind data */

extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size);
extern void func_8017A9E0(POLY_GT4 *primitive);
extern s32 GetGraphType(void);

/* @source 0x801EFD5C
 * @behavior Draws two semi-transparent textured POLY_GT4 bands around work
 *           screen halfwords 0x2E and 0x30. Takes no arguments and returns
 *           nothing; the caller at 0x801EFAF4 passes no arguments.
 *           Seeds scratchpad halfwords 0, 2, 4, 0xC and 0xE with 40, work
 *           byte 9, eight times work byte A, and the screen pair. Installs
 *           draw mode 0x2A5 for graph types 1 or 2, otherwise 0xB5, and links
 *           it with kind 2 and size 0xC at work words 0x34 and 0x38.
 *           Both quads span screen x minus 40 through x plus 40. The upper
 *           band runs from y minus work byte 9 minus 16 through y minus 16;
 *           the lower runs from y minus 16 through y. Both use tpage
 *           GetTPage(0, 1, 0x340, 0x100), clut GetClut(0, 0x1FA), and u
 *           coordinates 8 and 0x57. The upper v range is 8 through work
 *           byte 9 plus 8, and the lower is work byte 9 plus 8 through that
 *           byte plus 0x18. Upper colours and lower top colours take fresh
 *           reads of the tint halfword; lower bottom colours are 1. Each
 *           finished quad is linked with kind 2 and size 0x34. Record field
 *           roles and the unnamed primitive helpers remain address-anchored.
 * @status partial
 * @match 40.57
 * @residual Scratchpad access aliasing and saved-register allocation differ; 114 of 281 instructions match.
 */
void drawTranslucentTexturedQuadPairAtWorkScreenPairWithHalfWidth40AndWorkByte9HeightBmagicMagic12603_801EFD5C(void) {
  POLY_GT4 *primitive;
  s32 graph;
  u16 *screen;
  u8 *work;

  work = D_1F800044;
  D_1F800000 = 40;
  D_1F800002 = work[9];
  D_1F800004 = work[0xA] << 3;
  D_1F80000C = *(u16 *)(work + 0x2E);
  D_1F80000E = *(u16 *)(work + 0x30);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x2A5;
  } else {
    graph = 0xB5;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(D_1F800044 + 0x34),
                *(s32 *)(D_1F800044 + 0x38), 2, 0xC);
  primitive = (POLY_GT4 *)g_PrimCursor;
  func_8017A9E0(primitive);
  SetSemiTrans(primitive, 1);
  screen = &D_1F80000C;
  primitive->x0 = *screen - *(u16 *)(screen - 6);
  primitive->y0 = D_1F80000E - (D_1F800002 + 16);
  primitive->x1 = *screen + *(u16 *)(screen - 6);
  primitive->y1 = D_1F80000E - (D_1F800002 + 16);
  primitive->x2 = *screen - *(u16 *)(screen - 6);
  primitive->y2 = D_1F80000E - 16;
  primitive->x3 = *screen + *(u16 *)(screen - 6);
  primitive->y3 = D_1F80000E - 16;
  primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
  primitive->clut = GetClut(0, 0x1FA);
  primitive->u0 = 8;
  primitive->v0 = 8;
  primitive->u1 = 0x57;
  primitive->v1 = 8;
  primitive->u2 = 8;
  primitive->u3 = 0x57;
  primitive->v2 = PSX_REF(u8, SPAD_ADDRESS(2)) + 8;
  primitive->v3 = PSX_REF(u8, SPAD_ADDRESS(2)) + 8;
  primitive->r0 = *(u16 *)(screen - 4);
  primitive->g0 = *(u16 *)(screen - 4);
  primitive->b0 = *(u16 *)(screen - 4);
  primitive->r1 = *(u16 *)(screen - 4);
  primitive->g1 = *(u16 *)(screen - 4);
  primitive->b1 = *(u16 *)(screen - 4);
  primitive->r2 = *(u16 *)(screen - 4);
  primitive->g2 = *(u16 *)(screen - 4);
  primitive->b2 = *(u16 *)(screen - 4);
  primitive->r3 = *(u16 *)(screen - 4);
  primitive->g3 = *(u16 *)(screen - 4);
  primitive->b3 = *(u16 *)(screen - 4);
  func_80155A08(*(s32 *)(D_1F800044 + 0x34),
                *(s32 *)(D_1F800044 + 0x38), 2, 0x34);

  primitive = (POLY_GT4 *)g_PrimCursor;
  func_8017A9E0(primitive);
  SetSemiTrans(primitive, 1);
  primitive->x0 = *screen - *(u16 *)(screen - 6);
  primitive->y0 = D_1F80000E - 16;
  primitive->x1 = *screen + *(u16 *)(screen - 6);
  primitive->y1 = D_1F80000E - 16;
  primitive->x2 = *screen - *(u16 *)(screen - 6);
  primitive->y2 = D_1F80000E;
  primitive->x3 = *screen + *(u16 *)(screen - 6);
  primitive->y3 = D_1F80000E;
  primitive->tpage = GetTPage(0, 1, 0x340, 0x100);
  primitive->clut = GetClut(0, 0x1FA);
  primitive->u0 = 8;
  primitive->u1 = 0x57;
  primitive->v0 = PSX_REF(u8, SPAD_ADDRESS(2)) + 8;
  primitive->u2 = 8;
  primitive->v1 = PSX_REF(u8, SPAD_ADDRESS(2)) + 8;
  primitive->u3 = 0x57;
  primitive->v2 = PSX_REF(u8, SPAD_ADDRESS(2)) + 0x18;
  primitive->v3 = PSX_REF(u8, SPAD_ADDRESS(2)) + 0x18;
  primitive->r0 = *(u16 *)(screen - 4);
  primitive->g0 = *(u16 *)(screen - 4);
  primitive->b0 = *(u16 *)(screen - 4);
  primitive->r1 = *(u16 *)(screen - 4);
  primitive->g1 = *(u16 *)(screen - 4);
  primitive->b1 = *(u16 *)(screen - 4);
  primitive->r2 = 1;
  primitive->g2 = 1;
  primitive->b2 = 1;
  primitive->r3 = 1;
  primitive->g3 = 1;
  primitive->b3 = 1;
  func_80155A08(*(s32 *)(D_1F800044 + 0x34),
                *(s32 *)(D_1F800044 + 0x38), 2, 0x34);
}
