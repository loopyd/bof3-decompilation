#include "internal.h"

/* @behavior draws the local textured frame in four translucent FT4 strips, then
 * queues the matching inner fill rectangle.
 * @source 0x801f3d88 FUN_801f3d88
 */
void func_801f3d88(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4) {
  volatile RECT*     texture_window;
  volatile POLY_FT4* primitive;
  s16                left_x;
  s16                top_y;
  s16                left_inner_x;
  s16                bottom_y;
  s16                half_width;
  s16                right_inner_x;
  s16                right_x;
  u16                clut_x;
  u16                odd_width;
  u8                 bottom_v;

  texture_window = (volatile RECT*)WORLD00_AREA008_PRIMITIVE_PTR;
  WORLD00_AREA008_PRIMITIVE_PTR =
      (volatile u8*)((volatile u8*)texture_window + sizeof(RECT));
  texture_window->x = 0;
  texture_window->y = 0xf0;
  texture_window->w = 0x10;
  texture_window->h = 0x10;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  SetDrawMode((DR_MODE*)WORLD00_AREA008_PRIMITIVE_PTR, 0, 1, 0xf,
              (RECT*)texture_window);
  func_8014e5a0(1u, 0x0cu);

  left_x = arg0;
  top_y = arg1;
  left_inner_x = arg0 + 2;
  bottom_v = arg3 + 1u;
  bottom_y = arg1 + (s16)bottom_v;
  clut_x = ((u16)arg4 << 5) + 0x10u;

  primitive = (volatile POLY_FT4*)WORLD00_AREA008_PRIMITIVE_PTR;
  SetPolyFT4((POLY_FT4*)primitive);
  SetSemiTrans((void*)primitive, 1);
  primitive->x0 = left_x;
  primitive->y0 = top_y + 2;
  primitive->u0 = 0u;
  primitive->v0 = 2u;
  primitive->clut = GetClut(clut_x, 0x1e1);
  primitive->x1 = left_inner_x;
  primitive->y1 = top_y;
  primitive->u1 = 2u;
  primitive->v1 = 0u;
  primitive->tpage = 0x0fu;
  primitive->x2 = left_x;
  primitive->y2 = bottom_y - 3;
  primitive->u2 = 0u;
  primitive->v2 = bottom_v - 3u;
  primitive->pad1 = 2u;
  primitive->x3 = left_inner_x;
  primitive->y3 = bottom_y;
  primitive->u3 = 2u;
  primitive->v3 = bottom_v;
  primitive->pad2 = 0;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  func_8014e5a0(1u, 0x28u);

  half_width = (s16)(((u16)(arg2 + 1u) - 4u) >> 1);
  odd_width = ((u16)(arg2 - 3u)) & 1u;

  primitive = (volatile POLY_FT4*)WORLD00_AREA008_PRIMITIVE_PTR;
  SetPolyFT4((POLY_FT4*)primitive);
  SetSemiTrans((void*)primitive, 1);
  primitive->x0 = left_inner_x;
  primitive->y0 = top_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->clut = GetClut(clut_x, 0x1e1);
  primitive->x1 = left_inner_x + half_width;
  primitive->y1 = top_y;
  primitive->u1 = (u8)half_width;
  primitive->v1 = 0u;
  primitive->tpage = 0x0fu;
  primitive->x2 = left_inner_x;
  primitive->y2 = bottom_y;
  primitive->u2 = 0u;
  primitive->v2 = bottom_v;
  primitive->pad1 = (u8)half_width;
  primitive->x3 = left_inner_x + half_width;
  primitive->y3 = bottom_y;
  primitive->u3 = (u8)half_width;
  primitive->v3 = bottom_v;
  primitive->pad2 = 0;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  func_8014e5a0(1u, 0x28u);

  right_inner_x = left_inner_x + half_width;
  right_x = left_inner_x + half_width + half_width + (s16)odd_width;

  primitive = (volatile POLY_FT4*)WORLD00_AREA008_PRIMITIVE_PTR;
  SetPolyFT4((POLY_FT4*)primitive);
  SetSemiTrans((void*)primitive, 1);
  primitive->x0 = right_inner_x;
  primitive->y0 = top_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->clut = GetClut(clut_x, 0x1e1);
  primitive->x1 = right_x;
  primitive->y1 = top_y;
  primitive->u1 = (u8)(half_width + (s16)odd_width);
  primitive->v1 = 0u;
  primitive->tpage = 0x0fu;
  primitive->x2 = right_inner_x;
  primitive->y2 = bottom_y;
  primitive->u2 = 0u;
  primitive->v2 = bottom_v;
  primitive->pad1 = (u8)(half_width + (s16)odd_width);
  primitive->x3 = right_x;
  primitive->y3 = bottom_y;
  primitive->u3 = (u8)(half_width + (s16)odd_width);
  primitive->v3 = bottom_v;
  primitive->pad2 = 0;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  func_8014e5a0(1u, 0x28u);

  right_x = arg0 + arg2 + 1;

  primitive = (volatile POLY_FT4*)WORLD00_AREA008_PRIMITIVE_PTR;
  SetPolyFT4((POLY_FT4*)primitive);
  SetSemiTrans((void*)primitive, 1);
  primitive->x0 = right_x - 2;
  primitive->y0 = top_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->clut = GetClut(clut_x, 0x1e1);
  primitive->x1 = right_x;
  primitive->y1 = top_y + 2;
  primitive->u1 = 2u;
  primitive->v1 = 2u;
  primitive->tpage = 0x0fu;
  primitive->x2 = right_x - 2;
  primitive->y2 = bottom_y - 1;
  primitive->u2 = 0u;
  primitive->v2 = bottom_v - 1u;
  primitive->pad1 = 2u;
  primitive->x3 = right_x;
  primitive->y3 = bottom_y - 3;
  primitive->u3 = 2u;
  primitive->v3 = bottom_v - 3u;
  primitive->pad2 = 0;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  func_8014e5a0(1u, 0x28u);

  texture_window = (volatile RECT*)WORLD00_AREA008_PRIMITIVE_PTR;
  WORLD00_AREA008_PRIMITIVE_PTR =
      (volatile u8*)((volatile u8*)texture_window + sizeof(RECT));
  texture_window->x = 0;
  texture_window->y = 0;
  texture_window->w = 0x100;
  texture_window->h = 0x100;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  SetDrawMode((DR_MODE*)WORLD00_AREA008_PRIMITIVE_PTR, 0, 1, 0xf,
              (RECT*)texture_window);
  func_8014e5a0(1u, 0x0cu);
  func_801aeba0((s16)(arg0 + 2), (s16)(arg1 + 2), (s16)(arg2 - 4),
                (s16)(arg3 - 4), 0);
}
