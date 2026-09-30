#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D1E54
 * @behavior draws the shared translucent panel strip at the masked coordinates
 *           (arg0, arg1) sized arg2 by arg3: a first FT4 border quad around the
 *           0x10 by 0x10 texture window at (0, 0xF0) whose CLUT column is
 *           arg4 * 32 + 0x10 and whose flat colour is 0xAC, then the
 *           top/bottom span, the right-inner span and the right edge quads,
 *           each queued at texture page 0xF after a GetGraphType call; finally
 *           a second 0x100 by 0x100 texture window draw mode followed by the
 *           inner fill rectangle from (arg0 + 2, arg1 + 2) to
 *           (arg0 + arg2 - 4, arg1 + arg3 - 4) through func_801AEBA0.
 * @status partial
 * @match 68.44
 * @residual register allocation and basic-block scheduling: the reconstructed
 *           clean-C shape reproduces the instruction sequence in order, but the
 *           allocator places y, bottom_v, bottom_y and half_copy in different
 *           callee-saved registers and folds the half_copy copy, leaving a
 *           0x70 frame instead of 0x78; no register pinning, clobber or
 *           scheduling aid is permitted.
 */
void func_801D1E54(s32 arg0, s32 arg1, s32 arg2, s32 arg3, u8 arg4) {
  RECT*     texture_window;
  POLY_FT4* primitive;
  s32       x;
  s32       bottom_y;
  s32       half_copy;
  s32       y;
  s32       clut_x;
  s32       left_x;
  s32       bottom_v;
  u8        right_u;
  s16       odd16;
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

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  x = (s32)(u16)arg0;
  y = (s32)(u16)arg1;

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
  x = (s32)(u16)width_plus_1;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);
  x = ((s32)(u16)x - 4) >> 1;

  primitive = (POLY_FT4*)g_PrimCursor;
  odd_width = (arg2 - 3) & 1;
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
  odd16 = (s16)odd_width;

  if (GetGraphType() != 1) {
    GetGraphType();
  }

  primitive->tpage = 0x0fu;
  func_8014E5A0(1u, 0x28u);

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);
  SetSemiTrans(primitive, 1);
  right_base = arg0 + x;
  primitive->x0 = (s16)(right_base + two);
  primitive->y0 = (s16)y;
  primitive->x1 = (s16)(right_base + (x + odd_width + two));
  primitive->y1 = (s16)y;
  primitive->x2 = (s16)(right_base + two);
  primitive->y2 = (s16)bottom_y;
  primitive->x3 = (s16)(right_base + (x + odd_width + two));
  primitive->y3 = (s16)bottom_y;
  primitive->u0 = 0u;
  primitive->v0 = 0u;
  right_u = (u8)(half_copy + odd16);
  primitive->u1 = (u8)(half_copy + odd16);
  primitive->v1 = 0u;
  primitive->u2 = 0u;
  primitive->v2 = (u8)bottom_v;
  primitive->u3 = right_u;
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
  primitive->r0 = primitive->g0 = primitive->b0 = 0xacu;
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
