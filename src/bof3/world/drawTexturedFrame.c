#include "bof3/world/area00813_internal.h"

/* @behavior draws the local textured frame as four translucent FT4 border
 * strips (left edge, top/bottom span, right-inner span, right edge) around a
 * shared texture window, then queues the matching inner fill rectangle.
 * @source 0x801F3D88
 * @status partial
 * @match 82.89
 * @residual instruction scheduling/register allocation: the allocator assigns
 *           bottom_y to s2 and bottom_v to s3 (the original is the reverse); the
 *           x/y halfword loads are hoisted to the top of the entry block (the
 *           original schedules them in the first GetGraphType() call shadow),
 *           which is the first difference at object offset 0x04; the block C/D/E
 *           store, clamp and re-read order also differs. Frame (0x78), prologue
 *           and the primitive field mapping match, but the object is 337/339
 *           instructions (1348/1356 bytes). No register pinning, clobber,
 *           barrier or scheduling aid is permitted.
 * @note the byte-identical twins emi/etc/shop/00@0x801DAB90 and
 *       emi/etc/sisyou/00@0x801D1E54 share this body and are still partial.
 */
void drawTexturedFrame(s32 arg0, s32 arg1, s32 arg2, s32 arg3, u8 arg4) {
  RECT*     texture_window;
  POLY_FT4* primitive;
  s32       x;
  s32       bottom_y;
  s32       half_copy;
  s32       y;
  s32       clut_x;
  s32       left_x;
  s32       bottom_v;
  u16       odd16;
  s32       width_plus_1;
  s32       odd_width;
  s32       right_base;
  s32       two;
  s32       arg3_l;
  s32       arg1_l;
  s32       zero_v;

  texture_window = (RECT*)g_PrimCursor;
  g_PrimCursor = (u8*)(texture_window + 1);
  texture_window->y = 0xf0;
  texture_window->x = 0;
  texture_window->w = 0x10;
  texture_window->h = 0x10;

  x = (s32)(u16)arg0;
  y = (s32)(u16)arg1;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, 0xf, texture_window);
  func_8014E5A0(1u, 0x0cu);

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  clut_x = ((s32)arg4 << 5) + 0x10;
  two = 2;
  primitive->x0 = (s16)x;
  primitive->y0 = (s16)y;
  left_x = arg0 + two;
  primitive->y1 = (s16)y;
  primitive->x2 = (s16)x;
  primitive->x1 = (s16)left_x;
  arg3_l = arg3;
  arg1_l = arg1;
  primitive->x3 = (s16)left_x;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  bottom_v = arg3_l + 1;
  bottom_y = arg1_l + bottom_v;
  primitive->y2 = (s16)bottom_y;
  primitive->y3 = (s16)bottom_y;
  primitive->u1 = 2u;
  primitive->v3 = (primitive->v2 = (u8)bottom_v);
  primitive->u3 = 2u;
  primitive->y0 += two;
  primitive->y2 -= 3;
  primitive->v0 += two;
  primitive->v2 -= 3;
  primitive->clut = GetClut(clut_x, 0x1e1);

  width_plus_1 = arg2 + 1;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  x = (s32)(u16)width_plus_1;
  func_8014E5A0(1u, 0x28u);
  x = ((s32)(u16)x - 4) >> 1;

  primitive = (POLY_FT4*)g_PrimCursor;
  odd_width = (arg2 - 3) & 1;
  odd16 = (u16)odd_width;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  half_copy = x;
  primitive->x0 = (s16)left_x;
  primitive->y0 = (s16)y;
  primitive->x1 = (s16)(x + left_x);
  primitive->y1 = (s16)y;
  primitive->x2 = (s16)left_x;
  primitive->y2 = (s16)bottom_y;
  primitive->x3 = (s16)(x + left_x);
  primitive->y3 = (s16)bottom_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->u1 = (u8)half_copy;
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->v2 = (u8)bottom_v;
  primitive->u3 = (u8)half_copy;
  primitive->v3 = (u8)bottom_v;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  primitive->clut = GetClut(clut_x, 0x1e1);
  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  right_base = arg0 + half_copy;
  x = half_copy + odd_width;
  primitive->x0 = (s16)(right_base + two);
  primitive->y0 = (s16)y;
  primitive->x1 = (s16)(right_base + (x + two));
  primitive->y1 = (s16)y;
  primitive->x2 = (s16)(right_base + two);
  primitive->y2 = (s16)bottom_y;
  primitive->x3 = (s16)(right_base + (x + two));
  primitive->y3 = (s16)bottom_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->u1 = (u8)(half_copy + odd16);
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->v2 = (u8)bottom_v;
  primitive->u3 = (u8)(half_copy + odd16);
  primitive->v3 = (u8)bottom_v;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  primitive->clut = GetClut(clut_x, 0x1e1);

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  primitive->y2 = (s16)bottom_y;
  primitive->v2 = (u8)bottom_v;
  primitive->x0 = (s16)(arg0 + width_plus_1 - two);
  primitive->x2 = (s16)(arg0 + width_plus_1 - two);
  primitive->x1 = (s16)(arg0 + width_plus_1);
  primitive->y1 = (s16)y;
  primitive->x3 = (s16)(arg0 + width_plus_1);
  primitive->y3 = (s16)bottom_y;
  primitive->y0 = (s16)y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  primitive->u1 = 2u;
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->u3 = 2u;
  primitive->v3 = (u8)bottom_v;
  primitive->r0 = 0xacu;
  primitive->g0 = 0xacu;
  primitive->b0 = 0xacu;
  primitive->y1 += two;
  primitive->y3 -= 3;
  primitive->y2 -= 1;
  primitive->v1 += two;
  primitive->v2 -= 1;
  primitive->v3 -= 3;
  primitive->clut = GetClut(clut_x, 0x1e1);

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  texture_window = (RECT*)g_PrimCursor;
  g_PrimCursor = (u8*)(texture_window + 1);
  zero_v = 0;
  texture_window->x = 0;
  texture_window->y = zero_v;
  texture_window->w = 0x100;
  texture_window->h = 0x100;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, 0xf, texture_window);
  func_8014E5A0(1u, 0x0cu);
  func_801AEBA0((s16)left_x, (s16)(arg1 + two), (s16)(arg2 - 4),
                (s16)(arg3 - 4), 0);
}
